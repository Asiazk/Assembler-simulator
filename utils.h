#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "numUtils.h"
#include "globals.h"
#include "stringUtils.h"

void printError(char *errorComment, int line);
char *createFileName(char *input, char *extension, char *output);
int isNameValid(char *str);
int findCommandIndex(char word[]);
int divideInstruction(char *instruction, char first[], char second[]);
int checkLineLength(int counter, int maxSize, int line);
void removeSpacesBeforeChar(char *word, char noSpaces[], char c);
int splitByDotOperands(char *sentence, char first[], char second[]);
int isRegister(char *reg);
int countOperands(char *sentence);
void validateExternMalloc(ExternTable *ptr, int line);
void freeLabelList(LabelStruct *labelList[], GlobalCounters *globalCountersPtr);
void freeBinaryList(AddressAndCode *list[], GlobalCounters *globalCountersPtr);
void freeExternList(ExternTable *list[], GlobalCounters *globalCountersPtr);
void freeEntryList(EntryTable *list[], GlobalCounters *globalCountersPtr);
int checkLabelName(LabelStruct *labelList[], int size ,char *name);
int isDataPart(char *word);
int checkStruct(LabelStruct *labelList[], char *name, GlobalCounters *globalCountersPtr);
int checkLabelARE(char *name, EntryTable *entryPtrArray[], ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr);
int checkNumOperand(char *num);
void validateGlobalsMalloc(GlobalCounters *ptr);

#endif /* UTILS_H */