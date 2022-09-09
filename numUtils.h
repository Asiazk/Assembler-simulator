#ifndef NUMUTILS_H
#define NUMUTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* operations on bits to print only 10\8 LSB of integer (both positive&negative) */
#define MASKOF10 ((1 << 10) - 1)
#define MASKOF8 ((1 << 8) - 1)
#define SIZE_OF_BYTE 11

int isNumValid(char *num);
char *decimalTo10LSB(int num, char newArr[]);
char *decimalTo8LSB(int num, char newArr[]);
int binaryToDecimal(char *binary);
char *decimalTo32(int num, char newArr[]);
char *binaryTo32(char *binaryStr, char base32[]);

#endif /* NUMUTILS_H */