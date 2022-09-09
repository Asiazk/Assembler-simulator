CC = gcc # GCC Compiler
CFLAGS = -ansi -Wall -pedantic # Flags
OBJ = main.o globals.o utils.o preAssembler.o firstPass.o secondPass.o numUtils.o stringUtils.o # Object files

assembler: clean $(OBJ)
	$(CC) -g $(CFLAGS) -o assembler $(OBJ) -lm

main.o: main.c
	$(CC) -c main.c $(CFLAGS) -o main.o

preAssembler.o: preAssembler.c preAssembler.h
	$(CC) -c preAssembler.c $(CFLAGS) -o preAssembler.o

firstPass.o: firstPass.c firstPass.h
	$(CC) -c firstPass.c $(CFLAGS) -o firstPass.o

secondPass.o: secondPass.c secondPass.h
	$(CC) -c secondPass.c $(CFLAGS) -o secondPass.o

utils.o: utils.c utils.h
	$(CC) -c utils.c $(CFLAGS) -o utils.o

numUtils.o: numUtils.c numUtils.h globals.o
	$(CC) -c numUtils.c $(CFLAGS) -o numUtils.o

globals.o: globals.c globals.h
	$(CC) -c globals.c $(CFLAGS) -o globals.o

stringUtils.o: stringUtils.c stringUtils.h
	$(CC) -c stringUtils.c $(CFLAGS) -o stringUtils.o

clean:
	rm -rf *.o