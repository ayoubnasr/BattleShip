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

void test_dumb_bot_wins_and_never_repeats(void){
    for (int game = 0; game < 10; game++){
        Player *bot = NewTestPlayer("bot", true, DUMB);
        Player *target = NewTestPlayerWithFleet("target");

        PlayUntilSunk(bot, target, BotDumbAttack, 0);
        CHECK(is_empty(bot->stackMemory)); //the dumb bot doesn't chase its hits
    }
}

void test_dumb_bot_skips_cells_that_cannot_hold_ships(void){
    Player *bot = NewTestPlayer("bot", true, DUMB);
    Player *target = NewTestPlayerWithFleet("target");

    //Box in the bottom-right corner cell with misses: the bot must never pick it while other cells remain
    target->grid[8][9] = MISS;
    target->grid[9][8] = MISS;
    for (int turn = 0; turn < 50; turn++){
        BotDumbAttack(bot, target);
        CHECK_EQ_INT(target->grid[9][9], WATER_C);
    }
}

//Checks that the fleet is complete: every ship on the grid with the right length, straight, and nothing else
static void CheckFleet(Player *p){
    ShipBounds *ships[] = {&p->carrierBounds, &p->battleshipBounds, &p->destroyerBounds, &p->submarineBounds};
    char chars[] = {CARRIER_C, BATTLESHIP_C, DESTROYER_C, SUBMARINE_C};
    int lengths[] = {CARRIER_LENGTH, BATTLESHIP_LENGTH, DESTROYER_LENGTH, SUBMARINE_LENGTH};

    int shipCells = 0;
    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
            if (IsShip(p->grid[i][j])) shipCells++;
    CHECK_EQ_INT(shipCells, CARRIER_LENGTH + BATTLESHIP_LENGTH + DESTROYER_LENGTH + SUBMARINE_LENGTH);

    for (int s = 0; s < SHIPCOUNT; s++){
        ShipBounds *b = ships[s];
        CHECK(b->startRow == b->endRow || b->startCol == b->endCol);
        CHECK_EQ_INT((b->endRow - b->startRow) + (b->endCol - b->startCol) + 1, lengths[s]);
        for (int i = b->startRow; i <= b->endRow; i++)
            for (int j = b->startCol; j <= b->endCol; j++)
                CHECK_EQ_INT(p->grid[i][j], chars[s]);
    }
}

void test_bot_ship_placement(void){
    int usedLastRow = 0, usedLastCol = 0;

    for (int game = 0; game < 200; game++){
        Player *easy = NewTestPlayer("easy", true, DUMB);
        PlaceBotShips(easy);
        CheckFleet(easy);

        for (int k = 0; k < GRIDSIZE; k++){
            if (IsShip(easy->grid[GRIDSIZE - 1][k])) usedLastRow = 1;
            if (IsShip(easy->grid[k][GRIDSIZE - 1])) usedLastCol = 1;
        }

        Player *hard = NewTestPlayer("hard", true, SMART);
        PlaceBotShips(hard);
        CheckFleet(hard);
    }

    //The random placement can reach the last row and column (it used to stop one short)
    CHECK(usedLastRow);
    CHECK(usedLastCol);
}

void test_lowest_probability_placement(void){
    Player *bot = NewTestPlayer("bot", true, SMART);

    //Brute force the lowest probability sum of a carrier on the empty grid
    int lowest = -1;
    for (int o = 0; o < 2; o++){
        for (int i = 0; i < GRIDSIZE; i++){
            for (int j = 0; j < GRIDSIZE; j++){
                int sum = 0, fits = 1;
                for (int k = 0; k < CARRIER_LENGTH; k++){
                    int r = (o == 0) ? i : i + k, c = (o == 0) ? j + k : j;
                    if (r >= GRIDSIZE || c >= GRIDSIZE){ fits = 0; break; }
                    sum += bot->probabilityGrid[r][c];
                }
                if (fits && (lowest < 0 || sum < lowest)) lowest = sum;
            }
        }
    }

    for (int trial = 0; trial < 20; trial++){
        int row = -1, col = -1;
        char orientation[2];
        CHECK_EQ_INT(BotPickLowestProbabilityPlacement(&row, &col, orientation, bot, CARRIER_LENGTH), 1);

        int sum = 0;
        for (int k = 0; k < CARRIER_LENGTH; k++){
            sum += (orientation[0] == 'h') ? bot->probabilityGrid[row][col + k] : bot->probabilityGrid[row + k][col];
        }
        CHECK_EQ_INT(sum, lowest);
    }
}

