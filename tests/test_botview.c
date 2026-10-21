/**
 * Tests for the bot's view (BotView.c) and the decision record it shows (BotDecision, filled in by Bot.c).
 */
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "fixtures.h"
#include "../include/Bot.h"
#include "../include/BotView.h"
#include "../include/CalcProbs.h"

#define TEST_SNAPSHOT_FILE "test-bot-view.txt"

void test_decision_category_rotation(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *target = NewTestPlayerWithFleet("target");

    CHECK_EQ_INT(bot->lastDecision.type, DECISION_NONE);

    //Normal risk, empty stack: the rotation starts with the high list
    int expectedCategory = bot->currTargetCategory;
    BotSmartAttack(bot, target);

    BotDecision d = bot->lastDecision;
    CHECK_EQ_INT(d.type, DECISION_CATEGORY);
    CHECK_EQ_INT(d.category, expectedCategory);
    CHECK_EQ_INT(d.region, HashRegion(d.row, d.col));
    CHECK_EQ_INT(d.riskFactor, NORMAL_RISK);
    CHECK_EQ_INT(d.explored, 0);
    CHECK(target->grid[d.row][d.col] == HIT || target->grid[d.row][d.col] == MISS); //it fired where it says it did
    CHECK(d.probability > 0);
}

void test_decision_task(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *target = NewTestPlayerWithFleet("target");

    BotFireHelper(2, 3, bot, target); //hit the destroyer (D3), its neighbours go on the stack
    BotSmartAttack(bot, target);

    BotDecision d = bot->lastDecision;
    CHECK_EQ_INT(d.type, DECISION_TASK);
    CHECK_EQ_INT(abs(d.row - 2) + abs(d.col - 3), 1); //a cell next to the hit
}

void test_decision_risk_levels(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *target = NewTestPlayerWithFleet("target");

    bot->riskFactor = HIGH_RISK;
    BotSmartAttack(bot, target);
    CHECK_EQ_INT(bot->lastDecision.type, DECISION_HIGHEST_CATEGORY);
    CHECK_EQ_INT(bot->lastDecision.category, HIGHPROB_CATEGORY);
    CHECK_EQ_INT(bot->lastDecision.riskFactor, HIGH_RISK);

    //Extremely high risk: the hottest cell of the whole grid
    Player *bot2 = NewTestPlayer("bot", true, SMART);
    Player *target2 = NewTestPlayerWithFleet("target");
    int hottest = 0;
    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
            hottest = MAX(hottest, target2->probabilityGrid[i][j]);

    bot2->riskFactor = EXTREMELYHIGH_RISK;
    BotSmartAttack(bot2, target2);
    CHECK_EQ_INT(bot2->lastDecision.type, DECISION_HOTTEST_CELL);
    CHECK_EQ_INT(bot2->lastDecision.probability, hottest);
}

void test_decision_low_risk_exploring(void){
    //At low risk the bot sometimes leaves its plans to explore. It's random, so give it plenty of chances.
    int explored = 0;

    for (int attempt = 0; attempt < 200 && !explored; attempt++){
        Player *bot = NewTestPlayer("bot", true, SMART);
        Player *target = NewTestPlayerWithFleet("target");

        BotFireHelper(2, 3, bot, target); //something to chase
        bot->riskFactor = LOW_RISK;
        BotSmartAttack(bot, target);

        if (bot->lastDecision.explored){
            explored = 1;
            CHECK(bot->lastDecision.type != DECISION_TASK);
            CHECK(!is_empty(bot->stackMemory)); //the plans are still waiting
        }
    }
    CHECK(explored);
}

void test_decision_weapons_and_easy_bot(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    Player *target = NewTestPlayerWithFleet("target");

    bot->prevSunk = 1;
    BotSmartAttack(bot, target);
    CHECK_EQ_INT(bot->lastDecision.type, DECISION_ARTILLERY);
    CHECK(bot->lastDecision.probability > 0);

    SinkShip(target, &target->submarineBounds);
    SinkShip(target, &target->destroyerBounds);
    SinkShip(target, &target->battleshipBounds);
    RecalculateAllProbabilities(target);
    BotSmartAttack(bot, target);
    CHECK_EQ_INT(bot->lastDecision.type, DECISION_TORPEDO);
    CHECK((bot->lastDecision.row >= 0) != (bot->lastDecision.col >= 0)); //either a row or a column

    Player *easy = NewTestPlayer("easy", true, DUMB);
    Player *target2 = NewTestPlayerWithFleet("target");
    BotDumbAttack(easy, target2);
    CHECK_EQ_INT(easy->lastDecision.type, DECISION_RANDOM);
    char shot = target2->grid[easy->lastDecision.row][easy->lastDecision.col];
    CHECK(shot == HIT || shot == MISS); //it fired where it says it did
}

