#ifndef ATTACKS


#define ATTACKS

int Fire(char *coords, char** attacked, int easyMode, char ** outputMsg);

int performRadarSweep(char *inputC, Player* player, char ** outputMsg);

int applySmokeScreen(char *coords, Player* player, Player * opponent, char ** outputMsg);

int Artillery(char *inputC, Player* player, Player* opp, int difficulty, char ** outputMsg);

int Torpedo(char *inputC,Player* player,Player* opp, int difficulty, char ** outputMsg);
#endif