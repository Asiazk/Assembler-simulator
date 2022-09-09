#include "firstPass.h"

/* build label table and calculate addresses
   counting lines for creating a table with addresses and binary code
   input-am file
   parameters-array of pointers of labels, array of pointers to entry lables, array of pointers to extern lables
   if there are extern or entry lables, add to extern or entry table
   excess white spaces (tab\space) are always removed
   return-number of errors occuring in first pass */
int firstPass(FILE *amFile, LabelStruct *labelPtrArray[], EntryTable *entryPtrArray[],
              ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr) {
    char checkStrings[MAX_LINE_SIZE]={0};
    char firstOperand[OPERAND_SIZE]={0};
    char secondOperand[OPERAND_SIZE]={0};
    char lowerCasedCmd[NAME_SIZE]={0};
    char operatorName[NAME_SIZE]={0};
    char noComma[NAME_SIZE]={0};
    char labelNameWord[MAX_LABEL_NAME]={0};
    char dataContent[MAX_LINE_SIZE]={0};
    char structContent[MAX_LINE_SIZE]={0};
    char strToDataContent[MAX_LINE_SIZE]={0};
    char originalStruct[MAX_LINE_SIZE]={0};
    char checkStringData[MAX_LINE_SIZE]={0};

    char delimit[]=" \t\r\n\v\f";
    char delimitForNumbers[]=", \t\r\n\v\f";
    char delimitEndOfSen[]="\r\n\v\f";

    char *word;
    int gotLabelName=0, reachedDotOp1=0, reachedDotOp2=0;
    int lineCounter=0, labelPtrIndex=0, entryPtrIndex=0,  externPtrIndex=0;
    int errorTrack=0, numbersInData=0, addressTrack=0;
    int IC=0, DC=0, illegalNum=0;
    int possibleLabel, stringSize, operatorNum, operandNum, i;

    LabelStruct *currLabelPtr=NULL;
    EntryTable *currEntryPtr=NULL;
    ExternTable *currExternPtr=NULL;
    
    /* get sentence into buffer and read words */
    while (fgets(checkStrings,MAX_LINE_SIZE,amFile)) {
        lineCounter++;
        numbersInData=0;
        gotLabelName=0;
        clearString(labelNameWord,MAX_LABEL_NAME);
        clearString(lowerCasedCmd,NAME_SIZE);
        if (!isspace(checkStrings[0])) {
            word=strtok(checkStrings,delimit);
            removeSpaces(word);
            if (word[0]==';') { /* ignore comments */
                continue;
            }
            if ((possibleLabel=isDataPart(word))==0) { /* make sure word is not data */
                possibleLabel=isLabelValid(word);
                if (possibleLabel==1) { /* found new label */
                    strcpy(labelNameWord,word);
                    removeLastChar(labelNameWord);
                    if (labelPtrIndex>0) {
                        if(checkLabelName(labelPtrArray,labelPtrIndex,labelNameWord)==1) {
                            printError("label name already exists",lineCounter);
                            errorTrack++;
                        }
                    }
                    currLabelPtr=(LabelStruct*)malloc(sizeof(LabelStruct)); /* create label node and save in label pointer array */
                    validateLabelMalloc(currLabelPtr,lineCounter);
                    labelPtrArray[labelPtrIndex]=currLabelPtr;
                    labelPtrIndex++;
                    strcpy(currLabelPtr->labelName,labelNameWord);
                    gotLabelName=1;
                    currLabelPtr->address=addressTrack;
                    currLabelPtr->line=lineCounter;

                    word=strtok(NULL,delimit); /* now word contains operation: instruction or data */
                    if (!word) {
                        printError("label without definition", lineCounter);
                        errorTrack++;
                        continue;
                    }
                }
            }
            
            strcpy(lowerCasedCmd,word);
            toLowerString(lowerCasedCmd);
            if ((operatorNum=findCommandIndex(lowerCasedCmd))!=-1) { /* a command is found */
                strcpy(operatorName,lowerCasedCmd);
                if (operatorNum==14 || operatorNum==15) { /* in this case, next token might be null so I separated these commands from others */
                    word=strtok(NULL,delimitEndOfSen); /* now word is a whole sentence */
                    if (!word) {
                        if (gotLabelName==0) { /* this means we haven't allocated node for new label */
                            currLabelPtr=(LabelStruct*)malloc(sizeof(LabelStruct));
                            validateLabelMalloc(currLabelPtr,lineCounter);
                            labelPtrArray[labelPtrIndex]=currLabelPtr;
                            labelPtrIndex++;
                            strcpy(currLabelPtr->labelName,"empty label");
                            currLabelPtr->address=addressTrack;
                            
                        }
                        strcpy(currLabelPtr->operation,operatorName);
                        strcpy(currLabelPtr->operands,"no operands");
                        currLabelPtr->line=lineCounter;
                        IC++; /* any command needs at least 1 line */
                        addressTrack++;
                        continue;
                    }
                    if (word) {
                        if(checkWhiteSpaces(word)==1) { /* white spaces after commands hlt\rts is ok */
                            if (gotLabelName==0) {
                                currLabelPtr=(LabelStruct*)malloc(sizeof(LabelStruct));
                                validateLabelMalloc(currLabelPtr,lineCounter);
                                labelPtrArray[labelPtrIndex]=currLabelPtr;
                                labelPtrIndex++;
                                strcpy(currLabelPtr->labelName,"empty label");
                                currLabelPtr->address=addressTrack; /* assign address only if there's no label */ 
                            }
                            strcpy(currLabelPtr->operation,operatorName);
                            strcpy(currLabelPtr->operands,"no operands");
                            currLabelPtr->line=lineCounter;
                            IC++; /* any command needs at least 1 line */
                            addressTrack++;
                            continue;
                        }
                        else {
                            printError("no operands allowed after commands rts/hlt",lineCounter);
                            errorTrack++;
                            continue;
                        }
                    }
                }
                else if (operatorNum!=-1) {
                    if (gotLabelName==0) {
                        currLabelPtr=(LabelStruct*)malloc(sizeof(LabelStruct));
                        validateLabelMalloc(currLabelPtr,lineCounter);
                        labelPtrArray[labelPtrIndex]=currLabelPtr;
                        labelPtrIndex++; 
                        strcpy(currLabelPtr->labelName,"empty label");
                        currLabelPtr->address=addressTrack;
                    }
                    strcpy(currLabelPtr->operation,operatorName);
                    currLabelPtr->line=lineCounter;
                    IC++; /* any command needs at least 1 line */
                    addressTrack++;
                }
                
                word=strtok(NULL,delimitEndOfSen);/* word contains the whole command sentence */
                if (!word) {
                    printError("no operands in command which should be with at least one operand",lineCounter);
                    errorTrack++;
                    continue;
                }
                
                removeSpaces(word);
                strcpy(currLabelPtr->operands,word); /* only not null operands */
                
                divideInstruction(word,firstOperand,secondOperand);

                /* if command instruction is valid, continue counting lines */
                if(isInstructionValid(operatorName,word,lineCounter)!=-1) {
                    /* counting extra lines: each operand needs its line and if there's a dot-another line is needed */
                    operandNum=countOperands(word);
                    IC+=operandNum;
                    addressTrack+=operandNum;
                    
                    reachedDotOp1=findChar(firstOperand,'.');
                    if (reachedDotOp1==1) {
                        IC++;
                        addressTrack++;
                    }
                    reachedDotOp2=findChar(secondOperand,'.');
                    if (reachedDotOp2==1) {
                        IC++;
                        addressTrack++;
                    }

                    removeFirstChar(secondOperand,noComma);
                    if (isRegister(firstOperand)!=-1 && isRegister(noComma)!=-1) { /* in case both operands are registers-they share a 1 extra line */
                        IC--;
                        addressTrack--;
                    }

                    clearString(firstOperand,OPERAND_SIZE);
                    clearString(secondOperand,OPERAND_SIZE);
                    clearString(noComma,NAME_SIZE);
                    continue;
                }

                else {
                    printError("invalid number of operands",lineCounter);
                    clearString(firstOperand,OPERAND_SIZE);
                    clearString(secondOperand,OPERAND_SIZE);
                    clearString(noComma,NAME_SIZE);
                    errorTrack++;
                }
            }

            if (strcmp(word,".data")==0) {
                word=strtok(NULL,delimitEndOfSen);
                if (!word) { /* check if next token is null */
                    printError("no data was inserted", lineCounter);
                    errorTrack++;
                    continue;
                }
                if (checkWhiteSpaces(word)==1) { /* after data definition found only white spaces */
                    printError("no data was inserted",lineCounter);
                    errorTrack++;
                    continue;
                }
                removeSpaces(word);
                
                strcpy(strToDataContent,word); /* save the sentence before checking it with strtok */
                strcpy(dataContent,word);
                strcat(strToDataContent," ");  /* mark for strtok in second pass */

                if (findChar(word,',')==1) {
                    word=strtok(dataContent,delimitForNumbers);
                }

                while (word) {
                    if(isNumValid(word)==-1) {
                        illegalNum++;
                        break;
                    }
                    else {
                        numbersInData++; /* counting numbers */              
                    }
                    word=strtok(NULL,delimitForNumbers);   
                }
                if (illegalNum!=0) {
                    printError("invalid number in data definition",lineCounter);
                    errorTrack++;
                    illegalNum=0;
                    clearString(dataContent,MAX_LINE_SIZE);
                    clearString(strToDataContent,MAX_LINE_SIZE);
                    continue;
                }
                if (countChar(strToDataContent,',')!=numbersInData-1) {
                    printError("invalid number of commas",lineCounter);
                    errorTrack++;
                    clearString(dataContent,MAX_LINE_SIZE);
                    clearString(strToDataContent,MAX_LINE_SIZE);
                    continue;;
                }
                /* after we checked all numbers we save the operands */
                if (gotLabelName==0) {
                    currLabelPtr=(LabelStruct*)malloc(sizeof(LabelStruct));
                    validateLabelMalloc(currLabelPtr,lineCounter);
                    labelPtrArray[labelPtrIndex]=currLabelPtr;
                    labelPtrIndex++;
                    strcpy(currLabelPtr->labelName,"empty label");
                    currLabelPtr->address=addressTrack;
                }
                strcpy(currLabelPtr->operands,strToDataContent);
                strcpy(currLabelPtr->operation,".data");
                currLabelPtr->line=lineCounter;

                DC+=numbersInData;
                addressTrack+=numbersInData;
                clearString(dataContent,MAX_LINE_SIZE);
                clearString(strToDataContent,MAX_LINE_SIZE);
                continue;
            }

            if (strcmp(word,".string")==0) {
                word=strtok(NULL,delimitEndOfSen);
                if (!word) { /* check if next token is null */
                    printError("no data was inserted", lineCounter);
                    errorTrack++;
                    continue;
                }
                
                if (word[0]=='\"') {
                    removeFirstChar(word,dataContent);
                }
                else if (word[0]==' ' || word[0]=='\t') { /* ignore white spaces at start */
                    removeSpacesBeforeChar(word,checkStringData,'\"');
                    removeFirstChar(checkStringData,dataContent);
                }
                else {
                    printError("no starting quotes in string definition", lineCounter);
                    errorTrack++;
                    continue;
                }

                if (handleStringOperand(dataContent)==-1) {
                    printError("no closing quotes in string definition",lineCounter);
                    errorTrack++;
                    continue;
                }

                stringSize=strlen(dataContent)+1; /* plus one for '\0' */
                /*strcat(dataContent,"0");*/
                if (gotLabelName==0) {
                    currLabelPtr=(LabelStruct*)malloc(sizeof(LabelStruct));
                    validateLabelMalloc(currLabelPtr,lineCounter);
                    labelPtrArray[labelPtrIndex]=currLabelPtr;
                    labelPtrIndex++; /* this represents how many labels are in array */
                    strcpy(currLabelPtr->labelName,"empty label");
                    currLabelPtr->address=addressTrack;
                }
                strcpy(currLabelPtr->operands,dataContent);
                strcpy(currLabelPtr->operation,".string");
                currLabelPtr->line=lineCounter;
                clearString(dataContent,MAX_LINE_SIZE);
                clearString(checkStringData,MAX_LINE_SIZE);

                DC+=stringSize;
                addressTrack+=stringSize;
                continue;
            }

            if (strcmp(word,".struct")==0) { /* struct: num,"string" */
                word=strtok(NULL,delimitEndOfSen);
                if (!word) { /* check if next token is null */
                    printError("no data was inserted", lineCounter);
                    errorTrack++;
                    continue;
                }

                strcpy(originalStruct,word);
                strcpy(dataContent,word);

                word=strtok(dataContent,",");/* first operand must be a number */
                if (!word) {
                    printError("no number was inserted in struct",lineCounter);
                    errorTrack++;
                    continue;
                }
                removeSpaces(word);
                if(isNumValid(word)==-1) {
                    printf("invalid number in struct\n");
                    errorTrack++;
                    continue;
                }
                else {
                    numbersInData++;
                }
                strcat(structContent,word);
                strcat(structContent,",");

                word=strtok(originalStruct,"\"");
                if (!word) {
                    printError("no string was inserted in struct",lineCounter);
                    errorTrack++;
                    continue;
                }

                word=strtok(NULL,delimitEndOfSen); /* now word is the string with second quotes */
                if (!word) {
                    printError("empty string in struct",lineCounter);
                    errorTrack++;
                    continue;
                }

                if (handleStringOperand(word)==-1) {
                    printError("no closing quotes in string definition",lineCounter);
                    errorTrack++;
                    continue;
                }

                /* now in word we have the string */
                strcat(structContent,word);
                stringSize=strlen(word)+1; /* plus one for '\0' */
                strcat(structContent,"\n"); /* marking the last character to recognize with strtok in second pass */

                if (gotLabelName==0) {
                    currLabelPtr=(LabelStruct*)malloc(sizeof(LabelStruct));
                    validateLabelMalloc(currLabelPtr,lineCounter);
                    labelPtrArray[labelPtrIndex]=currLabelPtr;
                    labelPtrIndex++; /* this represents how many labels are in array */
                    strcpy(currLabelPtr->labelName,"empty label");
                    currLabelPtr->address=addressTrack;
                }
                strcpy(currLabelPtr->operands,structContent);
                strcpy(currLabelPtr->operation,".struct");
                currLabelPtr->line=lineCounter;

                DC+=stringSize;
                DC+=numbersInData;
                addressTrack+=stringSize;
                addressTrack+=numbersInData;
                clearString(structContent,MAX_LINE_SIZE);
                clearString(dataContent,MAX_LINE_SIZE);
                clearString(originalStruct,MAX_LINE_SIZE);
                continue;
            }
             
            if (strcmp(word,".extern")==0) {
                if (gotLabelName==1) { /* Yakir wrote in forum: label with extern is error */
                    printError("extern definition after label is illegal",lineCounter);
                    errorTrack++;
                    continue;
                }
                word=strtok(NULL,delimit);
                if (!word) {  /*check if next token is null*/ 
                    printError("no data was inserted", lineCounter);
                    errorTrack++;
                    continue;
                }
                if(isNameValid(word)==1) { 
                    currExternPtr=(ExternTable*)malloc(sizeof(ExternTable));
                    validateExternMalloc(currExternPtr,lineCounter);
                    strcpy(currExternPtr->content,word);
                    currExternPtr->address=-1;
                    externPtrArray[externPtrIndex]=currExternPtr;
                    externPtrIndex++;
                }
                else {
                    printError("invalid extern content",lineCounter);
                    errorTrack++;
                    continue;
                }
                word=strtok(NULL,delimit);
                if (!word) {   
                    continue;
                }
                else { /* no more words allowed */
                    if (checkWhiteSpaces(word)==1) {
                        continue;
                    }
                    else {
                        printError("invalid extern content",lineCounter);
                        errorTrack++;
                    }
                }
                continue;
            }

            if (strcmp(word,".entry")==0) {
                if (gotLabelName==1) { /* Yakir wrote in forum: label with entry is error */
                    printError("entry definition after label is illegal",lineCounter);
                    errorTrack++;
                    continue;
                }
                word=strtok(NULL,delimit);
                if (!word) {  /*check if next token is null */
                    printError("no data was inserted", lineCounter);
                    errorTrack++;
                    continue;
                }
                if(isNameValid(word)==1) {
                    currEntryPtr=(EntryTable*)malloc(sizeof(EntryTable));
                    validateEntryMalloc(currEntryPtr,lineCounter);
                    strcpy(currEntryPtr->content,word);
                    currEntryPtr->address=-1;
                    entryPtrArray[entryPtrIndex]=currEntryPtr;
                    entryPtrIndex++;
                }
                else {
                    printError("invalid entry content",lineCounter);
                    errorTrack++;
                    continue;
                }

                word=strtok(NULL,delimit);
                if (!word) {   
                    continue;
                }
                else { /* no more words allowed */
                    if (checkWhiteSpaces(word)==1) {
                        continue;
                    }
                    else {
                        printError("invalid entry content",lineCounter);
                        errorTrack++;
                    }
                }
                continue;
            }
            else {            
                printError("invalid label name", lineCounter);
                errorTrack++;
                continue;
            }
        }
    }

    globalCountersPtr->dataCounter=DC;
    globalCountersPtr->instCounter=IC;
    globalCountersPtr->labelListSize=labelPtrIndex;
    globalCountersPtr->entryTableSize=entryPtrIndex;
    globalCountersPtr->externTableSize=externPtrIndex;

    for (i=0; i<globalCountersPtr->labelListSize; i++) {
        currLabelPtr=labelPtrArray[i];
        if (!(isDataPart(currLabelPtr->operation))) {
            operatorNum=validateOperandType(currLabelPtr->operation,currLabelPtr->operands,labelPtrArray,entryPtrArray,externPtrArray,globalCountersPtr);
            if (operatorNum==-1) {
                printError("invalid operand type",currLabelPtr->line);
                errorTrack++; 
            }
        }
    }

    if (DC==0 && IC==0) {
        printf("Warning: no instructions or data in file\n");
        errorTrack++;
    }

    /* I used this function to see the label list is correct
    printLabelList(labelPtrArray,globalCountersPtr->labelListSize);*/
    
    return errorTrack; 
}

