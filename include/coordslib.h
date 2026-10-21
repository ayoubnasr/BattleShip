#ifndef COORDSLIB

#include "defs.h"
#include "ShortcutFuncs.h"

#define COORDSLIB

int CoordToIndex(char coords[], int start, int end, char startingChar, char endingChar, int coordShift);

int *alloc_ArrayCoordsFromUserCoords(char coords[], char ** outputMsg);

int *alloc_GridAreaFromInput (char startingCoords[], char orientation[], int width, int height, char ** outputMsg);

int IsCoordValid(char coords[], int start, int endEXC, char startingCoord);

int IndexWithinRange(int index);


extern const char startingCoordinate_1;
extern const char endingCoordinate_1;
extern const char startingCoordinate_2;
extern const char endingCoordinate_2;
extern const int coord_1_shift;
extern const int coord_2_shift;

#endif