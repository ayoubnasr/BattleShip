#ifndef CALCPROBS_H
#define CALCPROBS_H

#include "defs.h"

typedef struct Player Player;

/**
 * Calculates the number of possible placements for a ship in a specific square
 * without exceeding the boundaries and updates the probability grid.
 * 
 * Excludes sunk ships from calculations.
 * 
 * @param player Pointer to the Player structure containing the probability grid and ship bounds.
 * @param target A 2-element array representing the target square coordinates.
 * @return 1 if the function worked properly, 0 otherwise.
 */
int CalcCutoffProb(Player *player, int target[2]);

int InitalizeCutOffProb(Player *player, int target[2]);

/**
 * Adjusts the probability grid based on whether the target square is a hit or miss.
 * 
 * Excludes sunk ships from calculations.
 * 
 * @param player Pointer to the Player structure containing the probability grid and ship bounds.
 * @param target A 2-element array representing the target square coordinates.
 * @return 1 if the function worked properly, 0 otherwise.
 */
int CalcOverlapProb(Player *player, int target[2]);

/**
 * Updates the surrounding probabilities of a target square after an attack.
 * 
 * Recalculates probabilities for all surrounding cells based on the state of the grid
 * and excludes sunk ships from calculations.
 * 
 * @param player Pointer to the Player structure containing the grid and probability grid.
 * @param target A 2-element array representing the target square coordinates.
 * @return 1 if the function worked properly, 0 otherwise.
 */
int UpdateSurroundingProbabilities(Player *player, int target[2]);

/**
 * Checks if the target square is valid for further attacks.
 * 
 * A square is invalid if it has already been marked as a hit or miss.
 * 
 * @param grid The grid of the player's opponent's ships.
 * @param target A 2-element array representing the target square coordinates.
 * @return 1 if the square is valid for attack, 0 otherwise.
 */
int CheckHitOrMiss(char** grid, int target[2]);

#endif // CALCPROBS_H
