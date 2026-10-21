/**
 * Benchmark: how many turns the hard bot needs to sink a whole fleet.
 *
 * Plays many games of the smart bot against randomly placed fleets (the same random placement the easy bot uses) and prints the
 * average number of turns, its standard error and the worst game. The bot may use its artillery and torpedo like in a real game.
 * The random generator is seeded with a fixed number, so every run on the same code gives the same result.
 *
 * Usage:   make benchmark                     (2000 games at normal risk)
 *          bin/benchmark <games> <risk>       (risk: 0 low, 1 normal, 2 high, 3 extremely high)
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/Player.h"
#include "../include/Bot.h"

int main(int argc, char *argv[]){

    int games = (argc >= 2) ? atoi(argv[1]) : 2000;
    int risk = (argc >= 3) ? atoi(argv[2]) : NORMAL_RISK;

    long total = 0, totalSquares = 0;
    int worst = 0;

    srand(12345);

    for (int game = 0; game < games; game++)
    {
        Player *bot = NULL, *target = NULL;
        alloc_InitializePlayer(&bot, "bot", true, SMART);
        alloc_InitializePlayer(&target, "target", false, DUMB);
        PlaceBotShips(target);

        //The risk stays fixed: there is no opponent attacking the bot here
        bot->riskFactor = risk;

        int turns = 0, sunk = 0;
        while (countSunkShips(target) < SHIPCOUNT && turns < GRIDSIZE * GRIDSIZE)
        {
            BotSmartAttack(bot, target);
            turns++;

            //Like the driver: after a turn that sank a ship, the bot may use its artillery/torpedo
            int nowSunk = countSunkShips(target);
            bot->prevSunk = nowSunk > sunk;
            sunk = nowSunk;
        }

        total += turns;
        totalSquares += (long)turns * turns;
        if (turns > worst) worst = turns;
    }

    double average = (double)total / games;
    double deviation = sqrt((double)totalSquares / games - average * average);

    printf("%d games at risk %d: %.1f turns on average (standard error %.2f), worst game %d turns\n",
        games, risk, average, deviation / sqrt(games), worst);

    return 0;
}
