#ifndef STRING_UTILS_H
#define STRING_UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int containsMoreChars(char *str);
void removeSpaces(char *word);
void toLowerString(char *word);
void removeLastChar(char *labelName);
void clearString(char *str, int max);
int checkWhiteSpaces(char *word);
int findChar(char *str, char c);
int findLastQuotes(char *str);
int countChar(char *word, char c);
void removeFirstChar(char *withChar, char noChar[]);

#endif