#include "stringUtils.h"

/* check if given word contains any printable characters. if so-return 1,else-return 0  */
int containsMoreChars(char *str) {
    int i;
    for (i=0; i<strlen(str); i++) {
        if (isprint(str[i])) {
            return 1;
        }
    }
    return 0;
}

/* removes all spaces from given word */
void removeSpaces(char *word) {
    int i, j;
    for (i = 0, j = 0; i < strlen(word); i++) {
	    word[i - j] = word[i];        
	    if (word[i] == ' ' || word[i] == '\t')
		    j++;
    }

    word[i - j] = '\0';
}

/* lower case all letters in given word */
void toLowerString(char *word) {
    int i=0;
    while (word[i]!='\0') {
        word[i]=tolower(word[i]);
        i++;
    }
}

/* remove last character in given string
   before using this function, make sure it's a character you want to remove */
void removeLastChar(char *labelName) {
    labelName[strlen(labelName)-1] = '\0';
}

/* initialize a given word with length of max */
void clearString(char *str, int max) {
    int i=0;
    while (i<max) {
        str[i] = 0;
        i++;
    }
}

/* check if the given string contains white spaces only. if so-return 1, else-return 0 */
int checkWhiteSpaces(char *word) {
    int i;

    for (i=0; i<strlen(word); i++) {
        if (word[i]!=' ' && word[i]!='\t' && word[i]!='\n') {
            return 0;
        }
    }
    return 1;
}

void removeSpacesBeforeChar(char *word, char noSpaces[], char c) {
    int i,j;
    int foundChar=-1;
    for (i=0,j=0; i<strlen(word); i++) {
        if (word[i]==c) {
            if (foundChar==-1) {
                foundChar=1;
            }
        }
        if (word[i]==' ' || word[i]=='\t') {
            if (foundChar==-1) {
                continue;
            }
        }
        noSpaces[j]=word[i];
        j++;
    }
}

/* return first occurance of given character in given word */
int findChar(char *str, char c) {
    int i;
    for (i=0; i<strlen(str); i++) {
        if (str[i]==c) {
            return 1;
        }
    }
    return -1;
}

/* check the index of quotes of a given string.
   if it's the last character-return 1. else-return -1 */
int findLastQuotes(char *str) {
    int i;
    int quoteIndex=-1;
    for (i=0; i<strlen(str); i++) {
        if (str[i]=='"') {
            quoteIndex=i;
        }
    }
    if (quoteIndex==strlen(str)-1) {
        return 1;
    }
    return -1;
}

/* count the occurance of a given character in given word */
int countChar(char *word, char c) {
    int result=0;
    int i;
    for (i=0; i<strlen(word); i++) {
        if (word[i]==c) {
            result++;
        }
    }
    return result;
}

/* remove the first character from a given string */
void removeFirstChar(char *withChar, char noChar[]) {
    int i;
    for (i=0;i<strlen(withChar); i++) {
        noChar[i]=withChar[i+1];
    }
}