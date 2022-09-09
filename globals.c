#include "globals.h"

const char *RESERVED_NAMES[] = {
            "r0","r1","r2","r3","r4","r5","r6","r7",
            "mov","cmp","add","sub","not","clr","lea","inc","dec","jmp","bne","get","prn",
            "jsr","rts","hlt",".data","data",".struct","struct",".string","string",".extern",
            "extern",".entry","entry","macro","endmacro"};

const char *COMMANDS[] = {
            "mov","cmp","add","sub","not","clr","lea","inc","dec","jmp",
            "bne","get","prn","jsr","rts","hlt"};

const char *BINARY_COMMANDS[] = {
            "0000","0001","0010","0011","0100","0101","0110","0111","1000",
            "1001","1010","1011","1100","1101","1110","1111"};

const char *REGISTERS[] = {
            "r0","r1","r2","r3","r4","r5","r6","r7"};

const char *BINARY_REGISTERS[] = {
            "0000","0001","0010","0011","0100","0101","0110","0111"};

const char *BINARY_ADDRESS_TYPE[] = {
            "00","01","10","11"};