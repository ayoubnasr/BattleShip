#include "../include/coordslib.h"
#include <stdbool.h>
#include <stdlib.h>
#include "../include/defs.h"
#include "../include/Player.h"
#include "../include/ShortcutFuncs.h"


int Fire(char *inputC, char ** grid, int difficulty, char ** outputMsg)
{
    int *coords = alloc_ArrayCoordsFromUserCoords(inputC, outputMsg);//Convert from user-input coordinates to array coords

    if (coords == NULL){
        return -1;
    }

    char * c = &grid[coords[0]][coords[1]];

    if (*c == HIT){
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "This coordinate has already been hit."); //Storing the output message to be printed in the driver.
        return -1;
    }
    if (IsShip(*c))
    {

        *c = HIT;

        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Hit!");//Storing the output message to be printed in the driver.
    }
    else
    {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Miss");//Storing the output message to be printed in the driver.
        if (difficulty == 0)
        {
            *c = MISS;
        }
    }
    free(coords);

    return 1;
}

int performRadarSweep(char *inputC, Player* player, char ** outputMsg)
{

    int *coords = alloc_ArrayCoordsFromUserCoords(inputC, outputMsg);

    if (coords == NULL) {
        //if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Invalid coordinates");
        return -1;
    }

    if (player->sweepsLeft == 0) {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "No more radar sweeps allowed! You lose your turn.");
        free(coords);
        return 1;
    }

    (player->sweepsLeft)--;
    int foundShip = -1;

    for (int i = coords[0]; i < coords[0] + 2 && i < GRIDSIZE; i++) {
        for (int j = coords[1]; j < coords[1] + 2 && j < GRIDSIZE; j++) {
            if ((player->smokeGrid)[i][j]) {
                continue;
            }
            if ((player->grid)[i][j] != WATER_C && (player->grid)[i][j] != MISS) {
                foundShip = 1;
            }
        }
    }

    if (outputMsg != NULL) *outputMsg = (foundShip>0) ? CreateString_alloc(1, "Enemy ships found") : CreateString_alloc(1, "No enemy ships found");
    free(coords);

    return 1;
}


int applySmokeScreen(char *inputC, Player* player, Player * opponent, char ** outputMsg)
{
    int *coords = alloc_ArrayCoordsFromUserCoords(inputC, outputMsg);


    if (coords == NULL){
        return -1;
    }

    //printf("\nGayy");

    if (player->usedsmokes >= countSunkShips(opponent))
    {
        //printf("sunkships: %d\n", opponent->currSunkShips);
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "You cannot use more smoke screens than the ships you've sunk!");
        return -1;
    }

    

    for (int i = coords[0]; i < coords[0] + 2 && i < GRIDSIZE; i++)
    {
        for (int j = coords[1]; j < coords[1] + 2 && j < GRIDSIZE; j++)
        {
            (player->smokeGrid)[i][j] = true;
        }
    }

    player->usedsmokes++;
    if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Smoke screen applied.");

    //printf("Smoke screen applied at %c%d!\n", 'A' + coords[0], coords[1] + 1); //???
    free(coords);

    return 1;
}

int Artillery(char *inputC, Player* player, Player* opp, int difficulty, char ** outputMsg)
{
    int *coords = alloc_ArrayCoordsFromUserCoords(inputC, outputMsg);

    if (player->prevSunk==0)
    {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Artillery can only be used in the round right after sinking an opponent's ship!");
        return -1;
    }

    if (coords == NULL)
    {
        return -1;
    }
    int hitcount =0;
    for (int i = coords[0]; i < coords[0] + 2; i++)
    {
        for (int j = coords[1]; j < coords[1] + 2; j++)
        {
            if ((opp->grid)[i][j] == CARRIER_C ||
                (opp->grid)[i][j] == BATTLESHIP_C ||
                (opp->grid)[i][j] == DESTROYER_C ||
                (opp->grid)[i][j] == SUBMARINE_C )
            {
                (opp->grid)[i][j] = HIT;
                hitcount++;
            }
            else if ((opp->grid)[i][j]==HIT)continue;
            else {
                    if (difficulty==0){  
                        (opp->grid)[i][j] = MISS;
                    }
                }
        }
    }
    if (hitcount>0)
    {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Hit!");
    }
    else
    {
        
        
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Miss!");
        
        
    }
    
    free(coords);

    return 1;
}

//Note: For a larger number of players, we need to save the number of ships every player sunk for each opponent
int Torpedo(char *inputC,Player* player,Player* opp, int difficulty, char ** outputMsg)
{

    if (countSunkShips(opp) < 3 || !(player->prevSunk==1)) 
    {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Torpedo can only be used after sinking the opponent's third ship!");
        return -1;
    }

    

    int coord_1 = CoordToIndex(inputC, 0, strlen(inputC), startingCoordinate_1, endingCoordinate_1, coord_1_shift);

    int coord_2 = CoordToIndex(inputC, 0, strlen(inputC), startingCoordinate_2, endingCoordinate_2, coord_2_shift);

    printf("inp: %s\n", inputC);
    printf("COORD1: %d\nCOORD2: %d\n", coord_1, coord_2);

    if (!IndexWithinRange(coord_1) && !IndexWithinRange(coord_2)){

        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Invalid coordinate! Please pick a coordinate within range.");
        return -1;

    }


    int hitCount = 0;

    for (int i = 0; i < GRIDSIZE; i++)
    {
        char * c;
        c = (coord_1 >= 0)? (&(opp->grid)[i][coord_1]) : (&(opp->grid)[coord_2][i]);

        if (IsShip(*c))
        {
            *c = HIT;
            hitCount++;
        }
        else {
            if (difficulty==0)
            {  
                *c = MISS;
            }
        }

    }
    
    //printf("\ndsfadsfadsf");

    if (hitCount == 0)
    {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Miss.");
    }
    else
    {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, "Hit!");
    }

    return 1;
}
