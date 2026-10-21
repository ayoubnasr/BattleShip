/**
 * Tests for the bot (Bot.c): its stack memory of tasks ("the brain"), firing, and full games played by the bots.
 */
#include <stdlib.h>
#include "test.h"
#include "fixtures.h"
#include "../include/Bot.h"

static int taskCalls = 0;
static int lastTaskArgument = 0;

static int RecordingTask(void **args){
    taskCalls++;
    lastTaskArgument = *(int *)args[0];
    return 1;
}

static void **OneIntArgument(int value){
    void **args = malloc(sizeof(void *));
    int *v = malloc(sizeof(int));
    *v = value;
    args[0] = v;
    return args;
}

void test_bot_task_stack_order(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    CHECK(bot->stackMemory != NULL);
    CHECK(is_empty(bot->stackMemory));

    //High priority goes on top of the stack, low priority at the bottom:
    AssignNewTask(TASKFLAG_LOWPRIORITY, bot, RecordingTask, OneIntArgument(1), 1, NULL, 0);
    AssignNewTask(TASKFLAG_HIGHPRIORITY, bot, RecordingTask, OneIntArgument(2), 1, NULL, 0);
    AssignNewTask(TASKFLAG_HIGHPRIORITY, bot, RecordingTask, OneIntArgument(3), 1, NULL, 0);
    CHECK_EQ_INT(AssignNewTask(42, bot, RecordingTask, NULL, 0, NULL, 0), 0); //unknown priority flag
    CHECK_EQ_INT(bot->stackMemory->size, 3);

    int expectedOrder[3] = {3, 2, 1};
    taskCalls = 0;
    for (int i = 0; i < 3; i++){
        BotTask *task = GetNextTask(bot);
        CHECK(task != NULL);
        if (task == NULL) break;
        CHECK_EQ_INT(PerformTask(task), 1);
        CHECK_EQ_INT(lastTaskArgument, expectedOrder[i]);
        CHECK_EQ_INT(freeTask(task), 1);
    }
    CHECK_EQ_INT(taskCalls, 3);
    CHECK(GetNextTask(bot) == NULL);
}

void test_bot_task_creation(void){
    void **args = OneIntArgument(7);
    BotTask *task = CreateTask(RecordingTask, args, 1, NULL, 0);
    CHECK(task != NULL);
    if (task == NULL) return;
    CHECK(task->function == RecordingTask);
    CHECK(task->arguments == args);
    CHECK_EQ_INT(task->argumentCount, 1);
    freeTask(task);

    BotTask *empty = CreateTask(NULL, NULL, 0, NULL, 0);
    CHECK_EQ_INT(PerformTask(empty), 0); //a task with no function does nothing
    freeTask(empty);
}

void CheckRegionSystem(Player *p); //test_probabilities.c

static int CountShots(Player *p){
    int shots = 0;
    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
            if (p->grid[i][j] == HIT || p->grid[i][j] == MISS) shots++;
    return shots;
}

/**
 * Lets the bot attack the target until every ship is sunk. Every turn must shoot exactly one new cell (never the same one twice) and
 * the region system must stay consistent. Returns the number of turns it took.
 */
static int PlayUntilSunk(Player *bot, Player *target, void (*attack)(Player *, Player *), int checkRegions){
    int turns = 0;
    while (countSunkShips(target) < SHIPCOUNT && turns < GRIDSIZE * GRIDSIZE){
        int before = CountShots(target);
        attack(bot, target);
        turns++;
        CHECK_EQ_INT(CountShots(target), before + 1);
        if (checkRegions) CheckRegionSystem(target);
    }
    CHECK_EQ_INT(countSunkShips(target), SHIPCOUNT);
    return turns;
}

void test_smart_bot_wins_at_every_risk_level(void){
    int risks[] = {LOW_RISK, NORMAL_RISK, HIGH_RISK, EXTREMELYHIGH_RISK};

    for (int r = 0; r < 4; r++){
        for (int game = 0; game < 5; game++){
            Player *bot = NewTestPlayer("bot", true, SMART);
            Player *target = NewTestPlayerWithFleet("target");
            bot->riskFactor = risks[r];

            int turns = PlayUntilSunk(bot, target, BotSmartAttack, game == 0);
            CHECK(turns <= GRIDSIZE * GRIDSIZE);
        }
    }
}

void test_smart_bot_fleet_in_corner(void){
    //Ships packed along the edges (lowest starting probability) still get found
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *target = NewTestPlayer("target", false, DUMB);
    PlaceShip(target, CARRIER_C, "J1", "v");
    PlaceShip(target, BATTLESHIP_C, "A10", "h");
    PlaceShip(target, DESTROYER_C, "A1", "v");
    PlaceShip(target, SUBMARINE_C, "I10", "h");

    PlayUntilSunk(bot, target, BotSmartAttack, 1);
}
