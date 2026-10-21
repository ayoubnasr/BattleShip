
#include "Player.h"
#include "coordslib.h"
#include "defs.h"

#ifndef SHIPPLACEMENT

#define SHIPPLACEMENT

void ModifyGridArea(char **grid, int rowSize, int * bounds, char c);
void setShipBounds(int startRow, int startCol, int endRow, int endCol, ShipBounds* shipBounds);
int PlaceShipOnGridHelper(Player *player, char shipChar, char **grid, char coords[], char orientation[], int ship_size[2], char ** outputMsg);
int PlaceShipOnGridHorizontal(Player *player,char shipChar, char **grid, char coords[], int ship_size[2], char ** outputMsg);
int CheckForOverlap(char **grid, int bounds[]);
#endif