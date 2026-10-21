#ifndef BOTVIEW
#define BOTVIEW

/**
 * The bot's view: a second window that shows what the bot is thinking while you play against it.
 *
 * After every turn the game writes a snapshot of the bot's mind to a file (WriteBotSnapshot()). The same program started with the
 * --bot-view argument reads that file and redraws it whenever it changes (RunBotViewer()). The two windows never share memory: the file
 * is all they have in common, so each one stays a simple loop that only prints.
 *
 * The snapshot only holds what the bot knows about the opponent's grid (hits, misses, sunk ships), never where the ships are.
 */

#include <stdio.h>
#include "Player.h"

#define BOT_VIEW_FILE "battleship-bot-view.txt"
#define BOT_VIEW_ARG "--bot-view"

#define BOT_VIEW_MAX_TASKS 64  //At most this many planned cells are written to the snapshot
#define BOT_VIEW_NAMELENGTH 32
#define BOT_VIEW_TEXT_WIDTH 90  //Long sentences in the view wrap at this width

//Cells of the opponent's grid in a snapshot:
#define BOT_VIEW_UNKNOWN '~'
#define BOT_VIEW_SUNK '#'

typedef struct BotSnapshot{

    long sequence;  //Grows with every snapshot written, so the viewer knows when to redraw
    int turn;
    int gameOver;

    char botName[BOT_VIEW_NAMELENGTH];
    char opponentName[BOT_VIEW_NAMELENGTH];
    int botIQ;

    //Risk (see the RISK_ macros in Player.h)
    int riskFactor;
    int opponentShots, opponentHits;
    int recentHits, recentTurns;
    int opponentSunk;   //The bot's ships the opponent sank
    int botSunk;        //The opponent's ships the bot sank

    BotDecision decision;

    char grid[GRIDSIZE][GRIDSIZE];          //What the bot knows: BOT_VIEW_UNKNOWN, HIT, MISS or BOT_VIEW_SUNK
    int probabilities[GRIDSIZE][GRIDSIZE];  //Smart bot: probability of each unknown cell. Easy bot: 1 if a ship could still be there, else 0.
    int regionCategory[PROB_REGION_COUNT];  //Category of every region (NO_CATEGORY when it has no cell left)

    int taskCount;                          //Cells planned on the bot's stack (next first)
    int tasks[BOT_VIEW_MAX_TASKS][2];

} BotSnapshot;

int WriteBotSnapshot(const char * path, Player * bot, Player * opponent, int turn, int gameOver);

int ReadBotSnapshot(const char * path, BotSnapshot * snapshot);

void DrawBotView(FILE * out, BotSnapshot * snapshot);

int RunBotViewer(const char * path);

int OpenBotViewWindow(const char * programPath, const char * path);

#endif
