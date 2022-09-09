#ifndef FIRST_PASS_H
#define FIRST_PASS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "numUtils.h"
#include "utils.h"
#include "stringUtils.h"

int firstPass(FILE *amFile, LabelStruct *labelPtrArray[], EntryTable *entryPtrArray[], ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr);
int handleStringOperand(char *word);
int isInstructionValid(char *operator, char *sentence, int line);
void validateEntryMalloc(EntryTable *ptr, int line);
void printLabelList(LabelStruct *labelPtrArray[], int listSize);
int validateOperandType(char *operation, char *operands, LabelStruct *labelList[], EntryTable *entryPtrArray[], ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr);
void validateLabelMalloc(LabelStruct *ptr, int line);
int isLabelValid(char *label);
int getOperandAddressType(char *operand, LabelStruct *labelList[], EntryTable *entryPtrArray[], ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr);


#endif /* FIRST_PASS_H */