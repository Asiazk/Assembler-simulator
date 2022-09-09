#include "secondPass.h"

/* turn label list to address and binary code list
   print to ob file the 32 base of address&code table
   return number of errors */
int secondPass(AddressAndCode *adCodeTable[], LabelStruct *labelList[],
               EntryTable *entryPtrArray[], ExternTable *externPtrArray[],
               GlobalCounters *globalCountersPtr, FILE *obFile, FILE *extFile) {
    int errors;

    errors=labelTableToBinary(adCodeTable,labelList,entryPtrArray,externPtrArray,globalCountersPtr,extFile);
    if (errors>0) { /* errors are found during encoding to binary, can't move to next function */
        return errors;
    }
    globalCountersPtr->addressCodeSize=(globalCountersPtr->instCounter)+(globalCountersPtr->dataCounter);
    /* I used this function to check address&code table is correct
    printBinaryList(adCodeTable,globalCountersPtr);*/
    printToOb(adCodeTable,globalCountersPtr,obFile);/* write into .ob file with translation to base 32 with symbols */
    return errors;
}

/* turn label list to address and binary code list
   return number of errors */
int labelTableToBinary(AddressAndCode *adCodeTable[], LabelStruct *labelList[],
                       EntryTable *entryPtrArray[], ExternTable *externPtrArray[],
                       GlobalCounters *globalCountersPtr, FILE *extFile) {
    int i;
    int errors;
    int errorTrack=0;
    
    /* translate label table to binary */
    for (i=0; i<globalCountersPtr->labelListSize; i++) {
        errors=encodeInstruction(labelList[i]->operation,labelList[i]->operands,adCodeTable,labelList,entryPtrArray,externPtrArray,globalCountersPtr,labelList[i]->line,labelList[i]->address,extFile);
        errorTrack+=errors;
    }
    
    if (errors>0) {
        printf("Errors occured while creating binary codes\n");
        return errorTrack;
    }
    return errorTrack;
}

/* encode operand with a dot. operand must be a struct. if so-retrun 1, else-return -1 */
int encodeOperandWithDot(char *operand, LabelStruct *labelList[],
                      EntryTable *entryPtrArray[], ExternTable *externPtrArray[],
                      GlobalCounters *globalCountersPtr, char tenBits[],int line) {
    char *binaryARE;
    char structBinaryAddress[NUM_OF_ADDRESS_BITS]={0};
    int labelAddress;

    if (checkStruct(labelList,operand,globalCountersPtr) == -1) {
        printError("No such struct",line);
        return -1;
    }
    /* before the dot it must be a struct label */
    labelAddress = checkLabelAddress(labelList,globalCountersPtr,operand);           
    if (labelAddress==-1) {
        printError("No label was found",line);
        return -1;
    }
    decimalTo8LSB(labelAddress, structBinaryAddress); 
    
    binaryARE=getLabelCodeType(operand,entryPtrArray,externPtrArray,globalCountersPtr);
    if (strcmp(binaryARE,"-1")==0) {
        printError("invalid operand",line);
        return -1;
    }
    strcat(tenBits,structBinaryAddress);
    strcat(tenBits,binaryARE);

    return 1;
}

/* check if operand after a dot is 1 or 2 and encode number to binary
   if number if valid return 1,else-return -1 */
int encodeStructOperandNum(char *operand, char tenBits[], int line) {
    int num;
    char binaryAddress[NUM_OF_ADDRESS_BITS]={0};

    /* after the dot it must be a number-create new line for number */
    num=atoi(operand);
    if (num!=1 && num!=2) {
        printError("invalid struct field number,must be 1 or 2",line);
        return -1;
    }
    decimalTo8LSB(num, binaryAddress);
    strcat(tenBits,binaryAddress);
    strcat(tenBits,"00"); /* ARE of a line of binary number is always 00 */

    return 1;
}

