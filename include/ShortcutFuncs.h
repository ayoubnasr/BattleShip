#ifndef SHORTCUTS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define SHORTCUTS

void Print(char* str);

void Println(char* str);

int ParseInt(char* str);

char * CreateString_alloc(int strCount, ...);
char * ConcatCharArrays_alloc(int strCount, ...);

#endif