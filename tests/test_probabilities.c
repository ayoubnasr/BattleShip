/**
 * Tests for the probability system: the probability grid (CalcProbs.c) and the region heaps and categories (Player.c / Bot.c),
 * as described in the presentation (2.1 probability grid, 2.2 regions).
 */
#include <stdlib.h>
#include "test.h"
#include "fixtures.h"
#include "../include/Bot.h"
#include "../include/CalcProbs.h"

//Number of horizontal + vertical placements of a ship through a cell, per the presentation's formula Min(x + 1, GRIDWIDTH - x, SHIPLENGTH)
static int Placements(int row, int col, int shipLength){
    int h = MIN(MIN(col + 1, GRIDSIZE - col), shipLength);
    int v = MIN(MIN(row + 1, GRIDSIZE - row), shipLength);
    return h + v;
}

static int CountHeapElements(Node *node, Player *p, int *ok){
    if (node == NULL) return 0;
    int *elem = (int *)node->value;
    //Every element is a (probability, row, col) triple whose probability is the cell's current one, and the cell can still be targeted
    if (elem[0] != p->probabilityGrid[elem[1]][elem[2]]) *ok = 0;
    if (p->grid[elem[1]][elem[2]] == HIT || p->grid[elem[1]][elem[2]] == MISS) *ok = 0;
    if (elem[0] <= 0) *ok = 0;
    return 1 + CountHeapElements(node->leftChild, p, ok) + CountHeapElements(node->rightSibling, p, ok);
}

/**
 * Checks every invariant of the region system for a player:
 *  - every heap holds exactly the cells of its region that can still be targeted, with their current probability
 *  - every non-empty heap is in exactly one category list, the one matching its highest probability, through the stored node
 *  - empty heaps are in no list
 */
void CheckRegionSystem(Player *p){
    int listed = 0;

    for (int region = 0; region < PROB_REGION_COUNT; region++){
        BinomialHeap *heap = p->probabilityHeapSet[region];

        int expectedCount = 0, expectedMax = 0;
        for (int i = 0; i < GRIDSIZE; i++){
            for (int j = 0; j < GRIDSIZE; j++){
                if (HashRegion(i, j) != region) continue;
                if (p->grid[i][j] == HIT || p->grid[i][j] == MISS || p->probabilityGrid[i][j] <= 0) continue;
                expectedCount++;
                expectedMax = MAX(expectedMax, p->probabilityGrid[i][j]);
            }
        }

        int ok = 1;
        int count = CountHeapElements(heap->head, p, &ok);
        if (!ok || count != expectedCount){
            fprintf(stderr, "    region %d: heap has %d cells (expected %d)%s\n", region, count, expectedCount, ok ? "" : ", with a stale or shot cell");
        }
        CHECK(ok);
        CHECK_EQ_INT(count, expectedCount);

        int *top = BinHeap_FindHighestProbabilityCell(heap);
        int category = p->probabilityHeapCategory[region];

        if (expectedCount == 0){
            CHECK(top == NULL);
            CHECK_EQ_INT(category, NO_CATEGORY);
            CHECK(p->probabilityHeapNodes[region] == NULL);
        }
        else {
            CHECK(top != NULL);
            if (top) CHECK_EQ_INT(top[0], expectedMax);
            CHECK_EQ_INT(category, GetProbabilityCategory(expectedMax));
            CHECK(p->probabilityHeapNodes[region] != NULL);
            if (p->probabilityHeapNodes[region]) CHECK(p->probabilityHeapNodes[region]->data == heap);

            //The stored node really is in that list
            int found = 0;
            if (category >= 0){
                for (D_ListNode *n = get_first(&p->probabilityHeapCategoryLists[category]); n != NULL; n = n->next){
                    if (n == p->probabilityHeapNodes[region]) found++;
                }
            }
            CHECK_EQ_INT(found, 1);
            listed++;
        }
    }

    int listSizes = 0;
    for (int c = 0; c < PROB_CATEGORYCOUNT; c++) listSizes += p->probabilityHeapCategoryLists[c].size;
    CHECK_EQ_INT(listSizes, listed);
}

void test_initial_probability_grid(void){
    Player *p = NewTestPlayer("grid", false, DUMB);
    int sizes[] = SHIPSIZES;

    for (int i = 0; i < GRIDSIZE; i++){
        for (int j = 0; j < GRIDSIZE; j++){
            int expected = 0;
            for (int s = 0; s < SHIPCOUNT; s++) expected += Placements(i, j, sizes[s]);
            CHECK_EQ_INT(p->probabilityGrid[i][j], expected);
            //Symmetric across both axes
            CHECK_EQ_INT(p->probabilityGrid[i][j], p->probabilityGrid[GRIDSIZE - 1 - i][GRIDSIZE - 1 - j]);
        }
    }

    CHECK_EQ_INT(p->probabilityGrid[0][0], 2 * SHIPCOUNT);    //a corner: 1 way each direction for every ship
    CHECK_EQ_INT(p->probabilityGrid[4][4], 2 * (2 + 3 + 4 + 5)); //the center: the maximum, 28
}

void test_initial_region_system(void){
    Player *p = NewTestPlayer("grid", false, DUMB);
    CheckRegionSystem(p);

    //With 2x2 regions on a 10x10 grid there are 25 regions, all with cells to target
    CHECK_EQ_INT(PROB_REGION_COUNT, 25);
    int total = 0;
    for (int c = 0; c < PROB_CATEGORYCOUNT; c++) total += p->probabilityHeapCategoryLists[c].size;
    CHECK_EQ_INT(total, PROB_REGION_COUNT);
    CHECK(p->probabilityHeapCategoryLists[HIGHPROB_CATEGORY].size > 0); //the center is high probability
}

