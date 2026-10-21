#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/BotView.h"
#include "../include/Bot.h"
#include "../include/Player.h"
#include "../include/UITools.h"
#include "../include/InputLib.h"
#include "../include/coordslib.h"


#pragma region [Writing and Reading Snapshots]

/**
 * Writes a snapshot of the bot's mind to the file at path: what it knows about the opponent's grid, its probabilities, its regions, the
 * plans on its stack, its risk, and its last decision.
 *
 * The snapshot is written to a temporary file first, then renamed, so the viewer never reads a half-written file.
 *
 * Returns 1 on success, -1 if the file could not be written.
 */
int WriteBotSnapshot(const char * path, Player * bot, Player * opponent, int turn, int gameOver){

    //The sequence starts from the clock so that a viewer left open from a previous game still sees the new game as new:
    static long sequence = 0;
    if (sequence == 0) sequence = ((long)time(NULL) % 1000000) * 1000;
    sequence++;

    char * tempPath = CreateString_alloc(2, (char *)path, ".tmp");
    FILE * file = fopen(tempPath, "w");

    if (file == NULL){
        free(tempPath);
        return -1;
    }

    //Risk numbers, the same way UpdateRiskFactor() computes them:
    int recentTurns = MIN(bot->opponentShots, RISK_RECENT_TURNS);
    int recentHits = 0;
    for (int i = 0; i < recentTurns; i++)
    {
        recentHits += bot->recentOpponentHits[i];
    }

    fprintf(file, "BATTLESHIP-BOT-VIEW 1\n");
    fprintf(file, "sequence %ld\n", sequence);
    fprintf(file, "gridsize %d\n", GRIDSIZE);
    fprintf(file, "turn %d\n", turn);
    fprintf(file, "gameover %d\n", gameOver ? 1 : 0);
    fprintf(file, "bot %s\n", bot->name);
    fprintf(file, "opponent %s\n", opponent->name);
    fprintf(file, "iq %d\n", bot->botIQ);
    fprintf(file, "risk %d %d %d %d %d %d %d\n", bot->riskFactor, bot->opponentShots, bot->opponentHits, recentHits, recentTurns,
        countSunkShips(bot), countSunkShips(opponent));

    BotDecision * d = &bot->lastDecision;
    fprintf(file, "decision %d %d %d %d %d %d %d %d\n", d->type, d->row, d->col, d->probability, d->category, d->region, d->explored, d->riskFactor);

    //Only what the bot knows: hits, misses and sunk ships. Ships that were not hit stay unknown.
    fprintf(file, "grid\n");
    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            char c = opponent->grid[i][j];
            char known = BOT_VIEW_UNKNOWN;

            if (c == HIT) known = IsCellOfSunkShip(opponent, i, j) ? BOT_VIEW_SUNK : HIT;
            else if (c == MISS) known = MISS;

            fputc(known, file);
        }
        fputc('\n', file);
    }

    fprintf(file, "probabilities\n");
    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            int value = 0;

            if (opponent->grid[i][j] != HIT && opponent->grid[i][j] != MISS){
                //The smart bot works with the probability grid, the easy bot only knows whether a ship could still be there
                value = (bot->botIQ == SMART) ? opponent->probabilityGrid[i][j] : CanCellHoldShip(opponent, i, j);
            }

            fprintf(file, "%d ", value);
        }
        fputc('\n', file);
    }

    fprintf(file, "thresholds %d %d\n", opponent->highProbBase, opponent->avgProbBase);

    fprintf(file, "regions %d\n", PROB_REGION_COUNT);
    for (int r = 0; r < PROB_REGION_COUNT; r++)
    {
        fprintf(file, "%d ", opponent->probabilityHeapCategory[r]);
    }
    fputc('\n', file);

    //The fire tasks on the bot's stack, from the top (next) down:
    int taskCount = 0;
    int tasks[BOT_VIEW_MAX_TASKS][2];

    if (bot->stackMemory != NULL){
        for (D_ListNode * node = get_first(bot->stackMemory); node != NULL && taskCount < BOT_VIEW_MAX_TASKS; node = get_next(node))
        {
            BotTask * task = (BotTask *)get_data(node);

            if (task->function == BotFire && task->arguments != NULL){
                tasks[taskCount][0] = *(int *)task->arguments[0];
                tasks[taskCount][1] = *(int *)task->arguments[1];
                taskCount++;
            }
        }
    }

    fprintf(file, "tasks %d\n", taskCount);
    for (int t = 0; t < taskCount; t++)
    {
        fprintf(file, "%d %d\n", tasks[t][0], tasks[t][1]);
    }

    fprintf(file, "end\n");
    fclose(file);

    //Replace the old snapshot (rename() won't overwrite an existing file on Windows):
    remove(path);
    int renamed = rename(tempPath, path);
    free(tempPath);

    return (renamed == 0) ? 1 : -1;
}

