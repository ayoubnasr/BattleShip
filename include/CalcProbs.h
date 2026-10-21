#ifndef CALCPROBS_H
#define CALCPROBS_H

#include "defs.h"

typedef struct Player Player;

/**
 * Counts the placements of a ship of the given length that cover the cell in one orientation (0 horizontal, 1 vertical) and fit:
 * inside the grid, with no miss and no sunk ship in the way.
 */
int CountFittingPlacements(Player *player, int rowc, int colc, int shipLength, int vertical);

/**
 * Calculates the number of possible placements for a ship in a specific square
 * that fit in the empty space around it (inside the grid, no miss and no sunk ship in the way) and updates the probability grid.
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
 * Raises the probability of the target square for every hit (of a ship still afloat) within reach of the remaining ships along its
 * row and column. Misses are already accounted for by CalcCutoffProb().
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
 * Recalculates the probabilities of every cell in a rectangular area. Cells that were shot (hit or miss) are skipped.
 * Used on the whole grid when a ship sinks.
 *
 * @param player Pointer to the Player structure containing the grid and probability grid.
 * @param target A 4-element array {row0, row1, col0, col1} of the area's bounds (inclusive).
 * @return 1 if the function worked properly, 0 otherwise.
 */
int UpdateRegionProbabilities(Player *player, int target[4]);

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
