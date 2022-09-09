#include "preAssembler.h"
#include "firstPass.h"
#include "secondPass.h"
#include "globals.h"
#define FILE_ERROR -1

/* get file names from user, add an extension and open .as file
   pre assembler: copy the .as file content to .am file, find macros and copy the macro content to .am file
   first pass: build the label table and check the content of .am file
   second pass: building address and binary code table using the label table and then write to .ob file with base 32 symbols
   write to .ext\.ent file if necessary
   if error is found in one of the passes, stop reading current file and move to next-if exists */
int main(int argc, char* argv[]) {
    char inputFileName[FILENAME_MAX];
    char amFileName[FILENAME_MAX];
    char obFileName[FILENAME_MAX];
    char entFileName[FILENAME_MAX];
    char extFileName[FILENAME_MAX];
    int fileNum=1;
    int preAssemblerErrors, firstPassErrors, secondPassErrors;
    FILE *asFile, *amFile, *obFile,  *entFile, *extFile;
    LabelStruct *labelPtrArray[MAX_LABEL_PTR];
    EntryTable *entryPtrArray[MAX_LABEL_PTR];
    ExternTable *externPtrArray[MAX_LABEL_PTR];
    AddressAndCode *addressAndCodeTable[MAX_LABEL_PTR];
    GlobalCounters *globalCountersPtr;

    if (argc==1) {
        printf("ERROR: no file name\nPlease enter a file name in command line\n");
    }
    
    /* reading all files we get from user */
    while (fileNum<argc) {

        /* create global counters struct and initialize all counters */
        globalCountersPtr=(GlobalCounters*)malloc(sizeof(GlobalCounters));
        validateGlobalsMalloc(globalCountersPtr);
        globalCountersPtr->labelListSize=0;
        globalCountersPtr->instCounter=0;
        globalCountersPtr->dataCounter=0;
        globalCountersPtr->entryTableSize=0;
        globalCountersPtr->externTableSize=0;
        globalCountersPtr->addressCodeSize=0;
        globalCountersPtr->addressCodeInd=0;
        
        printf("Opening and reading .as file number %d\n",fileNum);
        createFileName(argv[fileNum], ".as", inputFileName);
        asFile=fopen(inputFileName,"r"); 
        if (!asFile) {
            printf("ERROR: .as file does not exist\nPlease enter a file name in command line\n");
            exit(FILE_ERROR);
        }

        createFileName(argv[fileNum], ".am", amFileName);
        printf("Opening .am file for writing\n");
        amFile=fopen(amFileName,"w");
        if (!amFile) { 
            printf("ERROR: error occured while creating .am file\n");
            exit(FILE_ERROR);
        }

        printf("Starting pre assembler for file #%d %s\n", fileNum, inputFileName);
        preAssemblerErrors=preAssembler(asFile,amFile);
        if (preAssemblerErrors!=0) {
            printf(ANSI_COLOR_RED "Pre assembler errors - cannot continue to first pass\n\n" ANSI_COLOR_RESET);
            fclose(amFile);
            fclose(asFile);
            printf(ANSI_COLOR_MAGENTA "*******************************************************\n" ANSI_COLOR_RESET);
            fileNum++;
            continue;
        }
        else {
            printf(ANSI_COLOR_CYAN "Pre assembler passed successfully!\n\n" ANSI_COLOR_RESET);
            fclose(amFile);
            fclose(asFile);
        }

        printf("Opening .am file for reading\n");
        amFile=fopen(amFileName,"r");
        if (!amFile) { 
            printf("ERROR: error occured while opening .am file\n");
            exit(FILE_ERROR);
        }

        printf("Starting first pass\n");
        firstPassErrors=firstPass(amFile,labelPtrArray,entryPtrArray,externPtrArray,globalCountersPtr);
        if (firstPassErrors!=0) {
            printf(ANSI_COLOR_RED "First pass errors - cannot continue to second pass\n" ANSI_COLOR_RESET);
            fclose(amFile);
            if (globalCountersPtr->labelListSize!=0) {
                freeLabelList(labelPtrArray,globalCountersPtr);
            }
            free(globalCountersPtr);
            printf(ANSI_COLOR_MAGENTA "*******************************************************\n" ANSI_COLOR_RESET);
            fileNum++;
            continue;
        }
        else {
            printf(ANSI_COLOR_CYAN "First pass tests passed successfully!\n\n" ANSI_COLOR_RESET);
            fclose(amFile);
        }

        printf("preparing for second pass\n");
        printf("Opening .ob file for writing:\n");
        
        createFileName(argv[fileNum], ".ob", obFileName);
        obFile=fopen(obFileName,"w"); 
        if (!obFile) { 
            printf("ERROR: error occured while creating .ob file\n");
            exit(FILE_ERROR);
        }

        /* if we find extern labels we write them directely into .ext file during second pass */
        createFileName(argv[fileNum], ".ext", extFileName);
        extFile=fopen(extFileName,"w");
        if (!extFile) {
            printf("ERROR: error occured while creating .ext file\n");
            exit(FILE_ERROR);
        }

        secondPassErrors=secondPass(addressAndCodeTable,labelPtrArray,entryPtrArray,externPtrArray,globalCountersPtr,obFile,extFile);
        if (secondPassErrors!=0) {
            printf(ANSI_COLOR_RED "Second pass errors - cannot continue\n" ANSI_COLOR_RESET);
            if (globalCountersPtr->labelListSize!=0) {
                freeLabelList(labelPtrArray,globalCountersPtr);
            }
            if (globalCountersPtr->addressCodeSize!=0) {
                freeBinaryList(addressAndCodeTable,globalCountersPtr);
            }
            free(globalCountersPtr);
            remove(extFileName);
            remove(obFileName);
            printf(ANSI_COLOR_MAGENTA "*******************************************************\n" ANSI_COLOR_RESET);
            fileNum++;
            continue;
        }
        else {
            printf(ANSI_COLOR_CYAN "Second pass tests passed successfully!\n\n" ANSI_COLOR_RESET);
        }

        printf("Checking for entry variables\n");
        if (globalCountersPtr->entryTableSize!=0) {
            createFileName(argv[fileNum], ".ent", entFileName);
            entFile=fopen(entFileName,"w");
            if (!entFile) {
            printf("ERROR: error occured while creating .ent file\n");
            exit(FILE_ERROR);
        }
            printToEnt(entryPtrArray,labelPtrArray,globalCountersPtr,entFile);
            printf(ANSI_COLOR_CYAN ".ent file created successfully!\n\n" ANSI_COLOR_RESET);
            fclose(entFile);
        }
        else {
            printf(".ent file is not needed\n\n");
        }

        printf("Checking for extern variables\n");
        if (globalCountersPtr->externTableSize!=0) {
            printf(ANSI_COLOR_CYAN ".ext file created successfully!\n\n" ANSI_COLOR_RESET);
            fclose(extFile);
        }
        else {
            remove(extFileName);
            printf(".ext file is not needed\n\n");
        }
        
        /* free all lists before moving to next file */
        if (globalCountersPtr->labelListSize!=0) {
            freeLabelList(labelPtrArray,globalCountersPtr);
        }
        if (globalCountersPtr->addressCodeSize!=0) {
            freeBinaryList(addressAndCodeTable,globalCountersPtr);
        }
        if (globalCountersPtr->externTableSize!=0) {
            freeExternList(externPtrArray,globalCountersPtr);
        }
        if (globalCountersPtr->entryTableSize!=0) {
            freeEntryList(entryPtrArray,globalCountersPtr);
        }
        free(globalCountersPtr);

        printf(ANSI_COLOR_BLUE "All tests passed successfully in file #%d %s!\n" ANSI_COLOR_RESET, fileNum, inputFileName);
        fileNum++; /* move to reading next file */
        
        printf(ANSI_COLOR_MAGENTA "*******************************************************\n" ANSI_COLOR_RESET);
    }
return 0;
}