//Reads the next word of the file and checks that it is the expected one
static int ExpectWord(FILE * file, const char * expected){
    char word[32];
    return fscanf(file, "%31s", word) == 1 && strcmp(word, expected) == 0;
}

/**
 * Reads a snapshot written by WriteBotSnapshot().
 *
 * Returns 1 on success, and 0 if the file is missing, incomplete, or was written for another grid size.
 */
int ReadBotSnapshot(const char * path, BotSnapshot * s){

    FILE * file = fopen(path, "r");
    if (file == NULL) return 0;

    int ok = 1;
    int version = 0, gridSize = 0, regionCount = 0, type = 0;

    ok = ok && ExpectWord(file, "BATTLESHIP-BOT-VIEW") && fscanf(file, "%d", &version) == 1 && version == 1;
    ok = ok && ExpectWord(file, "sequence") && fscanf(file, "%ld", &s->sequence) == 1;
    ok = ok && ExpectWord(file, "gridsize") && fscanf(file, "%d", &gridSize) == 1 && gridSize == GRIDSIZE;
    ok = ok && ExpectWord(file, "turn") && fscanf(file, "%d", &s->turn) == 1;
    ok = ok && ExpectWord(file, "gameover") && fscanf(file, "%d", &s->gameOver) == 1;
    ok = ok && ExpectWord(file, "bot") && fscanf(file, "%31s", s->botName) == 1;
    ok = ok && ExpectWord(file, "opponent") && fscanf(file, "%31s", s->opponentName) == 1;
    ok = ok && ExpectWord(file, "iq") && fscanf(file, "%d", &s->botIQ) == 1;
    ok = ok && ExpectWord(file, "risk") && fscanf(file, "%d %d %d %d %d %d %d", &s->riskFactor, &s->opponentShots, &s->opponentHits,
        &s->recentHits, &s->recentTurns, &s->opponentSunk, &s->botSunk) == 7;
    ok = ok && ExpectWord(file, "decision") && fscanf(file, "%d %d %d %d %d %d %d %d", &type, &s->decision.row, &s->decision.col,
        &s->decision.probability, &s->decision.category, &s->decision.region, &s->decision.explored, &s->decision.riskFactor) == 8;
    s->decision.type = (BotDecisionType)type;

    ok = ok && ExpectWord(file, "grid");
    for (int i = 0; ok && i < GRIDSIZE; i++)
    {
        for (int j = 0; ok && j < GRIDSIZE; j++)
        {
            ok = fscanf(file, " %c", &s->grid[i][j]) == 1;
        }
    }

    ok = ok && ExpectWord(file, "probabilities");
    for (int i = 0; ok && i < GRIDSIZE; i++)
    {
        for (int j = 0; ok && j < GRIDSIZE; j++)
        {
            ok = fscanf(file, "%d", &s->probabilities[i][j]) == 1;
        }
    }

    ok = ok && ExpectWord(file, "thresholds") && fscanf(file, "%d %d", &s->highProbBase, &s->avgProbBase) == 2;

    ok = ok && ExpectWord(file, "regions") && fscanf(file, "%d", &regionCount) == 1 && regionCount == PROB_REGION_COUNT;
    for (int r = 0; ok && r < PROB_REGION_COUNT; r++)
    {
        ok = fscanf(file, "%d", &s->regionCategory[r]) == 1;
    }

    ok = ok && ExpectWord(file, "tasks") && fscanf(file, "%d", &s->taskCount) == 1 && s->taskCount >= 0 && s->taskCount <= BOT_VIEW_MAX_TASKS;
    for (int t = 0; ok && t < s->taskCount; t++)
    {
        ok = fscanf(file, "%d %d", &s->tasks[t][0], &s->tasks[t][1]) == 2;
    }

    ok = ok && ExpectWord(file, "end");

    fclose(file);

    return ok ? 1 : 0;
}

