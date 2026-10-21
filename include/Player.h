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

//Index of each category in the probabilityHeapCategoryLists array:
#define LOWPROB_CATEGORY 0
#define AVGPROB_CATEGORY 1
#define HIGHPROB_CATEGORY 2
#define NO_CATEGORY -1 //Regions with no cell left to target (all shot, or zero probability) are not in any list

//THE HIGHEST A PROBABILITY CAN BE IS 2(5 + 4 + 3 + 2) = 28
#define NULLCHANCEPROB 0
#define LOWPROB_BASE 1
#define AVGPROB_BASE 10
#define HIGHPROB_BASE 18

#define PROB_REGION_WIDTH 2
#define PROB_REGION_HEIGHT 2
#define PROB_REGION_SIZE {PROB_REGION_WIDTH, PROB_REGION_HEIGHT}

//Number of regions on one row of the grid, and in total. Rounded up: when the grid size is not a multiple of the region size, the last
//regions of a row/column are smaller.
#define PROB_REGIONS_PER_ROW ((GRIDSIZE + PROB_REGION_WIDTH - 1) / PROB_REGION_WIDTH)
#define PROB_REGIONS_PER_COL ((GRIDSIZE + PROB_REGION_HEIGHT - 1) / PROB_REGION_HEIGHT)
#define PROB_REGION_COUNT (PROB_REGIONS_PER_ROW * PROB_REGIONS_PER_COL)

#define EXTREMELYHIGH_RISK 3
#define HIGH_RISK 2
#define NORMAL_RISK 1
#define LOW_RISK 0

/**
 * Risk control (presentation, 2.3). The bot measures how dangerous its opponent is with three numbers:
 *      - Accuracy     = opponent's hits / opponent's total shots
 *      - Success rate = opponent's hits in the last RISK_RECENT_TURNS turns / RISK_RECENT_TURNS
 *      - Sunk ratio   = opponent's sunk ships (the bot's ships it sank) / bot's sunk ships (the opponent's ships the bot sank)
 * Each number past its threshold below counts as one sign of danger: 0 signs is LOW_RISK, 1 is NORMAL_RISK, 2 is HIGH_RISK and 3 is
 * EXTREMELYHIGH_RISK. Changing these values changes how strongly the bot reacts to the opponent's success.
 */
#define RISK_RECENT_TURNS 5
#define RISK_MIN_SHOTS 3                //The bot stays at NORMAL_RISK until the opponent has attacked this many times
#define RISK_ACCURACY_THRESHOLD 0.30
#define RISK_SUCCESS_RATE_THRESHOLD 0.40
#define RISK_SUNK_RATIO_THRESHOLD 1.0

//At LOW_RISK the bot takes more risks: this is the chance (in %) that it attacks somewhere else while it is sinking a ship
#define LOWRISK_EXPLORE_CHANCE 25


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
     * The heaps are categorized in three linked lists (probabilityHeapCategoryLists):
     *      - index 0 (LOWPROB_CATEGORY): low probability regions
     *      - index 1 (AVGPROB_CATEGORY): avg probability regions
     *      - index 2 (HIGHPROB_CATEGORY): high probability regions
     *      Null chance regions (no cell left that could hold a ship) are kept out of the lists.
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

    /**
     * For every region (same index as probabilityHeapSet), the node of the category list that holds the region's heap, and the index of
     * that category list (NO_CATEGORY and NULL when the region is in no list).
     *
     * This is what makes moving a region to another category O(1): the heap's list node is found directly from the hash of the region,
     * then unlinked and appended to the new category without searching through the lists. (presentation, 2.2.5: final region system)
     */
    D_ListNode ** probabilityHeapNodes;
    int * probabilityHeapCategory;

    int currTargetCategory; //This is used to allow the bot to follow a pattern picking once from every category every time.

    //Stack Memory:
    D_LinkedList * stackMemory;

    //Risk Variables:
    int riskFactor;

    int opponentShots;  //Number of attacks the opponent made against this player
    int opponentHits;   //How many of those attacks hit
    int recentOpponentHits[RISK_RECENT_TURNS]; //Whether each of the opponent's last attacks hit (1) or not (0), oldest overwritten first

    //What the bot decided on its last turn and why (shown in the bot's view):
    BotDecision lastDecision;

}Player;


Player ** alloc_InitializePlayerArray(int playerCount, Player *** playersArray);
Player* alloc_InitializePlayer(Player** output, char* playerName, int isBot, BotIQ botIQ);

int InitializeProbabilities(Player * player);

int InitializeProbabilityHeaps(Player * player);

int HashRegion(int i, int j);

int GetProbabilityCategory(int probability);

int PlaceHeapInCategory(Player * player, int heapIndex);

int BinHeap_Insert_ProbElement(BinomialHeap * binHeap, int Prob, int row, int col);

int IsCellOfSunkShip(Player * player, int row, int col);


int InitializeBotStackMemory(Player * bot);

int* BinHeap_FindHighestProbabilityCell(BinomialHeap * binHeap);

void InOrderTraversalTree_ProbNode(Node * root);
void InOrderTraversal_ProbNode_Print(BinomialHeap * heap);

void DisplayGrid(char ** grid, int gridSize);
void DisplayIntGrid(int ** grid, int gridSize);

void DisplayOpponentGrid(char ** grid, int gridSize, int showMiss);



int countSunkShips(Player* player);

int countHitCells(Player* player);

int checkIfSunk(Player *player, ShipBounds* shipBounds);

void ShowPlayerStats(Player * player);

void Show_2_PlayerStats(Player * player, Player * opponent);
#endif
