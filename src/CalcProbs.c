#include <stdio.h>
#include "../include/defs.h"
#include "../include/Player.h"
#include "../include/CalcProbs.h"


int CheckHitOrMiss(char** grid, int target[2]) // checks if the cell is a hit or a miss
{
    int rowc = target[0];
    int colc = target[1];
    if (grid[rowc][colc] == HIT || grid[rowc][colc] == MISS)
        return 1;
    return 0;
}


int CalcCutoffProb(Player *player, int target[2]) // player here is the opponent
{
    int rowc = target[0];
    int colc = target[1];

    //printf("%d\n", rowc);

    if (rowc < 0 || rowc >= GRIDSIZE || colc < 0 || colc >= GRIDSIZE)
    {
        printf("Invalid target coordinates.\n");
        return 0;
    }

    player->probabilityGrid[rowc][colc] = 0; // initializing the certain cell to zero

    ShipBounds ships[] = {
        player->carrierBounds,
        player->battleshipBounds,
        player->destroyerBounds,
        player->submarineBounds}; // add ships to an array to use their size and to know if one of them is sunked

    for (int s = 0; s < SHIPCOUNT; s++)
    {
        if (ships[s].IsSunk)
            continue; // skipping the ship if its sunk

        int shipLength = ships[s].endRow - ships[s].startRow + 1; // Use ship bounds to determine length
        if (shipLength == 1)
            shipLength = ships[s].endCol - ships[s].startCol + 1;

        int Horiprob = (colc + 1 < GRIDSIZE - colc) ? colc + 1 : GRIDSIZE - colc;
        if (Horiprob > shipLength)
            Horiprob = shipLength;
        /*the formula is the probability of the ship to pass here horizontally is
        the minimum between nb of cells left to the cell, right to the cell and the shiplength
        */

        int Vertiprob = (rowc + 1 < GRIDSIZE - rowc) ? rowc + 1 : GRIDSIZE - rowc;
        if (Vertiprob > shipLength)
            Vertiprob = shipLength;
        // same formula but vertical

        player->probabilityGrid[rowc][colc] += Horiprob + Vertiprob; // adding both probabilities to the cell in the probgrid

        //printf("%d\n", Horiprob);
    }
    return 1;
}

int InitalizeCutOffProb(Player *player, int target[2]) // player here is the opponent
{
    int rowc = target[0];
    int colc = target[1];

    //printf("%d\n", rowc);

    if (rowc < 0 || rowc >= GRIDSIZE || colc < 0 || colc >= GRIDSIZE)
    {
        printf("Invalid target coordinates.\n");
        return 0;
    }

    player->probabilityGrid[rowc][colc] = 0; // initializing the certain cell to zero

    int ShipSizes[] = SHIPSIZES;

    for (int s = 0; s < SHIPCOUNT; s++)
    {

        int shipLength = ShipSizes[s];

        int Horiprob = (colc + 1 < GRIDSIZE - colc) ? colc + 1 : GRIDSIZE - colc;
        if (Horiprob > shipLength)
            Horiprob = shipLength;
        /*the formula is the probability of the ship to pass here horizontally is
        the minimum between nb of cells left to the cell, right to the cell and the shiplength
        */

        int Vertiprob = (rowc + 1 < GRIDSIZE - rowc) ? rowc + 1 : GRIDSIZE - rowc;
        if (Vertiprob > shipLength)
            Vertiprob = shipLength;
        // same formula but vertical

        player->probabilityGrid[rowc][colc] += Horiprob + Vertiprob; // adding both probabilities to the cell in the probgrid

        //printf("%d\n", Horiprob);
    }
    return 1;
}