#pragma endregion


#pragma region [Drawing the Bot View]

//Name and color of each category list
static const char * CategoryName(int category){
    switch (category)
    {
    case HIGHPROB_CATEGORY: return "high";
    case AVGPROB_CATEGORY: return "average";
    case LOWPROB_CATEGORY: return "low";
    default: return "no";
    }
}

static const char * CategoryColor(int category){
    switch (category)
    {
    case HIGHPROB_CATEGORY: return BRIGHT_RED;
    case AVGPROB_CATEGORY: return YELLOW;
    case LOWPROB_CATEGORY: return CYAN;
    default: return BRIGHT_BLACK;
    }
}

static const char * RiskName(int risk){
    switch (risk)
    {
    case LOW_RISK: return "LOW";
    case NORMAL_RISK: return "NORMAL";
    case HIGH_RISK: return "HIGH";
    default: return "EXTREMELY HIGH";
    }
}

static const char * RiskColor(int risk){
    switch (risk)
    {
    case LOW_RISK: return GREEN;
    case NORMAL_RISK: return WHITE;
    case HIGH_RISK: return YELLOW;
    default: return BRIGHT_RED;
    }
}

static const char * RiskMeaning(int risk){
    switch (risk)
    {
    case LOW_RISK: return "the opponent is struggling, so the bot takes risks: it takes turns between the high, average and low lists, and sometimes explores elsewhere in the middle of sinking a ship.";
    case NORMAL_RISK: return "the bot takes turns between the high, average and low lists, and finishes every ship it hits.";
    case HIGH_RISK: return "the bot feels threatened: it only picks from the highest probability list.";
    default: return "the bot is in danger: it fires at the single hottest cell of the grid.";
    }
}

//The cell's coordinates as shown on the grid (eg. "D05")
static char * CellName_alloc(int row, int col){
    return alloc_GetCoordsFromIndices(row, col, GRIDSIZE, startingCoordinate_1, startingCoordinate_2, endingCoordinate_1, endingCoordinate_2,
        coord_1_shift, coord_2_shift);
}

//Whether the cell was targeted by the bot's last decision (one cell, the artillery's 2x2 area, or the torpedo's line)
static int IsLastTarget(BotSnapshot * s, int row, int col){
    BotDecision * d = &s->decision;

    switch (d->type)
    {
    case DECISION_NONE: return 0;
    case DECISION_ARTILLERY: return row >= d->row && row <= d->row + 1 && col >= d->col && col <= d->col + 1;
    case DECISION_TORPEDO: return (d->row >= 0 && row == d->row) || (d->col >= 0 && col == d->col);
    default: return row == d->row && col == d->col;
    }
}

//Position of the cell in the bot's plan (1 = next), or 0 if it's not planned. Planned cells that were shot since don't count.
static int PlanPosition(BotSnapshot * s, int row, int col){
    int position = 0;

    for (int t = 0; t < s->taskCount; t++)
    {
        int r = s->tasks[t][0], c = s->tasks[t][1];
        if (s->grid[r][c] != BOT_VIEW_UNKNOWN) continue;

        position++;
        if (r == row && c == col) return position;
    }

    return 0;
}

