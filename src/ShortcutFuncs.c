#include "../include/ShortcutFuncs.h"


void Print(char* str){
    printf("%s", str);
}

void Println(char* str){
    printf("%s\n", str);
}

int ParseInt(char* str){
    int res;
    int i = 0;

    while (str[i] != '\0')
    {
        if (str[i] > '9' || str[i] < '0'){
            Println("Cannot parse to Integer. Please check the input.");
            return -1;
        }
        res += (str[i] % '9') * pow(10, i);
        i++;
    }
    
    return res;
}

/**
 * A variadic function that takes strCount strings and concatenates them into one string. It allocates memory for the string on the heap.
 */
char * CreateString_alloc(int strCount, ...){

    va_list args; //We define a (char *) list of the arguments which contains a pointer that points to the current argument in the list
    va_start(args, strCount); // We initialize the list starting at the first parameter strCount

    //Let's measure the size of the char * we want to allocate
    char * res;
    int len = 0;
    for (int i = 0; i < strCount; i++)
    {
        len += strlen((char *)va_arg(args, char*)); //va_arg is like next(), it moves the pointer to the next argument and returns the top of the stack
    }
    len++;

    res = (char*)(malloc(sizeof(char) * len));

    va_end(args);
    va_start(args, strCount);

    int k = 0;
    for (int i = 0; i < strCount; i++)
    {
        char * word = (char *)(va_arg(args, char *));

        for (int j = 0; j < strlen(word); j++)
        {
            res[k] = word[j];
            k++;
        }
    }
    res[len - 1] = '\0';

    va_end(args);
    
    return res;
}


/**
 * A variadic function that takes strCount char arrays and concatinates them. It DOES NOT return a string with a null terminating char, only a char array.
 */
char * ConcatCharArrays_alloc(int strCount, ...){

    va_list args; //We define a (char *) list of the arguments which contains a pointer that points to the current argument in the list
    va_start(args, strCount); // We initialize the list starting at the first parameter strCount

    //Let's measure the size of the char * we want to allocate
    char * res;
    int len = 0;
    for (int i = 0; i < strCount; i++)
    {
        len += strlen((char *)va_arg(args, char*)); //va_arg is like next(), it moves the pointer to the next argument and returns the top of the stack
    }
    //len++;

    res = (char*)(malloc(sizeof(char) * len));

    va_end(args);
    va_start(args, strCount);

    int k = 0;
    for (int i = 0; i < strCount; i++)
    {
        char * word = (char *)(va_arg(args, char *));

        for (int j = 0; j < strlen(word); j++)
        {
            res[k] = word[j];
            k++;
        }
    }

    va_end(args);
    
    return res;
}