/* building first binary line of instruction. if all operands ate correct-return 1, else-return -1 */
int encodeFirstLine(char *operand, char *beforeDot, char *afterDot,
                          LabelStruct *labelList[], EntryTable *entryPtrArray[],
                          ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr, char opBits[]) {
    char *binaryType2Bits;
    int dotExists = splitByDotOperands(operand, beforeDot, afterDot);
    int returnValue = 1;
    if (dotExists==1) {    
        binaryType2Bits = getOperandTypeFirstLine(beforeDot,labelList,entryPtrArray,externPtrArray,globalCountersPtr);
        if (strcmp(binaryType2Bits,"-1")==0) {
            returnValue = -1;
        }
        else {
            strcat(opBits,binaryType2Bits);
        }
    }
    else {
        /* finding the address type of first operand */
        binaryType2Bits = getOperandTypeFirstLine(operand,labelList,entryPtrArray,externPtrArray,globalCountersPtr);
        if (strcmp(binaryType2Bits,"-1")==0) {
            returnValue = -1;
        }
        else {
            strcat(opBits,binaryType2Bits);
        }
    }
    clearString(operand,MAX_OPERAND_LENGTH);
    clearString(beforeDot,MAX_OPERAND_LENGTH);
    clearString(afterDot,MAX_OPERAND_LENGTH);

    return returnValue;
}

/* build the first binary line, search for labels\registers\numbers and build next lines
   return number of errors */
