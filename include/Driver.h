

#ifndef DRIVER

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "defs.h"
#include "Player.h"
#include "Bot.h"
#include "coordslib.h"
#include "Attacks.h"
#include "InputLib.h"
#include "Statements.h"
#include "ShortcutFuncs.h"
#include "ShipPlacement.h"
#include "UITools.h"

#define DRIVER

const int PlayerCount = 2;
int currPlayer;
GameMode gameMode;

void RefreshScreen();

void StartGame();

void Quit();

void SetPlayerName(struct Player p);

void SetupPlayerGrid(struct Player p);

void RunTurn(int opponentID);

void Attack(int attackID, int opponentID);

int SelectGameMode(char * input);

bool CheckTurn();

Player* alloc_InitializePlayer();

#endif
