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

void BotAverageAttack(Player * bot, Player * opponent);

void BotDumbAttack(Player * bot, Player * opponent);

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

void PlaceBotShips(Player * bot);

#endif