static void DrawGrid(FILE * out, BotSnapshot * s){

    int smart = s->botIQ == SMART;

    fprintf(out, "     ");
    for (int j = 0; j < GRIDSIZE; j++)
    {
        char * num = alloc_IntegerToNumeral(j + coord_1_shift, GRIDSIZE, startingCoordinate_1, endingCoordinate_1);
        fprintf(out, "%3s ", num); //Lined up with the last character of each cell's number
        free(num);
    }
    fprintf(out, "\n");

    for (int i = 0; i < GRIDSIZE; i++)
    {
        char * num = alloc_IntegerToNumeral(i + coord_2_shift, GRIDSIZE, startingCoordinate_2, endingCoordinate_2);
        fprintf(out, "  %3s", num);
        free(num);

        for (int j = 0; j < GRIDSIZE; j++)
        {
            char cell = s->grid[i][j];
            int value = s->probabilities[i][j];
            int planned = smart ? PlanPosition(s, i, j) : 0;
            const char * color = WHITE;
            char text[16];

            if (cell == HIT){ color = RED; sprintf(text, "  x"); }
            else if (cell == MISS){ color = WHITE; sprintf(text, "  O"); }
            else if (cell == BOT_VIEW_SUNK){ color = BRIGHT_BLACK; sprintf(text, "  #"); }
            else if (!smart){
                if (value > 0){ color = WHITE; sprintf(text, "  ?"); }
                else { color = BRIGHT_BLACK; sprintf(text, "  ."); }
            }
            else if (planned > 0){
                color = BRIGHT_MAGENTA;
                sprintf(text, "*%2d", MAX(-9, MIN(value, 99)));
            }
            else if (value <= 0){ color = BRIGHT_BLACK; sprintf(text, "  ."); }
            else {
                int category = (value >= s->highProbBase) ? HIGHPROB_CATEGORY : (value >= s->avgProbBase) ? AVGPROB_CATEGORY : LOWPROB_CATEGORY;
                color = CategoryColor(category);
                sprintf(text, "%3d", MIN(value, 999));
            }

            //The cells the bot just fired at are marked with '<'
            int last = IsLastTarget(s, i, j);
            fprintf(out, "%s%s%s%s%c%s", (planned == 1) ? BOLD : "", color, text, last ? BRIGHT_WHITE : "", last ? '<' : ' ', RESET);
        }
        fprintf(out, "\n");
    }

    fprintf(out, "\n");
    if (smart){
        fprintf(out, "  Chance: %s%d+%s high   %s%d+%s average   %s%d+%s low   %s.%s no ship fits\n",
            BRIGHT_RED, s->highProbBase, RESET, YELLOW, s->avgProbBase, RESET, CYAN, LOWPROB_BASE, RESET, BRIGHT_BLACK, RESET);
        fprintf(out, "  Marks:  %sx%s hit   O miss   %s#%s sunk   %s*%s planned   < last shot\n", RED, RESET, BRIGHT_BLACK, RESET, BRIGHT_MAGENTA, RESET);
    }
    else {
        fprintf(out, "  Chance: ? a ship could be here   %s.%s no ship fits\n", BRIGHT_BLACK, RESET);
        fprintf(out, "  Marks:  %sx%s hit   O miss   %s#%s sunk   < last shot\n", RED, RESET, BRIGHT_BLACK, RESET);
    }
}

/**
 * Prints text word by word, starting a new indented line before a word would go past BOT_VIEW_TEXT_WIDTH, so long sentences stay
 * readable in a narrow window. Color codes take no room on screen, so they don't count in the width.
 * 
 * column is how many characters are already on the current line, indent is where the following lines start.
 */