void test_snapshot_round_trip(void){
    Player *bot = NewTestPlayer("Botteyi", true, SMART);
    Player *target = NewTestPlayerWithFleet("Ayoub");

    //Play a few turns: a sunk submarine, a damaged carrier, some misses
    BotFireHelper(4, 6, bot, target);
    BotFireHelper(5, 6, bot, target);
    BotFireHelper(0, 2, bot, target);
    BotFireHelper(9, 9, bot, target);
    RecordOpponentAttack(bot, 1);
    RecordOpponentAttack(bot, 0);

    CHECK_EQ_INT(WriteBotSnapshot(TEST_SNAPSHOT_FILE, bot, target, 7, 0), 1);

    BotSnapshot *s = malloc(sizeof(BotSnapshot));
    CHECK_EQ_INT(ReadBotSnapshot(TEST_SNAPSHOT_FILE, s), 1);

    CHECK_EQ_INT(s->turn, 7);
    CHECK_EQ_INT(s->gameOver, 0);
    CHECK_EQ_STR(s->botName, "Botteyi");
    CHECK_EQ_STR(s->opponentName, "Ayoub");
    CHECK_EQ_INT(s->botIQ, SMART);
    CHECK_EQ_INT(s->opponentShots, 2);
    CHECK_EQ_INT(s->opponentHits, 1);
    CHECK_EQ_INT(s->botSunk, 1);
    CHECK_EQ_INT(s->decision.type, bot->lastDecision.type);

    //The grid holds only what the bot knows, never where the ships are
    for (int i = 0; i < GRIDSIZE; i++){
        for (int j = 0; j < GRIDSIZE; j++){
            char c = s->grid[i][j];
            CHECK(!IsShip(c));
            if (target->grid[i][j] == MISS) CHECK_EQ_INT(c, MISS);
            else if (target->grid[i][j] == HIT) CHECK_EQ_INT(c, IsCellOfSunkShip(target, i, j) ? BOT_VIEW_SUNK : HIT);
            else {
                CHECK_EQ_INT(c, BOT_VIEW_UNKNOWN);
                CHECK_EQ_INT(s->probabilities[i][j], target->probabilityGrid[i][j]);
            }
        }
    }
    CHECK_EQ_INT(s->grid[4][6], BOT_VIEW_SUNK); //the submarine
    CHECK_EQ_INT(s->grid[0][2], HIT);           //the carrier, still afloat
    CHECK_EQ_INT(s->grid[1][1], BOT_VIEW_UNKNOWN);

    for (int r = 0; r < PROB_REGION_COUNT; r++) CHECK_EQ_INT(s->regionCategory[r], target->probabilityHeapCategory[r]);

    //The plans are the stack, in order
    CHECK_EQ_INT(s->taskCount, bot->stackMemory->size);
    int t = 0;
    for (D_ListNode *n = get_first(bot->stackMemory); n != NULL && t < s->taskCount; n = n->next, t++){
        BotTask *task = n->data;
        CHECK_EQ_INT(s->tasks[t][0], *(int *)task->arguments[0]);
        CHECK_EQ_INT(s->tasks[t][1], *(int *)task->arguments[1]);
    }

    //Each new snapshot has a new sequence number, which is how the viewer knows to redraw
    long first = s->sequence;
    WriteBotSnapshot(TEST_SNAPSHOT_FILE, bot, target, 8, 1);
    ReadBotSnapshot(TEST_SNAPSHOT_FILE, s);
    CHECK(s->sequence != first);
    CHECK_EQ_INT(s->gameOver, 1);

    free(s);
    remove(TEST_SNAPSHOT_FILE);
}

void test_snapshot_rejects_bad_files(void){
    BotSnapshot *s = malloc(sizeof(BotSnapshot));

    remove(TEST_SNAPSHOT_FILE);
    CHECK_EQ_INT(ReadBotSnapshot(TEST_SNAPSHOT_FILE, s), 0); //missing

    FILE *f = fopen(TEST_SNAPSHOT_FILE, "w");
    fprintf(f, "BATTLESHIP-BOT-VIEW 1\nsequence 5\ngridsize %d\nturn 3\n", GRIDSIZE); //cut off halfway
    fclose(f);
    CHECK_EQ_INT(ReadBotSnapshot(TEST_SNAPSHOT_FILE, s), 0);

    f = fopen(TEST_SNAPSHOT_FILE, "w");
    fprintf(f, "BATTLESHIP-BOT-VIEW 1\nsequence 5\ngridsize %d\n", GRIDSIZE + 1); //written by a game with another grid size
    fclose(f);
    CHECK_EQ_INT(ReadBotSnapshot(TEST_SNAPSHOT_FILE, s), 0);

    free(s);
    remove(TEST_SNAPSHOT_FILE);
}

//Draws the view of the snapshot written for this bot into a string
static char *DrawToString(Player *bot, Player *target){
    BotSnapshot *s = malloc(sizeof(BotSnapshot));
    WriteBotSnapshot(TEST_SNAPSHOT_FILE, bot, target, 1, 0);
    ReadBotSnapshot(TEST_SNAPSHOT_FILE, s);
    remove(TEST_SNAPSHOT_FILE);

    FILE *out = tmpfile();
    DrawBotView(out, s);
    long size = ftell(out);
    rewind(out);
    char *text = calloc(size + 1, 1);
    fread(text, 1, size, out);
    fclose(out);
    free(s);
    return text;
}

void test_draw_bot_view(void){
    Player *bot = NewTestPlayer("Botteyi", true, SMART);
    Player *target = NewTestPlayerWithFleet("Ayoub");
    BotFireHelper(2, 3, bot, target);
    BotSmartAttack(bot, target);

    char *text = DrawToString(bot, target);
    CHECK(strstr(text, "Botteyi's brain") != NULL);
    CHECK(strstr(text, "How Botteyi sees Ayoub's grid") != NULL);
    CHECK(strstr(text, "followed the plan on top of its stack") != NULL);
    CHECK(strstr(text, "Plan") != NULL);
    CHECK(strstr(text, "Regions") != NULL);
    CHECK(strstr(text, "Risk:") != NULL);
    free(text);

    Player *easy = NewTestPlayer("Easy", true, DUMB);
    Player *target2 = NewTestPlayerWithFleet("Ayoub");
    BotDumbAttack(easy, target2);
    text = DrawToString(easy, target2);
    CHECK(strstr(text, "Easy bot") != NULL);
    CHECK(strstr(text, "fired at a random cell") != NULL);
    CHECK(strstr(text, "Regions") == NULL); //the easy bot has no regions, plans or risk to show
    free(text);
}