void test_smart_bot_uses_artillery_after_sinking(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *target = NewTestPlayerWithFleet("target");

    bot->prevSunk = 1;
    int before = CountShots(target);
    BotSmartAttack(bot, target);
    CHECK_EQ_INT(CountShots(target), before + 4); //a 2x2 area
    CheckRegionSystem(target);
}

void test_smart_bot_uses_torpedo_after_third_ship(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *target = NewTestPlayerWithFleet("target");

    SinkShip(target, &target->submarineBounds);
    SinkShip(target, &target->destroyerBounds);
    SinkShip(target, &target->battleshipBounds);
    RecalculateAllProbabilities(target);

    bot->prevSunk = 1;
    BotSmartAttack(bot, target);

    //The best line is the one with the most probability left: it must have fired a whole row or column, through the carrier (row 1)
    int fullRows = 0, fullCols = 0;
    for (int k = 0; k < GRIDSIZE; k++){
        int rowShot = 1, colShot = 1;
        for (int m = 0; m < GRIDSIZE; m++){
            if (target->grid[k][m] != HIT && target->grid[k][m] != MISS) rowShot = 0;
            if (target->grid[m][k] != HIT && target->grid[m][k] != MISS) colShot = 0;
        }
        fullRows += rowShot;
        fullCols += colShot;
    }
    CHECK(fullRows + fullCols >= 1);
    CheckRegionSystem(target);
}

void test_smart_bot_full_games_with_weapons(void){
    //Plays like the driver does: after a turn that sank a ship, the bot may use its artillery/torpedo on the next turn
    for (int game = 0; game < 20; game++){
        Player *bot = NewTestPlayer("bot", true, SMART);
        Player *target = NewTestPlayer("target", false, DUMB);
        PlaceBotShips(target); //random fleet

        int turns = 0, sunk = 0;
        while (countSunkShips(target) < SHIPCOUNT && turns < GRIDSIZE * GRIDSIZE){
            int before = CountShots(target);
            BotSmartAttack(bot, target);
            turns++;
            CHECK(CountShots(target) > before); //every turn shoots something new
            int nowSunk = countSunkShips(target);
            bot->prevSunk = nowSunk > sunk;
            sunk = nowSunk;
        }
        CHECK_EQ_INT(countSunkShips(target), SHIPCOUNT);
        if (game == 0) CheckRegionSystem(target);
    }
}

void test_risk_factor(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *human = NewTestPlayerWithFleet("human");
    PlaceBotShips(bot);

    //Starts at normal risk, and stays there before the opponent attacked enough
    CHECK_EQ_INT(bot->riskFactor, NORMAL_RISK);
    RecordOpponentAttack(bot, 1);
    CHECK_EQ_INT(UpdateRiskFactor(bot, human), NORMAL_RISK);

    //An opponent that keeps missing: low risk
    Player *bot2 = NewTestPlayer("bot", true, SMART);
    PlaceBotShips(bot2);
    for (int i = 0; i < 10; i++) RecordOpponentAttack(bot2, 0);
    CHECK_EQ_INT(UpdateRiskFactor(bot2, human), LOW_RISK);
    CHECK_EQ_INT(bot2->opponentShots, 10);
    CHECK_EQ_INT(bot2->opponentHits, 0);

    //Accurate overall, and hitting recently: two signs of danger, high risk
    Player *bot3 = NewTestPlayer("bot", true, SMART);
    PlaceBotShips(bot3);
    for (int i = 0; i < 10; i++) RecordOpponentAttack(bot3, i % 2);
    CHECK_EQ_INT(UpdateRiskFactor(bot3, human), HIGH_RISK);

    //...and it has sunk a ship while the bot sank none: extremely high risk
    SinkShip(bot3, &bot3->submarineBounds);
    CHECK_EQ_INT(UpdateRiskFactor(bot3, human), EXTREMELYHIGH_RISK);

    //Only the last RISK_RECENT_TURNS attacks count for the success rate: hits long ago, misses lately
    Player *bot4 = NewTestPlayer("bot", true, SMART);
    PlaceBotShips(bot4);
    for (int i = 0; i < 4; i++) RecordOpponentAttack(bot4, 1);
    for (int i = 0; i < RISK_RECENT_TURNS; i++) RecordOpponentAttack(bot4, 0);
    CHECK_EQ_INT(UpdateRiskFactor(bot4, human), NORMAL_RISK); //accuracy 4/9 is high, success rate 0 is not
}