static void PrintWrapped(FILE * out, const char * text, int column, int indent){

    const char * p = text;

    while (*p != '\0')
    {
        while (*p == ' ') p++;
        if (*p == '\0') break;

        //Find the end of the word and its visible length:
        const char * end = p;
        int visible = 0;
        while (*end != '\0' && *end != ' ')
        {
            if (*end == '\x1b'){
                while (*end != '\0' && *end != 'm') end++;
                if (*end != '\0') end++;
                continue;
            }
            visible++;
            end++;
        }

        if (column > indent && column + 1 + visible > BOT_VIEW_TEXT_WIDTH){
            fprintf(out, "\n%*s", indent, "");
            column = indent;
        }
        else if (column > indent){
            fputc(' ', out);
            column++;
        }

        fwrite(p, 1, end - p, out);
        column += visible;
        p = end;
    }

    fputc('\n', out);
}

//Explains the bot's last decision in a sentence
static void DrawDecision(FILE * out, BotSnapshot * s){

    BotDecision * d = &s->decision;
    char * cell = (d->row >= 0 && d->col >= 0) ? CellName_alloc(d->row, d->col) : NULL;

    char text[512];
    int n = 0;
    #define APPEND(...) n += snprintf(text + n, sizeof(text) - n, __VA_ARGS__)

    if (d->explored){
        APPEND("At low risk, it left the plans on its stack to explore elsewhere: ");
    }

    switch (d->type)
    {
    case DECISION_NONE:
        APPEND("%s hasn't played yet.", s->botName);
        break;
    case DECISION_TORPEDO:
        if (d->col >= 0){
            char * column = alloc_IntegerToNumeral(d->col + coord_1_shift, GRIDSIZE, startingCoordinate_1, endingCoordinate_1);
            APPEND("fired its torpedo down column %s, the line with the highest total probability (%d).", column, d->probability);
            free(column);
        }
        else {
            char * row = alloc_IntegerToNumeral(d->row + coord_2_shift, GRIDSIZE, startingCoordinate_2, endingCoordinate_2);
            APPEND("fired its torpedo along row %s, the line with the highest total probability (%d).", row, d->probability);
            free(row);
        }
        break;
    case DECISION_ARTILLERY:
        APPEND("fired its artillery at the 2x2 area from %s, the area with the highest total probability (%d).", cell, d->probability);
        break;
    case DECISION_TASK:
        APPEND("followed the plan on top of its stack: %s, next to an earlier hit (probability %d).", cell, d->probability);
        break;
    case DECISION_CATEGORY:
        APPEND("it was the %s%s%s list's turn in the rotation. It picked a random region from that list (region %d) and fired at the region's best cell, %s (probability %d).",
            CategoryColor(d->category), CategoryName(d->category), RESET, d->region, cell, d->probability);
        break;
    case DECISION_HIGHEST_CATEGORY:
        APPEND("feeling threatened, it picked a random region from the highest list left (%s%s%s, region %d) and fired at its best cell, %s (probability %d).",
            CategoryColor(d->category), CategoryName(d->category), RESET, d->region, cell, d->probability);
        break;
    case DECISION_HOTTEST_CELL:
        APPEND("in danger, it searched every region for the single hottest cell: %s (probability %d).", cell, d->probability);
        break;
    case DECISION_FALLBACK:
        APPEND("no cell had any probability left, so it fired at a random cell it hadn't tried: %s.", cell);
        break;
    case DECISION_RANDOM:
        APPEND("fired at a random cell where a ship could still be: %s.", cell);
        break;
    }

    #undef APPEND

    fprintf(out, "  %sLast move:%s", BOLD, RESET);
    PrintWrapped(out, text, (int)strlen("  Last move:"), 4);

    free(cell);
}

static void DrawPlan(FILE * out, BotSnapshot * s){

    fprintf(out, "  %sPlan%s (its task stack, next first): ", BOLD, RESET);

    int shown = 0, total = 0;
    for (int t = 0; t < s->taskCount; t++)
    {
        int r = s->tasks[t][0], c = s->tasks[t][1];
        if (s->grid[r][c] != BOT_VIEW_UNKNOWN) continue; //Already shot: the bot will skip it

        total++;
        if (shown < 8){
            char * cell = CellName_alloc(r, c);
            fprintf(out, "%s%s%s ", BRIGHT_MAGENTA, cell, RESET);
            free(cell);
            shown++;
        }
    }

    if (total == 0) fprintf(out, "empty, it is searching for a ship.");
    else if (total > shown) fprintf(out, "(+%d more)", total - shown);
    fprintf(out, "\n");
}

