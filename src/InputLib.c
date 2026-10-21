#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/InputLib.h"
#include "../include/Statements.h"
#include "../include/ShortcutFuncs.h"
#include "../include/UITools.h"

char** InstructionSet;

char* PRESTARTINSTRUC[TOTALINSTRUCTIONCOUNT] = {"start", "quit", "", "", "", "", "", ""};
char* PREGAMEINSTRUC[TOTALINSTRUCTIONCOUNT] = {"", "quit", "", "", "", "", "", ""};
char* INGAMEINSTRUC[TOTALINSTRUCTIONCOUNT] = {"", "quit", "next", "fire", "radar", "smoke", "artillery", "torpedo"};


/**
 * Takes a string and array of strings and returns the index where this string lies in the array if it does, and -1 otherwise.
 */
int StringToEnumIndex(char* str, char* arr[], int arrLen){

    for (int i = 0; i < arrLen; i++)
    {
        if (strcmpi(arr[i], str) == 0){
            return i;
        }
    }
    
    return -1;

}

/**
 * This function is used for converting from the decimal numeral system to a custom one according to the given char range.
 * Using simple math, it % the number over and over, finds the right char for each remainder and concats them into a string.
 * 
 * Example: (note: A = 0, B = 1, C = 2, ..., Z = 25)
 * 
 * BAC = 1 * 26^(2) + 0 * 26 + 2 = 678
 * 
 * Converting back:
 * 
 * 678 % 26 = 2 ----> C
 * 678 / 26 = 26
 * 26 % 26 = 0 -----> A
 * 26 / 26 = 1
 * 1 % 26 = 1 ------> B
 * 
 * Input:
 *      - int i: the number you want to transform
 *      - int maxInt: the largest integer you might want your numeral system to represent (used to know how much padding for the string) 
 * 
 */
char * alloc_IntegerToNumeral(int i, int maxInt, char startingChar, char endingChar){

    int base = endingChar - startingChar + 1;

    int len = 0;
    int k = maxInt;

    //We set the length of the string to maximum possible length based on what the largest integer possible is.
    do
    {
        len++;
        k/=base;
    } while (k > 0);
    len++;

    char * result = (char*)(malloc(sizeof(char) * len));

    result[len - 1] = '\0';

    for (int j = 0; j < len - 1; j++)
    {
        result[j] = startingChar;
    }
    

    int j = len - 2;
    k = i;
    do
    {
        char r = k%base + startingChar;
        result[j] = r;
        j--;
        k/=base;
    } while (k > 0);
    
    return result;
    
}

/**
 * 
 * 
 * Adds coordinate shifting to the numeral systems for grid purposes.
 * 
 */
char * alloc_GetCoordsFromIndices(int row, int col, int MaxInt, char startingCoordinate_1, char startingCoordinate_2, char endingCoordinate_1, char endingCoordinate_2, char coord1_shift, char coord2_shift){

    char * coords1 = alloc_IntegerToNumeral(col + coord1_shift, GRIDSIZE, startingCoordinate_1, endingCoordinate_1);
    char * coords2 = alloc_IntegerToNumeral(row + coord2_shift, GRIDSIZE, startingCoordinate_2, endingCoordinate_2);
    
    char * coords = CreateString_alloc(2, coords1, coords2);

    free (coords1);
    free(coords2);

    return coords;

}


/**
 * This returns the next word in a string. It allocates memory for a new char pointer. And it modifies the original char pointer to right after
 * the last word.
 * 
 * And make sure to use a copy of the input pointer to avoid losing the start of the string cuz freeing memory then would lead to problems.
 */
char* next(char ** input){

    char *word;

    if ((*input)[0] == '\0'){
        word = (char*)(malloc(sizeof(char)));
        word[0] = '\0';
        return word;
    }


    int i = 0;

    while ((*input)[i] != ' ' && (*input)[i] != '\0' && (*input)[i] != '\n')
    {
        i++;
    }
    i--; //This is to return back from the ' ' or '\0' or '\n'

    int k = i + 1;
    while ((*input)[k] == ' '){
        k++;
    }
    
    int wordLen = i + 2; //Extra spot for the null terminating character
    word = (char*)(malloc(sizeof(char) * wordLen));

    for (int j = 0; j <= i; j++)
    {
        word[j] = (*input)[j];
    }
    word[wordLen - 1] = '\0';

    *input = (*input + k);
    

    return word;

}

int nextInt(char ** input){
    char* nextStr = next(input);
    
    return ParseInt(nextStr);

}

/**
 * 
 * note: always returns '\0' if no word, never null
 */
char * alloc_Input(char * showMsg, char ** output){

    Println_Centered(showMsg, strlen(showMsg), WHITE);
    Print_Centered("> ", strlen(showMsg), WHITE);
    //printf("%s", showMsg);

    char * inputRes = (char*)(malloc(sizeof(char) * MAXINPUTLENGTH));

    

    char c = ' ';
    int i = 0;
    //int maxLen = MAXINPUTLENGTH;

    c = getchar();

    while (c != '\n' && i < MAXINPUTLENGTH - 1)
    {
        inputRes[i] = c;
        c = getchar();
        i++;
    }

    inputRes[i] = '\0';

    *output = inputRes;

    return inputRes;

}