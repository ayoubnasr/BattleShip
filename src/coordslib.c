
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include "../include/coordslib.h"

const char startingCoordinate_1 = 'A';
const char endingCoordinate_1 = 'Z';
const char startingCoordinate_2 = '0';
const char endingCoordinate_2 = '9';
const int coord_1_shift = 0;
const int coord_2_shift = 1;

#pragma region [Coordinates Conversion Scripts]

/**
 * The CoordsToIndex function takes in coordinates in their raw representation depending on the numeral system. Then they are converted
 * into i and j coordinates that work for arrays.
 * 
 * Details:
 * Since the game uses a square grid, it requires two numeral systems for separately numbering the rows and the columns.
    To make the input system scalable and compatable with larger grids, this function technically splits the coordinates input
    into two numbers of separate numeral systems (eg. 'AAA001' -> 'AAA' and '001'), which could also be equivalent systems.
    Then it retrieves an index for each number ('AAA' = 000 and '001' = 001).
    The developer could define any numeral system using any ASCII symbols on the condition that those symbols are in a sequence (eg. 'A' = 65, 'B' = 66...) because the conversion exploits the ordering of the ASCII numbers.
 */
int CoordToIndex(char coords[], int start, int endEXC, char startingChar, char endingChar, int coordShift){

    if (endEXC - start <= 0){
        return -1;
    }

    int val = 0;
    int base = endingChar - startingChar + 1;

    for (int i = 0; i < endEXC - start; i++){
        char currChar = coords[start + i];
        if (currChar < startingChar || currChar > endingChar){
            return -1;
        }

        val += (currChar - startingChar) * pow(base, (endEXC - start) - i - 1);

    }


    return val - coordShift; //We subtract by how much the numeral system was shifted for graph display. For example, if
    //we want the graph to show column numbers from 1 to 10, we can't just throw out the zero from the numeral system cause that
    //would change the entire number scale. So we just apply a shift and remove it in calculation.
}

/**Input: User-inputted coordinate string
 * Output: Returns the index in the char array that separates the two coordinate components (eg. for AB24 it returns 1 at 'B')
 * 
 * Assuming that the inputted coordinates are valid, this function splits the row numeral from the column numeral.
 * The function splits when it reaches a char that is not in the char range of the row numeral system.
 */
/*
*/
static int GetCoordSplitIndex(char coords[]){

    int split = 0;
    char currCoord = startingCoordinate_1;

    for (split = 0; split < strlen(coords); split++){
        char currChar = coords[split];
        if (currChar < currCoord || currChar > currCoord + GRIDSIZE){
            return split - 1;
        }
    }

    return split - 1;
}


/**Input: User-inputted coordinate string
 * Output: Returns the index in the char array that separates the two coordinate components (eg. for AB24 it returns 1 at 'B') 

static int GetCoordSplitIndex(char coords[]){

    int end = strlen(coords) - 1;

    return end / 2;

}
*/

/**Input: 
 *      - coords: string of user inputed coordinates
 *      - start: starting char
 *      - end: ending char
 *      - startingCoord: the starting coordinate of the number's numeral system
 * Output:
 *      - True: if the inputted coordinate component is valid (all characters in bound)
 *      - False: otherwise
*/
int IsCoordValid(char coords[], int start, int end, char startingCoord){
    
    for (int i = start; i < end; i++){
        char currChar = coords[i];
        if (currChar < startingCoord || currChar > startingCoord + GRIDSIZE){
            return -1;
        }
    }
    
    return 1;
}

/**
 * Input: index value
 * 
 * Output: Returns 1 if index is within range and 0 otherwise.
 */
int IndexWithinRange(int index){
    return index <= GRIDSIZE - 1 && index >= 0;
}

/*
- Input: Takes in a string (char array) of the user inputted coordinates.
- Output: If input is valid, it allocates memory for and returns an integer array of size 2 with the i and j coordinates {i, j}.
- Details:
    Since the game uses a square grid, it requires two numeral systems for separately numbering the rows and the columns.
    To make the input system scalable and compatable with larger grids, this function technically splits the coordinates input
    into two numbers of separate numeral systems (eg. 'AAA001' -> 'AAA' and '001'), which could also be equivalent systems.
    Then it retrieves an index for each number ('AAA' = 000 and '001' = 001).
    The developer could define any numeral system using any ASCII symbols on the condition that those symbols are in a sequence (eg. 'A' = 65, 'B' = 66...) because the functions this function is dependant on exploit that ordering of the ASCII numbers.
*/
int * alloc_ArrayCoordsFromUserCoords(char coords[], char ** outputMsg){
    
    int coordsLen = strlen(coords);

    //First: Split the coordinate input:
    int split = GetCoordSplitIndex(coords);

    if (split < 0 || !IsCoordValid(coords, split + 1, coordsLen, startingCoordinate_2)) {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Warning in Coordinates-to-Array conversion! Invalid coordinates entered.");
        return NULL;
    }

    int j = CoordToIndex(coords, 0, split + 1, startingCoordinate_1, endingCoordinate_1, coord_1_shift);
    int i = CoordToIndex(coords, split + 1, coordsLen, startingCoordinate_2, endingCoordinate_2, coord_2_shift);

    //if (*outputMsg != NULL) free(*outputMsg); //In case the CoordToIndex returns an error message.

    if (!IndexWithinRange(i) || !IndexWithinRange(j)){
        
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Warning in Coordinates-to-Array conversion! Coordinates out of bounds.");
        return NULL;
    }

    

    int *result = (int *)(malloc(sizeof(int) * 2));
    result[0] = i;
    result[1] = j;

    return result;

}

/**
 *Input: 
 *      - startingCoords: a string of the starting point on the game grid
 *      - orientation: a string representing the orientation of the area
 *      - width: the specified width of the area
 *      - height: the specified height of the area
 * 
 * Output: 
 * alloc_GridAreaFromInput will allocate memory for a 4-element integer array which stores the boundaries of the
 * area in array coordinates (eg. [row0, row1, col0, col1]).
 * 
 * Details: 
 * (i0, j0) is the starting coordinate. (i1, j1) is the ending coordinate. To get i1 and j1, the width or the height is added to
 * i0 and j0, depending on the orientation. Flipping the orientation flips the axis of the width and that of the height.
 */
int *alloc_GridAreaFromInput (char startingCoords[], char orientation[], int width, int height, char ** outputMsg){
    int *startArrayCoords = alloc_ArrayCoordsFromUserCoords(startingCoords, outputMsg);

    if (startArrayCoords == NULL){
        return NULL;
    }

    if (strcmpi(orientation, "v") == 0){
        int temp = width;
        width = height;
        height = temp;
    }
    int * bounds = (int *)(malloc(sizeof(int) * 4));


    bounds[0] = startArrayCoords[0];
    bounds[1] = startArrayCoords[0] + height;
    bounds[2] = startArrayCoords[1];
    bounds[3] = startArrayCoords[1] + width;

    free(startArrayCoords);

    return bounds;

}


#pragma endregion


///Must remember to free memory allocated for coords array.