static void DrawRegions(FILE * out, BotSnapshot * s){

    int counts[PROB_CATEGORYCOUNT] = {0};
    int empty = 0;

    for (int r = 0; r < PROB_REGION_COUNT; r++)
    {
        if (s->regionCategory[r] >= 0 && s->regionCategory[r] < PROB_CATEGORYCOUNT) counts[s->regionCategory[r]]++;
        else empty++;
    }

    fprintf(out, "  %sRegions%s (%dx%d cells each, kept in binomial heaps, [ ] = picked from last):\n", BOLD, RESET, PROB_REGION_WIDTH, PROB_REGION_HEIGHT);
    fprintf(out, "    %sH%s high %d   %sA%s average %d   %sL%s low %d   %s.%s nothing left %d\n",
        BRIGHT_RED, RESET, counts[HIGHPROB_CATEGORY], YELLOW, RESET, counts[AVGPROB_CATEGORY], CYAN, RESET, counts[LOWPROB_CATEGORY],
        BRIGHT_BLACK, RESET, empty);

    for (int rr = 0; rr < PROB_REGIONS_PER_COL; rr++)
    {
        fprintf(out, "      ");
        for (int rc = 0; rc < PROB_REGIONS_PER_ROW; rc++)
        {
            int region = rr * PROB_REGIONS_PER_ROW + rc;
            int category = s->regionCategory[region];
            const char * letter = (category == HIGHPROB_CATEGORY) ? "H" : (category == AVGPROB_CATEGORY) ? "A" : (category == LOWPROB_CATEGORY) ? "L" : ".";

            //The region the bot picked from last is highlighted
            int picked = (s->decision.type == DECISION_CATEGORY || s->decision.type == DECISION_HIGHEST_CATEGORY) && s->decision.region == region;
            fprintf(out, "%s%s%s%s%s%s", picked ? BOLD : "", CategoryColor(category), picked ? "[" : " ", letter, picked ? "]" : " ", RESET);
        }
        fprintf(out, "\n");
    }
}

static void DrawRisk(FILE * out, BotSnapshot * s){

    fprintf(out, "  %sRisk:%s %s%s%s%s  (updated at the start of each of %s's turns)\n", BOLD, RESET, BOLD, RiskColor(s->riskFactor),
        RiskName(s->riskFactor), RESET, s->botName);

    if (s->opponentShots < RISK_MIN_SHOTS){
        fprintf(out, "    %s has attacked %d time(s). The bot stays at normal risk until %d attacks.\n", s->opponentName, s->opponentShots, RISK_MIN_SHOTS);
    }

    int accuracy = (s->opponentShots > 0) ? (100 * s->opponentHits) / s->opponentShots : 0;
    int success = (s->recentTurns > 0) ? (100 * s->recentHits) / s->recentTurns : 0;
    int ahead = (s->botSunk == 0) ? (s->opponentSunk > 0) : ((double)s->opponentSunk / s->botSunk > RISK_SUNK_RATIO_THRESHOLD);

    const char * warn = BRIGHT_RED "  <- danger" RESET;

    fprintf(out, "    Accuracy:     %d/%d attacks hit (%d%%), danger from %d%%%s\n", s->opponentHits, s->opponentShots, accuracy,
        (int)(RISK_ACCURACY_THRESHOLD * 100), (s->opponentShots > 0 && accuracy >= (int)(RISK_ACCURACY_THRESHOLD * 100)) ? warn : "");
    fprintf(out, "    Recent form:  %d of the last %d attacks hit (%d%%), danger from %d%%%s\n", s->recentHits, s->recentTurns, success,
        (int)(RISK_SUCCESS_RATE_THRESHOLD * 100), (s->recentTurns > 0 && success >= (int)(RISK_SUCCESS_RATE_THRESHOLD * 100)) ? warn : "");
    fprintf(out, "    Ships sunk:   %s %d, %s %d, danger when %s is ahead%s\n", s->opponentName, s->opponentSunk, s->botName, s->botSunk,
        s->opponentName, ahead ? warn : "");
    fprintf(out, "    At this level");
    PrintWrapped(out, RiskMeaning(s->riskFactor), (int)strlen("    At this level"), 6);
}

