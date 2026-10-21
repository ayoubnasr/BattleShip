#ifndef PLAYER
#define PLAYER

#include <stdbool.h>
#include "defs.h"
#include "coordslib.h"
#include "InputLib.h"
#include "ShortcutFuncs.h"
#include "UITools.h"
#include "BinomialHeap.h"
#include "D_LinkedList.h"
#include "Bot.h"

#define playerColorCount 5 

extern char * PlayerColors[playerColorCount];

typedef struct ShipBounds {
    int startRow, startCol, endRow, endCol;
    bool IsSunk;
    //bool isSunk;
} ShipBounds;




/**
 * This is the number of distinct probability types regions could fall into.
 */
#define PROB_CATEGORYCOUNT 3

//THE HIGHEST A PROBABILITY CAN BE IS 2(5 + 4 + 3 + 2) = 28
#define NULLCHANCEPROB 0
#define LOWPROB_BASE 1
#define AVGPROB_BASE 10
#define HIGHPROB_BASE 18

#define PROB_REGION_WIDTH 2
#define PROB_REGION_HEIGHT 2
#define PROB_REGION_SIZE {PROB_REGION_WIDTH, PROB_REGION_HEIGHT}

#define PROB_REGION_COUNT ceil(GRIDSIZE / PROB_REGION_WIDTH) * ceil(GRIDSIZE / PROB_REGION_HEIGHT)

#define EXTREMELYHIGH_RISK 3
#define HIGH_RISK 2
#define NORMAL_RISK 1
#define LOW_RISK 0


typedef struct Player{

    char *name;
    char** grid;
    bool** smokeGrid;
    int usedsmokes;
    ShipBounds carrierBounds;
    ShipBounds battleshipBounds;
    ShipBounds destroyerBounds;
    ShipBounds submarineBounds;
    
    int prevSunk;
    int currSunkShips; //This stores the current number of sunk ships (the number of sunk ships at the end of the previous turn)
    int sweepsLeft;  // Variable for tracking the remaining sweeps
    //Note: Grid size is not set here. Player grids are allocated dynamically when the game starts.

    char * UIColor;

    bool isBot;
    BotIQ botIQ;
    int ** probabilityGrid;

    /**
     * This is an array of probability heaps. Each heap stores the coordinates of a region in the grid ordering them by probability. This way
     * we maintain that the time complexity of finding the highest probability coordinate in a k by k region is O(1).
     * 
     * The elements stored in the heaps will be key-value pairs:
     *      - key: probability
     *      - value: grid coordinate (i,j)
     * 
     * 
     * The array will be initialized with three slots:
     *      - index 0: null chance probability regions
     *      - index 1: low probability regions
     *      - index 2: avg probability regions
     *      - index 3: high probability regions
     * 
     * Throughout the game certain regions will lose or gain probability. When that happens, they will be moved between the three arrays and placed
     * in the corresponding one.
     * 
     * How we determine in which array a certain priority queue is placed:
     *      When the probabilities of some region are updated, the entire heap is reassembeled which takes O(k) where k is the size of the largest
     *      ship so O(1). According to the highest probability in the heap we can know whether the region falls in low, average, or high probability range.
     *      According to the range, we decide where to move it.
     * 
     * This method solves two main problems:
     *      - Bot predictability
     *      - Accessing the highest probability in O(1)
     */
    BinomialHeap ** probabilityHeapSet;
    //BinomialHeap *** probabilityHeapArray;
    D_LinkedList * probabilityHeapCategoryLists; //This pointer represents an array of linked lists.

    int currTargetCategory; //This is used to allow the bot to follow a pattern picking once from every category every time.

    //Stack Memory:
    D_LinkedList * stackMemory;

    //Risk Variables:
    int riskFactor;

}Player;


Player ** alloc_InitializePlayerArray(int playerCount, Player *** playersArray);
Player* alloc_InitializePlayer(Player** output, char* playerName, int isBot, BotIQ botIQ);

int InitializeProbabilities(Player * player);

int InitializeProbabilityHeaps(Player * player);

int HashRegion(int i, int j);


int InitializeBotStackMemory(Player * bot);

int* BinHeap_FindHighestProbabilityCell(BinomialHeap * binHeap);

void InOrderTraversalTree_ProbNode(Node * root);
void InOrderTraversal_ProbNode_Print(BinomialHeap * heap);

void DisplayGrid(char ** grid, int gridSize);
void DisplayIntGrid(int ** grid, int gridSize);

void DisplayOpponentGrid(char ** grid, int gridSize, int showMiss);



int countSunkShips(Player* player);

int checkIfSunk(Player *player, ShipBounds* shipBounds);

void ShowPlayerStats(Player * player);

void Show_2_PlayerStats(Player * player, Player * opponent);
#endif