void test_hash_region(void){
    CHECK_EQ_INT(HashRegion(0, 0), 0);
    CHECK_EQ_INT(HashRegion(1, 1), 0);
    CHECK_EQ_INT(HashRegion(0, 2), 1);
    CHECK_EQ_INT(HashRegion(2, 0), PROB_REGIONS_PER_ROW);
    CHECK_EQ_INT(HashRegion(GRIDSIZE - 1, GRIDSIZE - 1), PROB_REGION_COUNT - 1);

    //Every region index is used, by exactly PROB_REGION_WIDTH * PROB_REGION_HEIGHT cells on this grid
    int counts[PROB_REGION_COUNT] = {0};
    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
            counts[HashRegion(i, j)]++;
    for (int r = 0; r < PROB_REGION_COUNT; r++) CHECK_EQ_INT(counts[r], PROB_REGION_WIDTH * PROB_REGION_HEIGHT);
}

void test_probability_categories(void){
    CHECK_EQ_INT(GetProbabilityCategory(HIGHPROB_BASE), HIGHPROB_CATEGORY);
    CHECK_EQ_INT(GetProbabilityCategory(HIGHPROB_BASE - 1), AVGPROB_CATEGORY);
    CHECK_EQ_INT(GetProbabilityCategory(AVGPROB_BASE), AVGPROB_CATEGORY);
    CHECK_EQ_INT(GetProbabilityCategory(LOWPROB_BASE), LOWPROB_CATEGORY);
    CHECK_EQ_INT(GetProbabilityCategory(0), NO_CATEGORY);
    CHECK_EQ_INT(GetProbabilityCategory(-3), NO_CATEGORY);
}

void test_miss_lowers_and_hit_raises_neighbours(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *target = NewTestPlayerWithFleet("target");

    //Miss at I8 (row 7, col 8): water. The cells next to it lose probability.
    int before = target->probabilityGrid[7][7];
    CHECK_EQ_INT(BotFireHelper(7, 8, bot, target), 1);
    CHECK_EQ_INT(target->grid[7][8], MISS);
    CHECK(target->probabilityGrid[7][7] < before);
    CheckRegionSystem(target);
    CHECK(is_empty(bot->stackMemory)); //a miss gives the bot nothing to chase

    //Hit at C3 (row 2, col 2): destroyer. The cells next to it gain probability, and the bot queues its 4 neighbours.
    before = target->probabilityGrid[2][3];
    CHECK_EQ_INT(BotFireHelper(2, 2, bot, target), 1);
    CHECK_EQ_INT(target->grid[2][2], HIT);
    CHECK(target->probabilityGrid[2][3] > before);
    CheckRegionSystem(target);
    CHECK_EQ_INT(bot->stackMemory->size, 4);

    //Firing at the same cell again does nothing (and returns instead of looping forever)
    CHECK_EQ_INT(BotFireHelper(2, 2, bot, target), -1);
}

void test_sinking_recalculates_and_clears_tasks(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *target = NewTestPlayerWithFleet("target");

    //Sink the submarine (G5-G6: rows 4-5, column 6)
    BotFireHelper(4, 6, bot, target);
    CHECK(bot->stackMemory->size > 0);
    BotFireHelper(5, 6, bot, target);

    CHECK(target->submarineBounds.IsSunk);
    CHECK(is_empty(bot->stackMemory)); //nothing left to chase around a sunk ship
    CheckRegionSystem(target);

    //Every cell's probability now matches a full recalculation without the submarine
    int **snapshot = malloc(sizeof(int *) * GRIDSIZE);
    for (int i = 0; i < GRIDSIZE; i++){
        snapshot[i] = malloc(sizeof(int) * GRIDSIZE);
        for (int j = 0; j < GRIDSIZE; j++) snapshot[i][j] = target->probabilityGrid[i][j];
    }
    RecalculateAllProbabilities(target);
    for (int i = 0; i < GRIDSIZE; i++){
        for (int j = 0; j < GRIDSIZE; j++) CHECK_EQ_INT(target->probabilityGrid[i][j], snapshot[i][j]);
        free(snapshot[i]);
    }
    free(snapshot);

    //A cell on the far side has lost the submarine's share of the probability
    CHECK_EQ_INT(target->probabilityGrid[9][0], 2 * (SHIPCOUNT - 1));
}

void test_sinking_keeps_tasks_for_other_damaged_ship(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *target = NewTestPlayerWithFleet("target");

    BotFireHelper(0, 0, bot, target); //hit the carrier (A1), it stays afloat
    BotFireHelper(4, 6, bot, target); //sink the submarine
    BotFireHelper(5, 6, bot, target);

    CHECK(target->submarineBounds.IsSunk);
    CHECK(!is_empty(bot->stackMemory)); //the damaged carrier is still worth chasing
}

void test_cell_can_hold_ship(void){
    Player *target = NewTestPlayerWithFleet("target");

    CHECK(CanCellHoldShip(target, 9, 9));

    //Surround the corner cell J10 with misses: no ship can fit there anymore
    target->grid[8][9] = MISS;
    target->grid[9][8] = MISS;
    CHECK(!CanCellHoldShip(target, 9, 9));

    target->grid[5][5] = MISS;
    CHECK(!CanCellHoldShip(target, 5, 5)); //a miss itself
}