/**
 * Draws the whole bot's view from a snapshot: its picture of the opponent's grid, its last decision, and for the smart bot its plans,
 * its regions and its risk.
 */
void DrawBotView(FILE * out, BotSnapshot * s){

    int smart = s->botIQ == SMART;

    fprintf(out, "\n  %s%s%s's brain%s   turn %d%s\n", BOLD, BRIGHT_BLUE, s->botName, RESET, s->turn, s->gameOver ? "   (game over)" : "");
    fprintf(out, "  %s\n\n", smart ? "Hard bot: hunts with a probability map of your grid" : "Easy bot: fires at random");

    fprintf(out, "  How %s sees %s's grid%s:\n\n", s->botName, s->opponentName,
        smart ? ", with the chance of a ship on each cell" : "");

    DrawGrid(out, s);
    fprintf(out, "\n");

    DrawDecision(out, s);

    if (smart){
        DrawPlan(out, s);
        fprintf(out, "\n");
        DrawRegions(out, s);
        fprintf(out, "\n");
        DrawRisk(out, s);
    }
    else {
        fprintf(out, "\n  ");
        PrintWrapped(out, "The easy bot doesn't use probabilities, plans or risk. It never fires at the same cell twice, and skips cells where no ship it hasn't sunk could fit.", 2, 2);
    }

    fflush(out);
}

#pragma endregion


#pragma region [Viewer Window]

/**
 * The viewer: started with "battleship --bot-view [file]", it redraws the bot's view every time the game writes a new snapshot, until
 * the game is over.
 */
int RunBotViewer(const char * path){

    BotSnapshot * snapshot = (BotSnapshot *)malloc(sizeof(BotSnapshot));
    long lastSequence = -1;

    ClearScreen();
    printf("\n  Waiting for the game to start (reading %s)...\n", path);
    fflush(stdout);

    while (1)
    {
        if (ReadBotSnapshot(path, snapshot) == 1 && snapshot->sequence != lastSequence){

            lastSequence = snapshot->sequence;

            ClearScreen();
            DrawBotView(stdout, snapshot);

            if (snapshot->gameOver){
                printf("\n  The game is over. Press <enter> to close this window.");
                fflush(stdout);
                getchar();
                break;
            }
        }

        SleepMs(250);
    }

    free(snapshot);
    return 0;
}

/**
 * Opens the bot's view in a new window: the same program, started with --bot-view.
 * On Windows the window opens by itself. Elsewhere there is no standard way to open a terminal window, so the player is told the
 * command to run in a second terminal.
 *
 * Returns 1 if the window was opened, 0 if the player has to open it.
 */
int OpenBotViewWindow(const char * programPath, const char * path){

#ifdef _WIN32
    //start "<window title>" "<program>" --bot-view "<file>"
    char * command = CreateString_alloc(7, "start \"Bot's brain\" \"", (char *)programPath, "\" ", BOT_VIEW_ARG, " \"", (char *)path, "\"");
    int res = system(command);
    free(command);

    return res == 0;
#else
    char * command = CreateString_alloc(5, (char *)programPath, " ", BOT_VIEW_ARG, " ", (char *)path);
    char * text = CreateString_alloc(2, "Open a second terminal in this folder and run: ", command);

    Println("");
    Println_Centered(text, strlen(text), BRIGHT_CYAN);

    free(text);
    free(command);

    return 0;
#endif
}

#pragma endregion
