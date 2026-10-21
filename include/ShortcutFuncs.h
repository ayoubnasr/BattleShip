#ifndef SHORTCUTS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define SHORTCUTS

//strcmpi() (case-insensitive compare) only exists on Windows, other platforms call it strcasecmp():
#ifndef _WIN32
    #include <strings.h>
    #define strcmpi strcasecmp
#endif

void Print(char* str);

void Println(char* str);

int ParseInt(char* str);

char * CreateString_alloc(int strCount, ...);
char * ConcatCharArrays_alloc(int strCount, ...);

#endif
