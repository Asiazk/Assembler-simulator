#include "utils.h"
#include "stringUtils.h"

/* error printing function: gets a description of the error and line number */
void printError(char *errorComment, int line) {
    printf(ANSI_COLOR_RED "ERROR in line " ANSI_COLOR_RESET);
    printf(ANSI_COLOR_RED "%d-" ANSI_COLOR_RESET,line );
    printf("%s\n",errorComment);
}

/* creating file name with given name and extension, return the name in output */
char *createFileName(char *input, char *extension, char *output) {
    strcpy(output,input);
    strcat(output,extension);
    return output;
}

/* names must be legal:
    - start with alpha character and then only alpha-numeric characters
    - no reserved names such as register names or command names
  check if given word is a valid name, if valid-return 1, else-return 0 */
int isNameValid(char *str) {
    int i;

    if(strcmp(str,*COMMANDS)==0) {
        toLowerString(str);
    }
    /* check if given string is not on reserved names array */
    for (i=0; i<NUM_OF_RESERVED_NAMES; i++) {
        if (strcmp(*RESERVED_NAMES,str)==0) {
            return 0;
        }
    (*RESERVED_NAMES)++;
    }
    /* first character must be alpha only */
    if (!isalpha(str[0])) {
        return 0;
    }
    /* rest of the characters must be alpha-numeric only */
    for (i=1; i<strlen(str); i++) {
        if(!isalnum(str[i])) {
            return 0;
        }
    }
    if (*str == '\0') {
        return 0;
    }
    return 1;
}

/* check if given word is one of 16 commands. if so-return the index, else-return -1 */
int findCommandIndex(char word[]) {
    int i;
   
	for(i = 0; i < NUM_OF_COMMANDS; i++) {
		if(strcmp(COMMANDS[i], word)==0)
		   return i;
	}
	return -1;
}

/* dividing an instruction to operands (into first & second with comma) after getting a command */
int divideInstruction(char *instruction, char first[], char second[]) {
    int instIndex;
    int secondIndex;
    int reachedComma=0;

    for (instIndex=0; instIndex<strlen(instruction); instIndex++) {
        if (instruction[instIndex]==',') {
            reachedComma=1;
            break;
        }
        first[instIndex]=instruction[instIndex];
    }

    if (reachedComma==1) {
        for (secondIndex=0; instIndex<strlen(instruction); secondIndex++, instIndex++) {
            second[secondIndex]=instruction[instIndex];
        }
    }
    return reachedComma;
}

/* assuring line and no longer than 80 characters */
int checkLineLength(int counter, int maxSize, int line) {
    if (counter>=maxSize) {
        printError("Line is longer than 80 characters", line);
        return 0;
    }
    return 1;
}

/* split the given sentence with a dot to first&second
   before using the function we check there is a dot in sentence */
int splitByDotOperands(char *sentence, char first[], char second[]) {
    int i, j;
    int reachedDot = 0;

    if (findChar(sentence,'.')) {
        for (i = 0; i < strlen(sentence); i++) {
            if (sentence[i] == '.') {
                reachedDot = 1;
                break;
            }

            first[i] = sentence[i];
        }
    }

    if (reachedDot == 1) {
        i++;
        for (j = 0; i < strlen(sentence); j++, i++) {
            second[j] = sentence[i];
        }
    }
    return reachedDot;
}

/* check if given string is a name of a register
   if so-return index from register list,else-return -1 */
int isRegister(char *reg) {
    int i;
   
	for(i = 0; i < NUM_OF_REGISTERS; i++) {
		if(strcmp(REGISTERS[i], reg)==0)
		   return i;
	}
	return -1;
}

/* count operands of a given sentence after a command */
int countOperands(char *sentence) {
    char str[MAX_LINE_SIZE]={0}, delimitEndOfSen[]=" \r\n\v\f";
    char *word;

    if (sentence[0]==',') { /* this means no first operand */
        return -1;
    }
    if (findChar(sentence,',')==1) {
        strcpy(str,sentence);
        word=strtok(str,",");
        word=strtok(NULL,delimitEndOfSen);
        if (!word) { /* this means there's no word after the comma */
            return -1;
        }
        else {
            if (findChar(word,',')==1) { /* if in second operand we have comma-excess operand */
                return -1;
            }
            else {
                return 2;
            }
            
        }
    }
    else if (findChar(sentence,',')==-1) {
        return 1;
    }
    return -1;
}

