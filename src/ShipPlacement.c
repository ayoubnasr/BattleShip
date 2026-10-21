#include <stdio.h>
#include <stdlib.h>
#include "../include/ShipPlacement.h"


/**
 * This function is used to setup the bounds of the passed in Shipbound struct. Shipbound structs are stored in the Player struct
 * and used to store ship area location.
 */
void setShipBounds(int startRow, int endRow, int startCol, int endCol, ShipBounds *shipBounds) {
    shipBounds->startRow = startRow;
    shipBounds->startCol = startCol;
    shipBounds->endRow = endRow;
    shipBounds->endCol = endCol;
    shipBounds->IsSunk = false;
}

/**
 * Places the player's ship on the grid by filling the necessary elements in the grid 2D array. This function places ships with a default horizontal orientation.
 * It calls the PlaceShipOnGridHelper function. For more details check the documentation of the last function.
 */
int PlaceShipOnGridHorizontal(Player *player,char shipChar, char **grid, char coords[], int ship_size[2], char ** outputMsg){
    char orientation[] = "horizontal";
    return PlaceShipOnGridHelper(player,shipChar, grid, coords, orientation, ship_size, outputMsg);
}

/**
 * This function places the player's ships on the grid. It takes in user-input coordinates, orientation and ship size
 * and calculates the correct array indices in which the ship information will be stored.
 * It calculates the i and j array bounds within which the elements of the 2D array must be modified to store the ship character. Then it
 * calls the ModifyGridArea function which modifies those elements.
 * 
 * It also initializes the stored Shipbounds of each ship inside the player struct.
 */
int PlaceShipOnGridHelper(Player *player, char shipChar, char **grid, char coords[], char orientation[], int ship_size[2] , char ** outputMsg) {

    int *arrayCoords = alloc_ArrayCoordsFromUserCoords(coords, outputMsg);
    if (arrayCoords == NULL) {
        return -1;
    }

    int *shipbounds = alloc_GridAreaFromInput(coords, orientation, ship_size[0] - 1, ship_size[1], outputMsg);
    if (shipbounds == NULL) {
        free(arrayCoords);
        return -1;
    }

    if (!IndexWithinRange(shipbounds[0]) || !IndexWithinRange(shipbounds[1])
     || !IndexWithinRange(shipbounds[2]) || !IndexWithinRange(shipbounds[3])) {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Ship bounds out of range. Please pick an area inside the grid.");
        free(arrayCoords);
        free(shipbounds);
        return -1;
    }

    if (CheckForOverlap(grid, shipbounds) == true) {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Cannot place ship! A ship already exists in the designated area.");
        free(arrayCoords);
        free(shipbounds);
        return -1;
    }

    // Place ship on the grid
    ModifyGridArea(grid, GRIDSIZE, shipbounds, shipChar);

    // Set bounds for the specific ship in the player structure
    switch (shipChar) {
        case CARRIER_C: // Carrier
            setShipBounds(shipbounds[0], shipbounds[1], shipbounds[2], shipbounds[3], &(player->carrierBounds));
            break;
        case BATTLESHIP_C: // Battleship
            setShipBounds(shipbounds[0], shipbounds[1], shipbounds[2], shipbounds[3], &(player->battleshipBounds));
            break;
        case DESTROYER_C: // Destroyer
            setShipBounds(shipbounds[0], shipbounds[1], shipbounds[2], shipbounds[3], &(player->destroyerBounds));
            break;
        case SUBMARINE_C: // Submarine
            setShipBounds(shipbounds[0], shipbounds[1], shipbounds[2], shipbounds[3], &(player->submarineBounds));
            break;
        default:
            if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Unknown ship type.");
            free(arrayCoords);
            free(shipbounds);
            return -1;
    }

    free(arrayCoords);
    free(shipbounds);

    return 1;
}


/**
 * Updates the elements of the grid 2D array at the i and j indices within the specified bounds to the specified char c.
 */
void ModifyGridArea(char **grid, int rowSize, int * bounds, char c){

    for (int i = bounds[0]; i <= bounds[1]; i++)
    {
        for (int j = bounds[2]; j <= bounds[3]; j++)
        {
            grid[i][j] = c;
        }
    }

    return;
}

/**
 * Returns 1 if a ship exists within the inputed bounds on the grid and -1 otherwise.
 * The function expects an array of 4 integers [row0, row1, col0, col1].
 * The bounds are included in the check, so for a length of 1 you input x0 = 1 and x1 = 1.
 * 
 * Returns 1 if there is an overlap and 0 if no overlap.
 */
int CheckForOverlap(char **grid, int bounds[]) {
    int row0= bounds[0], row1=bounds[1], col0=bounds[2] , col1= bounds[3];
    for (int i = row0; i <= row1; i++) {
        for (int j = col0; j <= col1; j++) {

            if (IsShip(grid[i][j])) {
              return true; 
            }
        }
    }
    return false;  
}

