

#ifndef INPUT

#include <stdio.h>
#include "defs.h"

#define MAXINPUTLENGTH 25
//void AnalyzeInput();

#define TOTALINSTRUCTIONCOUNT 8

typedef enum InputOps{INVALIDOP = -1, START, QUIT, NEXTURN, FIRE, RADAR, SMOKE, ARTILLERY, TORPEDO} InputOps;

extern char** InstructionSet;

extern char* PRESTARTINSTRUC[TOTALINSTRUCTIONCOUNT];
extern char* PREGAMEINSTRUC[TOTALINSTRUCTIONCOUNT];
extern char* INGAMEINSTRUC[TOTALINSTRUCTIONCOUNT];


int StringToEnumIndex(char* str, char* arr[], int arrLen);

char * alloc_IntegerToNumeral(int i, int maxInt, char startingChar, char endingChar);

char * alloc_GetCoordsFromIndices(int row, int col, int MaxInt, char startingCoordinate_1, char startingCoordinate_2, char endingCoordinate_2, char endingCoordinate_1, char coord1_shift, char coord2_shift);

int ManageOperation(InputOps operation, char * rawInput);

char* next(char ** input);

int nextInt(char ** input);

char* alloc_Input(char* msg, char ** output);

#define INPUT

#endif