int CalcOverlapProb(Player *player, int target[2])
{
    int rowc = target[0];
    int colc = target[1];

    if (rowc < 0 || colc < 0 || rowc >= GRIDSIZE || colc >= GRIDSIZE)
    {
        printf("Invalid target coordinates.\n");
        return 0;
    }

    int adjustment = 0;

    ShipBounds ships[] = {
        player->carrierBounds,
        player->battleshipBounds,
        player->destroyerBounds,
        player->submarineBounds};

    for (int s = 0; s < SHIPCOUNT; s++)
    {
        if (ships[s].IsSunk)
            continue; // skip the ship if its sunk

        int shipLength = ships[s].endRow - ships[s].startRow + 1;
        if (shipLength == 1)
            shipLength = ships[s].endCol - ships[s].startCol + 1;

        for (int i = 0; i < shipLength; i++)
        {
            if (rowc - i >= 0 && player->grid[rowc - i][colc] == HIT)
                adjustment++;
            else if (rowc - i >= 0 && player->grid[rowc - i][colc] == MISS)
                adjustment--;

            if (colc - i >= 0 && player->grid[rowc][colc - i] == HIT)
                adjustment++;
            else if (colc - i >= 0 && player->grid[rowc][colc - i] == MISS)
                adjustment--;

            if (colc + i < GRIDSIZE && player->grid[rowc][colc + i] == HIT)
                adjustment++;
            else if (colc + i < GRIDSIZE && player->grid[rowc][colc + i] == MISS)
                adjustment--;

            if (rowc + i < GRIDSIZE && player->grid[rowc + i][colc] == HIT)
                adjustment++;
            else if (rowc + i < GRIDSIZE && player->grid[rowc + i][colc] == MISS)
                adjustment--;
            // for all above, below, left or right cells to a certain cell by the size of the ship, we check
            // if these are hits misses or none and update the variable adjustments based on that
        }
    }

    player->probabilityGrid[rowc][colc] += adjustment;// adding adjustment to the original probability
    //printf("adjustment = %d\n", adjustment);
    return 1;
}

int UpdateSurroundingProbabilities(Player *player, int target[2])
{
    int rowc = target[0];
    int colc = target[1];

    if (rowc < 0 || colc < 0 || rowc >= GRIDSIZE || colc >= GRIDSIZE)
    {
        printf("Invalid target coordinates.\n");
        return 0;
    }

    ShipBounds ships[] = {
        player->carrierBounds,
        player->battleshipBounds,
        player->destroyerBounds,
        player->submarineBounds};

    int maxSize = 0;
    for (int s = 0; s < SHIPCOUNT; s++)
    {
        if (!ships[s].IsSunk)
        {
            int size = ships[s].endRow - ships[s].startRow + 1;
            if (size == 1)
                size = ships[s].endCol - ships[s].startCol + 1;
            if (size > maxSize)
                maxSize = size;
            // calculating the max size of ships that are not sunk to know where to update the probabilities
        }
    }

    for (int i = rowc - maxSize; i <= rowc + maxSize; i++)
    {
        int VertiSurs[2] = {i, colc};
        if (i >= 0 && i < GRIDSIZE && !CheckHitOrMiss(player->grid, VertiSurs))
        {
            CalcCutoffProb(player, VertiSurs);
            CalcOverlapProb(player, VertiSurs);
        }
    }


    for (int i = colc - maxSize; i <= colc + maxSize; i++)
    {
        int HortiSurs[2] = {rowc, i};
        if (i >= 0 && i < GRIDSIZE && !CheckHitOrMiss(player->grid, HortiSurs))
        {
            CalcCutoffProb(player, HortiSurs);
            CalcOverlapProb(player, HortiSurs);
        }
        // we calculate the probability on the cell-maxsize to cell +maxsize vertically and horizontally
    }

    return 1;
}

int UpdateRegionProbabilities(Player *player, int target[4])
{
    int Hstart = target[0], Hend = target[1], Vstart = target[2], Vend = target[3];

    if (Hstart < 0 || Hend < 0 || Vstart < 0 || Vend < 0 ||
        Hstart >= GRIDSIZE || Hend >= GRIDSIZE || Vstart >= GRIDSIZE || Vend >= GRIDSIZE)
    {
        printf("Invalid target coordinates.\n");
        return 0;
    }

    ShipBounds ships[] = {
        player->carrierBounds,
        player->battleshipBounds,
        player->destroyerBounds,
        player->submarineBounds};

    for (int i = Hstart; i <= Hend; i++)
    {
        for (int j = Vstart; j <= Vend; j++)
        {
            int cell[2] = {i, j};
            bool skipCell = false;

            // Check if the cell is part of a sunk ship or is a hit or a miss
            for (int s = 0; s < SHIPCOUNT; s++)
            {
                if (ships[s].IsSunk &&
                        i >= ships[s].startRow && i <= ships[s].endRow &&
                        j >= ships[s].startCol && j <= ships[s].endCol ||
                    CheckHitOrMiss(player->grid, cell))
                {
                    skipCell = true;
                    break;
                }
            }

            if (!skipCell)
            {
                CalcCutoffProb(player, cell);
                CalcOverlapProb(player, cell);
            }
        }
    }

    return 1;
}