/* check if given name is in the given array of label pointers. if so-return 1, else return 0 */
int checkLabelName(LabelStruct *labelList[], int listSize ,char *name) {
    int i;
    for (i=0; i<listSize; i++) {
        if (strcmp(labelList[i]->labelName,name)==0) {
            return 1;
        }
    }
    return 0;
}

void validateExternMalloc(ExternTable *ptr, int line) {
    if (ptr==NULL) {
        printError("error while allocating new extern node",line);
        exit(-1);
    }
}

/* going over lable array and freeing the nodes */
void freeLabelList(LabelStruct *labelList[], GlobalCounters *globalCountersPtr) {
    int i;
    for (i=0; i<globalCountersPtr->labelListSize; i++) {
        free(labelList[i]);
    }
}

void freeBinaryList(AddressAndCode *list[], GlobalCounters *globalCountersPtr) {
    int i;
    for (i=0; i<globalCountersPtr->addressCodeSize; i++) {
        free(list[i]);
    }
}

void freeExternList(ExternTable *list[], GlobalCounters *globalCountersPtr) {
    int i;
    for (i=0; i<globalCountersPtr->externTableSize; i++) {
        free(list[i]);
    }
}

void freeEntryList(EntryTable *list[], GlobalCounters *globalCountersPtr) {
    int i;
    for (i=0; i<globalCountersPtr->entryTableSize; i++) {
        free(list[i]);
    }
}

/* check if a given word is data part. if so-return 1, else-return 0 */
int isDataPart(char *word) {
    if(strcmp(word,".data")==0 || strcmp(word,".string")==0 || strcmp(word,".struct")==0 ||
       strcmp(word,".entry")==0 || strcmp(word,".extern")==0) {
        return 1;
    }
    return 0;  
}

/* search label in label list and if found, check if operation field is ".struct". if so-return 1, else-return -1 */
int checkStruct(LabelStruct *labelList[], char *name, GlobalCounters *globalCountersPtr) {
    int i;
    for (i=0; i<globalCountersPtr->labelListSize; i++) {
        if (strcmp(labelList[i]->labelName,name)==0) {
            if (strcmp(labelList[i]->operation,".struct")==0) {
                return 1;
            }
        }
    }
    return -1;
}

/* check whether the label is entry\extern
   the search is done only if list contains any lables
   if entry-address type is 0
   if extern-address type is 1
   if neither-address type is 2 */
int checkLabelARE(char *name, EntryTable *entryPtrArray[],
                   ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr) {
    int i;

    if (globalCountersPtr->entryTableSize>0) {
        for (i=0; i<globalCountersPtr->entryTableSize; i++) {
            if (strcmp(entryPtrArray[i]->content,name)==0) {
                return 0;
            }
        }
    }
    if (globalCountersPtr->externTableSize>0) {
        for (i=0; i<globalCountersPtr->externTableSize; i++) {
            if (strcmp(externPtrArray[i]->content,name)==0) {
                return 1;
            }
        }
    }
    return 2;
}

/* check the number found in operand.
   legal format of a number in operand:
   -first character #\-\+\digit
   -second character -\+\digit
   -all other charactes only digits 
   if number is legal-return 1, else-return -1 */
int checkNumOperand(char *num) { 
    int i;
    if (strlen(num)>=1){
        if (num[0]!='#') { 
            return -1;
        }
    }
    if (strlen(num)>1) {
        if (!isdigit(num[1]) && num[1]!='+' && num[1]!='-') {
            return -1;
        }
        for(i=2;i<strlen(num);i++) {
            if(!isdigit(num[i])) {
                return -1;
            }
        }
    }
    return 1;
}

void validateGlobalsMalloc(GlobalCounters *ptr) {
    if (ptr==NULL) {
        printf("\nError while allocating new global counter node\n");
        exit(-1);
    }
}