/* check the given word and remove quotes. return 1 if characters removed, else-return -1 */
int handleStringOperand(char *word) {
    if (findLastQuotes(word)==1) {
        removeLastChar(word);
        return 1;
    }
    
    else if (word[strlen(word)-1]==' ' || word[strlen(word)-1]=='\t') {
        while (word[strlen(word)-1]!='"') {
            removeLastChar(word);
        }
        removeLastChar(word);/* remove closing quotes */
        return 1;
    } 
    
    return -1;
}

int isInstructionValid(char *operator, char *sentence, int line) {
    int numOfOperands=countOperands(sentence);
    
    switch(findCommandIndex(operator)){
        /*the 1st group - operator and 2 operands*/
        case 0: case 1: case 2:
        case 3: case 6:
        
            if (numOfOperands!=2) {
                return -1;
            }
            return 1;
        
        /*the 2nd group - operator and 1 operand*/
        case 4: case 5: case 7: case 8: case 9:
        case 10: case 11: case 12: case 13:

            if (numOfOperands!=1) {
                return -1;
            }
            return 1;

       /*case 14+15 the 3rd group with no operands is handled during first pass*/
    }
    return -1;    
}

void validateEntryMalloc(EntryTable *ptr, int line) {
    if (ptr==NULL) {
        printError("error while allocating new entry node",line);
        exit(-1);
    }
}

