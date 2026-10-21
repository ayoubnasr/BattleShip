#ifndef BOT
#define BOT

#include "BinomialHeap.h"
#include "D_LinkedList.h"

typedef struct Player Player;

typedef enum BotIQ{
    DUMB, AVG, SMART
} BotIQ;

#define TASKFLAG_LOWPRIORITY 0
#define TASKFLAG_HIGHPRIORITY 1

/**
 * The kinds of decisions the bot can make on its turn, one per branch of its attack functions.
 */
typedef enum BotDecisionType{
    DECISION_NONE,              //The bot hasn't played yet
    DECISION_TORPEDO,           //Torpedo on the row/column with the highest probability sum
    DECISION_ARTILLERY,         //Artillery on the 2x2 area with the highest probability sum
    DECISION_TASK,              //Followed a task from its stack memory (a cell next to a hit)
    DECISION_CATEGORY,          //Took its turn in the category rotation: random region from that category list, then the region's best cell
    DECISION_HIGHEST_CATEGORY,  //High risk: random region from the highest non-empty category, then its best cell
    DECISION_HOTTEST_CELL,      //Extremely high risk: the single highest probability cell of the grid
    DECISION_FALLBACK,          //No cell had any probability left: a random cell that was not shot yet
    DECISION_RANDOM             //Easy bot: a random cell that could still hold a ship
} BotDecisionType;

/**
 * What the bot decided on its last turn, and why. The attack functions fill it in as they decide, so that the bot's view (BotView.c) can
 * show it. Recording a decision never changes it.
 */
typedef struct BotDecision{

    BotDecisionType type;

    int row, col;       //Targeted cell. Artillery: the top-left cell of the 2x2 area. Torpedo: the row or the column, the other one is -1.
    int probability;    //Probability of the cell when it was chosen (artillery/torpedo: the probability sum of the area)
    int category;       //Category list of the targeted cell's region when it was chosen (-1 when it was in none)
    int region;         //Region index of the targeted cell (-1 for artillery and torpedo)
    int explored;       //1 if the bot left the plans on its stack to explore elsewhere (low risk)
    int riskFactor;     //The bot's risk level when it decided

} BotDecision;

typedef struct{

    /**
     * Function pointer to a task function. A valid task function should return an integer whether it successfully executed or not. The arguments
     * could be decoded from the void*.
     * This approach is easier to use than a variadic function for our specific purposes.
     */
    int (*function)(void**);

    /**
     * This void pointer could represent an array of arguments. These arguments could be passed into a function that knows how to read them correctly.
     */
    void ** arguments;
    int argumentCount;

    void ** flags;
    int flagCount;

} BotTask;

void PlaceBotShips(Player * bot);

void BotSmartAttack(Player * bot, Player * opponent);

//Average intelligence (checker pattern + hunting queue, see docs/bot-design.docx) is designed but not implemented yet.
void BotAverageAttack(Player * bot, Player * opponent);

void BotDumbAttack(Player * bot, Player * opponent);

int BotUseSpecialWeapon(Player * bot, Player * opponent);

void RecordOpponentAttack(Player * bot, int hit);

int UpdateRiskFactor(Player * bot, Player * opponent);

int GetRandomProbCoordinateFromCategory(int * row, int * col, D_LinkedList * heapCategory);

int GetRandomHighestProbCell(int * row, int* col, Player * player);

int GetHighestProbCoordinateFromCategory(int * row, int * col, D_LinkedList * heapCategory);

int GetHighestProbability(int * row, int* col, Player * player);

int UpdateHeap(Player * player, int heapIndex);

int UpdateHeapsWithinBounds(Player * player, int row0, int row1, int col0, int col1);

BotTask * GetNextTask(Player * bot);

BotTask * CreateTask(int (*function)(void**), void** arguments, int argumentCount, void** flags, int flagCount);

int freeTask(BotTask * task);

int AssignNewTask(int priorityFlag,  Player * bot, int (*function)(void**), void** arguments, int argumentCount, void** flags, int flagCount);

int PerformTask(BotTask * task);

int BotFire(void** args);

int BotFireHelper(int row, int col, Player * bot, Player * opponent);

void BotAnalyzeShot(int row, int col, Player * bot, Player * opponent);

int RecalculateAllProbabilities(Player * player);

void ClearStackMemory(Player * bot);

int CanCellHoldShip(Player * opponent, int row, int col);

int BotPickRandomIndicesWithinBounds(int * outRow, int * outCol, char** grid, int shipSize[2], char* orientation, int startRow, int endRow, int startCol, int endCol);

int BotPickLowestProbabilityPlacement(int * outRow, int * outCol, char * outOrientation, Player * bot, int shipLength);

int PickRandomCell(int * row, int * col, Player * opponent, int onlyCellsThatCanHoldShips);

#endif
