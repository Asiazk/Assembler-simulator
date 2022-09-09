#include "numUtils.h"
#include <math.h>

const char BASE_32_SYMBOLS[] = {
        '!', '@', '#', '$', '%', '^', '&', '*', '<', '>', 'a', 'b', 'c', 'd', 'e',
        'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v'};

/* verify num string starts only with the following characters: +\-\digit, and doesn't contain a decimal point
   if valid-return 1, else-return -1 */
int isNumValid(char *num) { 
    int i;
    if (strlen(num)>=1) {
        if(!isdigit(num[0]) && num[0]!='+' && num[0]!='-') {
        return -1;
        }
    }

    if (strlen(num)>1) {
        if ((num[0]=='+' || num[0]=='-') && num[1]=='0') {
            return -1;
        }        
        for(i=1;i<strlen(num);i++) {
            if(!isdigit(num[i])) {
                return -1;
            }
        }
    }
    return 1;
}

/* turn integer to required binary string
   note in this project only masks of 8 and 10 were used */
char *decimalToBinary(int numWithMask, char newArr[]) {
    int i=0, j=0, low=0;
    int high, arrSize, temp1, n;
    int arr[SIZE_OF_BYTE];

    while(numWithMask!=0) {
        n = numWithMask%2;
        arr[i]=n;
        i++;
        numWithMask/=2;
    }
    arrSize=i;

    /* reverse the array to get the accurate number in 2 base*/
    high=arrSize-1;
    while (low<=high) {
        temp1=arr[low];
        arr[low]=arr[high];
        arr[high]=temp1;
        low++;
        high--;
    }

    while(j<arrSize) {
        newArr[j]=arr[j]+'0'; /* casting int to char */
        j++;
    }
    newArr[j]='\0';
    return newArr;
}

/* turn integer to 10 LSB binary string
   if num's binary presentation is larger than 10 bits, only 10 LSB will be returned */
char *decimalTo10LSB(int num, char newArr[]) {
    return decimalToBinary(num&MASKOF10, newArr);
}

/* turn integer to 8 LSB binary string
   if num's binary presentation is larger than 8 bits, only 8 LSB will be returned */
char *decimalTo8LSB(int num, char newArr[]) {
    return decimalToBinary(num&MASKOF8, newArr);
}

/* turn binary string to decimal number */
int binaryToDecimal(char *binary) {
    int decimal=0;
    int i,p;
    for (p=0,i=strlen(binary)-1; i>=0; i--,p++) {
        if (binary[i]!='0' && binary[i]!='1') {
            printf("non-binary number as input");
        }
        decimal+=(binary[i]-'0')*(pow(2,p));
    }
    return decimal;
}

/* this function gets the number we would like to convert, int array and a char array.
   then converts the given number to 32 base (in symbols)  */
char *decimalTo32(int num, char newArr[]) {
    /* we need to get the array of integers in parameter
       the size should be 27 because max int size is 2^32 and the base is 2^5
       so max number of digits in 32 base is 32-5=27*/
    int newNum=num;
    int i=0, j=0, low=0;
    int high, arrSize, temp1, temp2, n;
    int arr[SIZE_OF_BYTE]={0}; /* biggest possible number with 10 bits is 1023 */

    if (num==0) {
        newArr[0]=BASE_32_SYMBOLS[num];
        return newArr;
    }
    /* calculate the given number to 32 base and put results in int array */
    while(newNum!=0) {
        n = newNum%32;
        arr[i]=n;
        i++;
        newNum/=32;
    }
    arrSize=i;

    /* reverse the array to get the accurate number in 32 base*/
    high=arrSize-1;
    while (low<=high) {
        temp1=arr[low];
        arr[low]=arr[high];
        arr[high]=temp1;
        low++;
        high--;
    }

    /* turn each number to matching symbol */ 
    while(j<arrSize) {
        temp2=arr[j]; /* turning each int to corresponding symbol as each index of symbols array is the matching symbol to index number */
        newArr[j]=BASE_32_SYMBOLS[temp2];
        j++;
    }
    newArr[j]='\0';
    return newArr;
}

/* turn binary number into base 32 symbols */
char *binaryTo32(char *binaryStr, char base32[]) {
    int num=binaryToDecimal(binaryStr);
    decimalTo32(num,base32);
    return base32;
}