/* print content of label list
   I used this function to check label list is correct */
void printLabelList(LabelStruct *labelPtrArray[], int listSize){
    int i;

    if (listSize!=0) {
        for (i=0; i<listSize; i++) {
                printf("\nname:%s\noperation:%s\nadress:%d\noperands(sentence):%s\n\n", 
                labelPtrArray[i]->labelName,labelPtrArray[i]->operation,labelPtrArray[i]->address,labelPtrArray[i]->operands);
        }
    }
    else {
        printf("\nlabel list is empty\n");
    }  
}

/* check if operands are of the legal type, if so-return 1, else-return -1 */
int validateOperandType(char *operation, char *operands, LabelStruct *labelList[],
                        EntryTable *entryPtrArray[], ExternTable *externPtrArray[],
                        GlobalCounters *globalCountersPtr) {
    char firstOperand[MAX_LABEL_NAME]={0};
    char temp[MAX_LABEL_NAME]={0};
    char secondOperand[MAX_LABEL_NAME]={0};

    if (findChar(operands,',')==1) {
        divideInstruction(operands,firstOperand,temp);
        removeFirstChar(temp,secondOperand);
    }
    if (findChar(firstOperand,'.')==1) {
        removeLastChar(firstOperand);
        removeLastChar(firstOperand);
    }
    if (findChar(secondOperand,'.')==1) {
        removeLastChar(secondOperand);
        removeLastChar(secondOperand);
    }

    switch(findCommandIndex(operation)) {
        case 0: case 3:
            if (getOperandAddressType(secondOperand,labelList,entryPtrArray,externPtrArray,globalCountersPtr)==0) {
                return -1;
            }
            break;
        case 6:
            if (getOperandAddressType(firstOperand,labelList,entryPtrArray,externPtrArray,globalCountersPtr)==0 || 
                getOperandAddressType(firstOperand,labelList,entryPtrArray,externPtrArray,globalCountersPtr)==3) {
                return -1;
            }
            if (getOperandAddressType(secondOperand,labelList,entryPtrArray,externPtrArray,globalCountersPtr)==0) {
                return -1;
            }
            break;
        case 4: case 5: case 7:
        case 8: case 9: case 10: case 11: case 13:
            if (getOperandAddressType(operands,labelList,entryPtrArray,externPtrArray,globalCountersPtr)==0) {
                return -1;
            }
            break;
    }
    return 1;
}

