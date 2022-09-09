#ifndef SECOND_PASS_H
#define SECOND_PASS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "numUtils.h"
#include "stringUtils.h"

int secondPass(AddressAndCode *adCodeTable[], LabelStruct *labelList[], EntryTable *entryPtrArray[], ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr, FILE *obFile, FILE *extFile);
int labelTableToBinary(AddressAndCode *adCodeTable[], LabelStruct *labelList[], EntryTable *entryPtrArray[], ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr, FILE *extFile);
int encodeInstruction(char *operator, char *sentence, AddressAndCode *addCodeTable[], LabelStruct *labelList[], EntryTable *entryPtrArray[], ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr, int line, int addressCounter, FILE *extFile);
char *getLabelCodeType(char *name, EntryTable *entryPtrArray[], ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr);
char *getOperandTypeFirstLine(char *operand, LabelStruct *labelList[], EntryTable *entryPtrArray[], ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr);
int getOperandNextLines(char *operand, char tenBits[], AddressAndCode *addCodeTable[], EntryTable *entryPtrArray[], ExternTable *externPtrArray[], LabelStruct *labelList[], GlobalCounters *globalCountersPtr, int operandNum, int addressCounter, FILE *extFile);
void stringToBinaryLines(char *str, AddressAndCode *addAndCodeList[] ,LabelStruct *labelList[], GlobalCounters *globalCountersPtr, int addressCounter, int line);
void numDataToBinaryLines(char *str, AddressAndCode *addAndCodeList[] ,LabelStruct *labelList[], GlobalCounters *globalCountersPtr, int addressCounter, int line);
void structToBinaryLines(char *str, AddressAndCode *addAndCodeList[] ,LabelStruct *labelList[], GlobalCounters *globalCountersPtr, int addressCounter, int line);
void createNewBinaryIntsLine(char opBits[], AddressAndCode *addAndCodeList[] ,LabelStruct *labelList[], GlobalCounters *globalCountersPtr, int line, int addressCounter);
void printToOb(AddressAndCode *addressCode[], GlobalCounters *globalCountersPtr, FILE *file);
void printBinaryList(AddressAndCode *adCodeTable[], GlobalCounters *globalCountersPtr);
void printToExt(char *externName, int address, FILE *file);
void validateAddressAndCodeMalloc(AddressAndCode *ptr, int line);
int checkLabelAddress(LabelStruct *labelList[], GlobalCounters *globalCountersPtr, char *name);
void printToEnt(EntryTable *entryPtrArray[], LabelStruct *labelList[], GlobalCounters *globalCountersPtr, FILE *file);
void fillEntryTable(EntryTable *entryPtrArray[], LabelStruct *labelList[], GlobalCounters *globalCountersPtr);
int encodeFirstLine(char *operand, char *beforeDot, char *afterDot, LabelStruct *labelList[], EntryTable *entryPtrArray[], ExternTable *externPtrArray[], GlobalCounters *globalCountersPtr, char opBits[]);

#endif /* SECOND_PASS_H */