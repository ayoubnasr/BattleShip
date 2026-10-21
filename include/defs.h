#ifndef DEFS
#define DEFS

#define GRIDSIZE 200

#define WATER_C '~'

#define SHIPCOUNT 4
#define SHIPSIZES {2,3,4,5}

#define SUBMARINE_C 's'
#define DESTROYER_C 'd'
#define BATTLESHIP_C 'b'
#define CARRIER_C 'c'
#define HIT 'x'
#define MISS 'O'

#define IsShip(c) (c == CARRIER_C || c == BATTLESHIP_C || c == DESTROYER_C || c == SUBMARINE_C)

#define SUBMARINE_LENGTH 2
#define DESTROYER_LENGTH 3
#define BATTLESHIP_LENGTH 4
#define CARRIER_LENGTH 5

#define LONGEST_SHIPLENGTH CARRIER_LENGTH

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define GAMEMODECOUNT 2

typedef enum GameMode{ INVALIDMODE = -1, PVP, PVE }GameMode;

extern char* GameModeStrings[2];

#define DIFFICULTYCOUNT 2
extern char* DifficultyStrings[2];

/**
 * Defines the difficulty of the game. Zero for easy mode and 1 for hard mode.
 */
extern int DifficultyValue;

#endif