/* operands can be:
    -name of label
    -number with '#' or just valid number
    -register name
   first divide sentence from lable list to 2 operands and check each operand separately
   first remove what's after the dot and search the struct in label list */
int getOperandAddressType(char *operand, LabelStruct *labelList[], EntryTable *entryPtrArray[],
                          ExternTable *externPtrArray[],GlobalCounters *globalCountersPtr) {
    if (isRegister(operand)!=-1) {
        return 3;
    }
    
    /* if the operand is name of label-return 1 */
    if (checkLabelName(labelList,globalCountersPtr->labelListSize,operand) &&
        checkStruct(labelList,operand,globalCountersPtr)==-1) {
        return 1;
    }

    /* more label names in extern and entry table */
    if (checkLabelARE(operand,entryPtrArray,externPtrArray,globalCountersPtr)!=2) {
        return 1;
    }

    if (checkStruct(labelList,operand,globalCountersPtr)==1 && checkNumOperand(operand)!=0) {
        return 2;
    }

    /* if operand is valid number with # in begining return 0 */
    if (checkNumOperand(operand)) {
        return 0;
    }

    return -1; /* invalid operand */
}

void validateLabelMalloc(LabelStruct *ptr, int line) {
    if (ptr==NULL) {
        printError("error while allocating new label node",line);
        exit(-1);
    }
}

/* check given word is a label-ends with ':' and less than 30 characters. if so-return if the name is valid, else-return 0 */
int isLabelValid(char *label) {
    char str[MAX_LABEL_NAME];

    if (strlen(label) > MAX_LABEL_NAME) {
        return 0;
    }
    if (label[strlen(label)-1]!=':') {
        return 0;
    }
    else {
        strcpy(str, label);
        str[strlen(str)-1] = '\0';
        return isNameValid(str);
    }
}