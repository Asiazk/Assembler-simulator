#ifndef GLOBALS_H
#define GLOBALS_H

#define NUM_OF_ADDRESS_BITS 9
#define MAX_OPERAND_LENGTH 32
#define DIGIT_MAX 5
#define MAX_LINE_SIZE 81
#define NUM_OF_RESERVED_NAMES 36
#define CHARS_INSIDE_MACRO 400 /* Yakir wrote in forum  we can limit the size of macro content */
#define MAX_LABEL_NAME 32
#define SIZE_OF_BYTE 11
#define NUM_OF_COMMANDS 16
#define OPERAND_SIZE 10
#define NUM_OF_REGISTERS 8
#define NAME_SIZE 10
#define BASE_32_WORD 3
#define MAX_OPERATION_NAME 8
#define ADD_TO_ADDRESS 100 /* adding 100 to address field of label to simulate addreesses of imaginary computer's RAM */
#define MAX_LABEL_PTR 156 /* Yakir wrote in forum we can limit the number of labels. each label in label struct represents a line with an individual address so max number of lines is 256-100=156 */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_BLUE    "\x1b[34m"

extern const char *RESERVED_NAMES[];
extern const char *COMMANDS[];
extern const char *BINARY_COMMANDS[];
extern const char *REGISTERS[];
extern const char *BINARY_REGISTERS[];
extern const char *BINARY_ADDRESS_TYPE[];

/* struct for saving and updating counters
   most functions from different passes must have access to multiple counters
   all counters change dinamically and must be accurate and updated for correct output */
typedef struct GlobalCounters
{
    unsigned int labelListSize;
    unsigned int instCounter;
    unsigned int dataCounter;
    unsigned int entryTableSize;
    unsigned int externTableSize;
    unsigned int addressCodeSize;
    unsigned int addressCodeInd;

} GlobalCounters;

/* struct for label table */
typedef struct LabelStruct
{
    unsigned int line;
    unsigned int address;
    char labelName[MAX_LABEL_NAME];
    char operation[MAX_OPERATION_NAME]; /* is it one of 16 commands or data part:.data\.string\.struct */
    char operands[MAX_LINE_SIZE];

} LabelStruct;

/* all following nodes will be saved in separate pointer array */
typedef struct EntryTable
{
    char content[MAX_LABEL_NAME];
    unsigned int address;

} EntryTable;

typedef struct ExternTable
{
    char content[MAX_LABEL_NAME];
    unsigned int address;

} ExternTable;

/* struct for address and binary code */
typedef struct AddressAndCode
{
    unsigned int address;
    char binaryCode[SIZE_OF_BYTE];
    unsigned int type; /* if type defined as 1-current node is instruction, 0-is data */
} AddressAndCode;

#endif /* GLOBALS_H */