int encodeInstruction(char *operator, char *sentence, AddressAndCode *addCodeTable[], 
                      LabelStruct *labelList[], EntryTable *entryPtrArray[], 
                      ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr,
                      int line, int addressCounter, FILE *extFile) {

    char opBits[SIZE_OF_BYTE] = {0};
    char firstOperand[MAX_OPERAND_LENGTH] = {0};
    char secondOperand[MAX_OPERAND_LENGTH] = {0};
    char secondOpWithoutComma[MAX_OPERAND_LENGTH] = {0};
    char first[MAX_OPERAND_LENGTH]={0};
    char second[MAX_OPERAND_LENGTH]={0};
    char tenBits[SIZE_OF_BYTE]={0};
    char *binaryReg1, *binaryReg2, *binaryType2Bits;
    int reg1, reg2, commandFound, dotExists, numOfOperands, dotOperationRes;
    int errorTrack=0;

    commandFound = findCommandIndex(operator);
    if (commandFound==14 || commandFound==15) {
        strcat(opBits,BINARY_COMMANDS[commandFound]);
        strcat(opBits,"000000");
        createNewBinaryIntsLine(opBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
        return errorTrack;
    }
    if (commandFound != -1) { /* operator is one of 16 commands */
        strcat(opBits,BINARY_COMMANDS[commandFound]);/* 4 first bits is opcode */
        numOfOperands = countOperands(sentence);
        if (numOfOperands == 2) {
            divideInstruction(sentence,firstOperand,secondOperand);
            removeFirstChar(secondOperand,secondOpWithoutComma);
            /* special case of both operands being registers */
            if ((reg1 = isRegister(firstOperand))!= -1 && (reg2 = isRegister(secondOpWithoutComma))!= -1) {
                strcat(opBits,"1111");/* next bits are address type of each register-11 */
                strcat(opBits,"00");/* ARE of first binary line of instruction if always 00 */
                createNewBinaryIntsLine(opBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                addressCounter++;
                /* next binary line for both registers */
                binaryReg1 = (char*)BINARY_REGISTERS[reg1];
                binaryReg2 = (char*)BINARY_REGISTERS[reg2];
                
                strcat(tenBits,binaryReg1);
                strcat(tenBits,binaryReg2);
                strcat(tenBits,"00"); /* ARE in case of 2 registers is always 00 */
                createNewBinaryIntsLine(tenBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                return errorTrack;
            }

            /* encoding first binary line */
            dotOperationRes = encodeFirstLine(firstOperand, first, second, labelList, entryPtrArray, externPtrArray, globalCountersPtr, opBits);
            if (dotOperationRes==-1) {
                printError("operand not found",line);
                errorTrack++;
                return errorTrack;
            }
            dotOperationRes = encodeFirstLine(secondOpWithoutComma, first, second, labelList, entryPtrArray, externPtrArray, globalCountersPtr, opBits);
            if (dotOperationRes==-1) {
                printError("operand not found",line);
                errorTrack++;
                return errorTrack;
            }
            strcat(opBits,"00"); /* ARE of first line is always 00 */
            createNewBinaryIntsLine(opBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
            addressCounter++;

            /* now encoding next lines */
            divideInstruction(sentence,firstOperand,secondOperand);
            removeFirstChar(secondOperand,secondOpWithoutComma);
            /* if first operand contains a dot, need to encode more binary lines */
            dotExists = splitByDotOperands(firstOperand,first,second);
            if (dotExists==1) {
                dotOperationRes=encodeOperandWithDot(first,labelList,entryPtrArray,externPtrArray,globalCountersPtr,tenBits,line);
                if (dotOperationRes==-1) {
                    errorTrack++;
                    return errorTrack;
                }
                createNewBinaryIntsLine(tenBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                addressCounter++;
                clearString(tenBits, SIZE_OF_BYTE);
                /* after the dot it must be a number-check it and create new line for number */
                dotOperationRes=encodeStructOperandNum(second,tenBits,line);
                if (dotOperationRes==-1) {
                    errorTrack++;
                    return errorTrack;
                }
                createNewBinaryIntsLine(tenBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                addressCounter++;

                clearString(tenBits, SIZE_OF_BYTE);
                clearString(first,MAX_OPERAND_LENGTH);
                clearString(second,MAX_OPERAND_LENGTH);
            }
            else {
                if (getOperandNextLines(firstOperand,tenBits,addCodeTable,entryPtrArray,externPtrArray,labelList,globalCountersPtr,1,addressCounter,extFile) != -1) {
                    createNewBinaryIntsLine(tenBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                    addressCounter++;
                    clearString(tenBits, SIZE_OF_BYTE);
                }
                else {
                    printError("invalid operand",line);
                    errorTrack++;
                    return errorTrack;
                }
            }
            
            /* if second operand contains a dot, need to encode more binary lines */
            dotExists=splitByDotOperands(secondOpWithoutComma, first, second);
            if (dotExists==1) {
                dotOperationRes=encodeOperandWithDot(first,labelList,entryPtrArray,externPtrArray,globalCountersPtr,tenBits,line);
                if (dotOperationRes==-1) {
                    errorTrack++;
                    return errorTrack;
                }
                createNewBinaryIntsLine(tenBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                addressCounter++;
                clearString(tenBits, SIZE_OF_BYTE);
                /* after the dot it must be a number-check it and create new line for number */
                dotOperationRes=encodeStructOperandNum(second,tenBits,line);
                if (dotOperationRes==-1) {
                    errorTrack++;
                    return errorTrack;
                }
                createNewBinaryIntsLine(tenBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                addressCounter++;

                clearString(tenBits, SIZE_OF_BYTE);
                clearString(first,MAX_OPERAND_LENGTH);
                clearString(second,MAX_OPERAND_LENGTH);
            }
            else {
                if (getOperandNextLines(secondOpWithoutComma,tenBits,addCodeTable,entryPtrArray,externPtrArray,labelList,globalCountersPtr,2,addressCounter,extFile) != -1) {
                    createNewBinaryIntsLine(tenBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                    addressCounter++;
                }
                else {
                    printError("invalid operand",line);
                    errorTrack++;
                    return errorTrack;
                }
            }   
        }
        if (numOfOperands == 1) {
            /* encoding first binary line */
            /* in case of one operand, first address type is always 00 */
            strcat(opBits,"00");

            dotExists = splitByDotOperands(sentence,first,second);
            if (dotExists==1) {
                binaryType2Bits = getOperandTypeFirstLine(first,labelList,entryPtrArray,externPtrArray,globalCountersPtr);
                if (strcmp(binaryType2Bits,"-1")==0) {
                    printError("operand not found",line);
                    errorTrack++;
                    return errorTrack;
                }
                strcat(opBits,binaryType2Bits);
            }
            else {
                /* finding the address type of operand */
                binaryType2Bits = getOperandTypeFirstLine(sentence,labelList,entryPtrArray,externPtrArray,globalCountersPtr);
                if (strcmp(binaryType2Bits,"-1")==0) {
                    printError("operand not found",line);
                    errorTrack++;
                    return errorTrack;
                }
                strcat(opBits,binaryType2Bits);
            }
            clearString(firstOperand,MAX_OPERAND_LENGTH);
            clearString(first,MAX_OPERAND_LENGTH);
            clearString(second,MAX_OPERAND_LENGTH);

            strcat(opBits,"00"); /* ARE of first line is always 00 */
            createNewBinaryIntsLine(opBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
            addressCounter++;

            /* now encoding next lines */
            dotExists = splitByDotOperands(sentence, first, second);
            if (dotExists == 1) {
                dotOperationRes=encodeOperandWithDot(first,labelList,entryPtrArray,externPtrArray,globalCountersPtr,tenBits,line);
                if (dotOperationRes==-1) {
                    errorTrack++;
                    return errorTrack;
                }
                createNewBinaryIntsLine(tenBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                addressCounter++;
                clearString(tenBits, SIZE_OF_BYTE);

                /* after the dot it must be a number-check it and create new line for number */
                dotOperationRes=encodeStructOperandNum(second,tenBits,line);
                if (dotOperationRes==-1) {
                    errorTrack++;
                    return errorTrack;
                }
                createNewBinaryIntsLine(tenBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                addressCounter++;

                clearString(tenBits, SIZE_OF_BYTE);
                clearString(first,MAX_OPERAND_LENGTH);
                clearString(second,MAX_OPERAND_LENGTH);
            }
            else {
                if (getOperandNextLines(sentence,tenBits,addCodeTable,entryPtrArray,externPtrArray,labelList,globalCountersPtr,2,addressCounter,extFile) != -1) {
                createNewBinaryIntsLine(tenBits,addCodeTable,labelList,globalCountersPtr,line,addressCounter);
                addressCounter++;
                }
                else {
                    printError("invalid operand",line);
                    errorTrack++;
                    return errorTrack;
                }
            }
        }
    }

    if (isDataPart(operator) == 1) {
        if (strcmp(operator, ".data") == 0) {
            numDataToBinaryLines(sentence,addCodeTable,labelList,globalCountersPtr,addressCounter,line);
        }
        else if (strcmp(operator, ".string") == 0) {
            stringToBinaryLines(sentence,addCodeTable,labelList,globalCountersPtr,addressCounter,line);
        } 
        else if (strcmp(operator, ".struct") == 0) { 
            structToBinaryLines(sentence,addCodeTable,labelList,globalCountersPtr,addressCounter,line);
        }
    }
    return errorTrack;
}

/*ARE type - for labels*/
char *getLabelCodeType(char *name, EntryTable *entryPtrArray[], ExternTable *externPtrArray[],
                       GlobalCounters *globalCountersPtr) {
    int res = checkLabelARE(name,entryPtrArray,externPtrArray,globalCountersPtr);
    switch (res) {
    case 0:
        return (char*)BINARY_ADDRESS_TYPE[0];
    case 1:
        return (char*)BINARY_ADDRESS_TYPE[1];
    case 2:
        return (char*)BINARY_ADDRESS_TYPE[2];
    default:
        return "-1";
    }
}

/* check the given operand and return the binary 2 bit type */
char *getOperandTypeFirstLine(char *operand, LabelStruct *labelList[],
                              EntryTable *entryPtrArray[], ExternTable *externPtrArray[],
                              GlobalCounters *globalCountersPtr) {
    if (isRegister(operand) != -1) {
        return (char*)BINARY_ADDRESS_TYPE[3];
    }

    /* if the operand is name of label-return 1 */
    if (checkLabelName(labelList,globalCountersPtr->labelListSize,operand)==1 &&
        checkStruct(labelList,operand,globalCountersPtr)==-1) {
        return (char*)BINARY_ADDRESS_TYPE[1];
    }

    /* search in extern\entry table-it's also a label */
    if (checkLabelARE(operand,entryPtrArray,externPtrArray,globalCountersPtr)!=2) {
        return (char*)BINARY_ADDRESS_TYPE[1];
    }

    /* if operand is struct-return 2 */
    if (checkStruct(labelList,operand,globalCountersPtr)==1) {
        return (char*)BINARY_ADDRESS_TYPE[2];
    }

    /* if operand is valid number with\without # in begining return 0 */
    if (checkNumOperand(operand) != -1) {
        return (char*)BINARY_ADDRESS_TYPE[0];
    }
    return "-1"; /* invalid operand */
}

/* encode next lines of binary code by operand type
   operandNum-when we need to handle register as an operand: if register is first operand-we get 1, if second-we get 2 */
int getOperandNextLines(char *operand, char tenBits[], AddressAndCode *addCodeTable[],
                        EntryTable *entryPtrArray[], ExternTable *externPtrArray[],
                        LabelStruct *labelList[], GlobalCounters *globalCountersPtr,
                        int operandNum, int addressCounter, FILE *extFile) {
    int labelAddress, registerNum;
    char *labelARE, *binaryReg;
    char bit8[NUM_OF_ADDRESS_BITS]={0},numNoHash[DIGIT_MAX]={0};

    if (operandNum==2) {
        if ((registerNum=isRegister(operand))!= -1) {
            binaryReg = (char*)BINARY_REGISTERS[registerNum];
            strcat(tenBits,"0000");/* when first operand is register, 4 first bits is number of register */
            strcat(tenBits,binaryReg); /* next bits are always 0 */
            strcat(tenBits,"00"); /* in this case, ARE is always 00 */
            return 1;
        }
    }

    if (operandNum==1) {
        if ((registerNum=isRegister(operand))!= -1) {
            binaryReg = (char*)BINARY_REGISTERS[registerNum];
            strcat(tenBits,binaryReg);/* when first operand is register, 4 first bits is number of register */
            strcat(tenBits,"000000"); /* next bits are always 0 */
            return 1;
        }
    }

    /* if the operand is name of label-return 1 */
    if (checkLabelName(labelList,globalCountersPtr->labelListSize,operand)) {
        labelAddress = checkLabelAddress(labelList,globalCountersPtr,operand);
        decimalTo8LSB(labelAddress, bit8);
        labelARE = getLabelCodeType(operand,entryPtrArray,externPtrArray,globalCountersPtr);
        strcat(tenBits,bit8);
        strcat(tenBits,labelARE);
        return 1;
    }

    /* if operand is valid number without # in begining return 0 */
    if (checkNumOperand(operand) != -1) {
        removeFirstChar(operand, numNoHash);
        decimalTo8LSB(atoi(numNoHash), bit8);
        strcat(tenBits,bit8);
        strcat(tenBits,"00"); /* ARE of binary line with number is always 00 */
        return 1;
    }

    /* binary of extern operand(label) is always is one
       also adding the extern label+address to extern table */
    if (checkLabelARE(operand,entryPtrArray,externPtrArray,globalCountersPtr) == 1) {
        printToExt(operand,addressCounter+ADD_TO_ADDRESS,extFile);
        strcat(tenBits,"0000000001");
        return 1;
    }
    return -1;
}


/* we get the string without "" from label table */
void stringToBinaryLines(char *str, AddressAndCode *addAndCodeList[] ,LabelStruct *labelList[],
                         GlobalCounters *globalCountersPtr, int addressCounter, int line) {
    int i;
    char tenBits[SIZE_OF_BYTE]={0};
    AddressAndCode *currentAddressAndCodePtr;

    for (i=0; i<strlen(str); i++) {
        /* turn each char ascii to binary and insert to new line */

        decimalTo10LSB(str[i],tenBits); 
        currentAddressAndCodePtr=(AddressAndCode*)malloc(sizeof(AddressAndCode));
        validateAddressAndCodeMalloc(currentAddressAndCodePtr,line);
        strcpy(currentAddressAndCodePtr->binaryCode,tenBits);
        currentAddressAndCodePtr->address=addressCounter+ADD_TO_ADDRESS;
        currentAddressAndCodePtr->type=0;
        addAndCodeList[globalCountersPtr->addressCodeInd]=currentAddressAndCodePtr;
        (globalCountersPtr->addressCodeInd)++;
        addressCounter++;
        clearString(tenBits,SIZE_OF_BYTE);
    }
    
    /* adding null terminated 0 */
    strcpy(tenBits,"0");
    currentAddressAndCodePtr=(AddressAndCode*)malloc(sizeof(AddressAndCode));
    validateAddressAndCodeMalloc(currentAddressAndCodePtr,line);
    strcpy(currentAddressAndCodePtr->binaryCode,tenBits);
    currentAddressAndCodePtr->address=addressCounter+ADD_TO_ADDRESS;
    currentAddressAndCodePtr->type=0;
    addAndCodeList[globalCountersPtr->addressCodeInd]=currentAddressAndCodePtr;
    (globalCountersPtr->addressCodeInd)++;
}

/* in first pass we already validated all the numbers in each .data */
void numDataToBinaryLines(char *str, AddressAndCode *addAndCodeList[] ,LabelStruct *labelList[],
                          GlobalCounters *globalCountersPtr, int addressCounter, int line) {
    int value;
    char tenBits[SIZE_OF_BYTE]={0};
    char nums[MAX_LINE_SIZE]={0};
    char delimitForNumbers[]=", ";
    char *numInArr;
    AddressAndCode *currentAddressAndCodePtr;

    strcpy(nums,str);
    numInArr=strtok(nums,delimitForNumbers);

    while (numInArr) {
        value=atoi(numInArr);
        decimalTo10LSB(value,tenBits);
        currentAddressAndCodePtr=(AddressAndCode*)malloc(sizeof(AddressAndCode));
        validateAddressAndCodeMalloc(currentAddressAndCodePtr,line);
        strcpy(currentAddressAndCodePtr->binaryCode,tenBits);
        currentAddressAndCodePtr->address=addressCounter+ADD_TO_ADDRESS;
        currentAddressAndCodePtr->type=0;
        addAndCodeList[globalCountersPtr->addressCodeInd]=currentAddressAndCodePtr;
        (globalCountersPtr->addressCodeInd)++;
        addressCounter++;
        clearString(tenBits,SIZE_OF_BYTE);

        numInArr=strtok(NULL,delimitForNumbers); /* get next number */
    }
}

/* in first pass we already checked:
   -first field to be a number
   -second field to be a string
   -extra characters after second " or invalid number of quotes */
void structToBinaryLines(char *str, AddressAndCode *addAndCodeList[] ,LabelStruct *labelList[],
                         GlobalCounters *globalCountersPtr, int addressCounter, int line) {
    int value, i;
    char *word;
    char tenBits[SIZE_OF_BYTE]={0};
    char sentence[MAX_LINE_SIZE]={0};
    AddressAndCode *currentAddressAndCodePtr;

    strcpy(sentence,str);
    word=strtok(sentence,","); /* now word is a number */
    
    value=atoi(word);
    decimalTo10LSB(value,tenBits);
    currentAddressAndCodePtr=(AddressAndCode*)malloc(sizeof(AddressAndCode));
    validateAddressAndCodeMalloc(currentAddressAndCodePtr,line);
    strcpy(currentAddressAndCodePtr->binaryCode,tenBits);
    currentAddressAndCodePtr->address=addressCounter+ADD_TO_ADDRESS;
    currentAddressAndCodePtr->type=0;
    addAndCodeList[globalCountersPtr->addressCodeInd]=currentAddressAndCodePtr;
    (globalCountersPtr->addressCodeInd)++;
    addressCounter++;
    
    word=strtok(NULL,"\n"); /* now word is a string */

    for (i=0; i<strlen(word); i++) {
        /* turn each char ascii to binary and insert to new line */
        decimalTo10LSB(word[i],tenBits); 
        currentAddressAndCodePtr=(AddressAndCode*)malloc(sizeof(AddressAndCode));
        validateAddressAndCodeMalloc(currentAddressAndCodePtr,line);
        strcpy(currentAddressAndCodePtr->binaryCode,tenBits);
        currentAddressAndCodePtr->address=addressCounter+ADD_TO_ADDRESS;
        currentAddressAndCodePtr->type=0;
        addAndCodeList[globalCountersPtr->addressCodeInd]=currentAddressAndCodePtr;
        (globalCountersPtr->addressCodeInd)++;
        addressCounter++;
        clearString(tenBits,SIZE_OF_BYTE);
    }
    /* adding null terminated 0 - ascii of zero is 48*/
    strcpy(tenBits,"0"); 
    currentAddressAndCodePtr=(AddressAndCode*)malloc(sizeof(AddressAndCode));
    validateAddressAndCodeMalloc(currentAddressAndCodePtr,line);
    strcpy(currentAddressAndCodePtr->binaryCode,tenBits);
    currentAddressAndCodePtr->address=addressCounter+ADD_TO_ADDRESS;
    currentAddressAndCodePtr->type=0;
    addAndCodeList[globalCountersPtr->addressCodeInd]=currentAddressAndCodePtr;
    (globalCountersPtr->addressCodeInd)++;
}

void createNewBinaryIntsLine(char opBits[], AddressAndCode *addAndCodeList[],
                             LabelStruct *labelList[], GlobalCounters *globalCountersPtr,
                             int line, int addressCounter) {
    AddressAndCode *currentAddressAndCodePtr;

    currentAddressAndCodePtr = (AddressAndCode*)malloc(sizeof(AddressAndCode));
    validateAddressAndCodeMalloc(currentAddressAndCodePtr,line);
    currentAddressAndCodePtr->address = addressCounter+ADD_TO_ADDRESS;
    currentAddressAndCodePtr->type = 1;
    strcpy(currentAddressAndCodePtr->binaryCode, opBits);
    addAndCodeList[globalCountersPtr->addressCodeInd]=currentAddressAndCodePtr;
    globalCountersPtr->addressCodeInd++;
}
/* turn content of address&code table to base 32 symbols and print to given ob file
   in the video of explanation about the project, Shadi said to print the instructions part first  */
void printToOb(AddressAndCode *addressCode[], GlobalCounters *globalCountersPtr, FILE *file) {
    int i=0;
    char base32Word1[20]={0};
    char base32Word2[20]={0};
    char newArrIC[BASE_32_WORD]={0};
    char newArrDC[BASE_32_WORD]={0};

    fprintf(file,"   %s  %s\n",decimalTo32(globalCountersPtr->instCounter, newArrIC), decimalTo32(globalCountersPtr->dataCounter, newArrDC));
    /* first printing the instruction part */
    for(i = 0; i<globalCountersPtr->addressCodeSize; i++) {
        if (addressCode[i]->type==1) {
            fprintf(file,"%s\t%s\n",decimalTo32(addressCode[i]->address,base32Word1),  binaryTo32(addressCode[i]->binaryCode,base32Word2));        
            clearString(base32Word1, BASE_32_WORD);
            clearString(base32Word2, BASE_32_WORD); 
        }
    }
    /* now printing the data part */
    for(i = 0; i<globalCountersPtr->addressCodeSize; i++) {
        if (addressCode[i]->type==0) {
            fprintf(file,"%s\t%s\n",decimalTo32(addressCode[i]->address,base32Word1),  binaryTo32(addressCode[i]->binaryCode,base32Word2));        
            clearString(base32Word1, BASE_32_WORD);
            clearString(base32Word2, BASE_32_WORD); 
        }
    }
}

/* print all nodes in address&code table
   I used this function to check table content is correct */
void printBinaryList(AddressAndCode *adCodeTable[], GlobalCounters *globalCountersPtr) {
    int i;

    if (globalCountersPtr->addressCodeSize!=0) {
        for (i=0; i<globalCountersPtr->addressCodeSize; i++) {
        printf("\n address:%d  type:%d  binary:%s\n",adCodeTable[i]->address,adCodeTable[i]->type,adCodeTable[i]->binaryCode);
        }
    }
    else {
        printf("\naddress and code table is empty\n");
    }
}

/* print the given parameters to given .ext file in base 32 symbols
   this function is used during the second pass when extern variables are found */
void printToExt(char *externName, int address, FILE *file) {
    char base32Word[BASE_32_WORD]={0};

    fprintf(file,"%s\t%s\n",externName, decimalTo32(address,base32Word));         
}

void validateAddressAndCodeMalloc(AddressAndCode *ptr, int line) {
    if (ptr==NULL) {
        printError("error while allocating new extern node",line);
        exit(-1);
    }
}

/* search label in label table. if found-return its address+100, else-return -1 */
int checkLabelAddress(LabelStruct *labelList[], GlobalCounters *globalCountersPtr, char *name) {
    int i;
    for (i=0; i<globalCountersPtr->labelListSize; i++) {
        if (strcmp(labelList[i]->labelName,name)==0) {
            return (labelList[i]->address)+ADD_TO_ADDRESS;
        }
    }
    return -1;
}

/* go over entry table, print the label name, turn address to base 32 symbols and print all to given ent file  */
void printToEnt(EntryTable *entryPtrArray[], LabelStruct *labelList[],
                GlobalCounters *globalCountersPtr, FILE *file) {
    int i;
    char base32Word[BASE_32_WORD]={0};

    fillEntryTable(entryPtrArray,labelList,globalCountersPtr);
   
    for(i=0; i<globalCountersPtr->entryTableSize; i++) {
        fprintf(file,"%s\t%s\n",entryPtrArray[i]->content, decimalTo32(entryPtrArray[i]->address ,base32Word));         
        clearString(base32Word,BASE_32_WORD);
    }
}

/* search for lables saved in entry table and fill the address field
   entry lables appear only once in label table (label table can't contain 2 labels with the same name)  */
void fillEntryTable(EntryTable *entryPtrArray[], LabelStruct *labelList[], GlobalCounters *globalCountersPtr) {
    int i,newAddress;

    for (i=0; i<globalCountersPtr->entryTableSize; i++) {
        if ((newAddress=checkLabelAddress(labelList,globalCountersPtr,entryPtrArray[i]->content))!=-1);
            entryPtrArray[i]->address=newAddress;
    }
}