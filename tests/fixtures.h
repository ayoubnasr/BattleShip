#ifndef FIXTURES_H
#define FIXTURES_H

/**
 * Helpers that build players and fleets for the tests.
 */
#include "../include/Player.h"

//A new player with an empty grid (initializes the probability grid and region heaps like the game does)
Player *NewTestPlayer(char *name, bool isBot, BotIQ botIQ);

//Places one ship by its character using the same function the game uses. Returns its result (1 or -1).
int PlaceShip(Player *p, char shipChar, char *coords, char *orientation);

/**
 * A player with this fixed fleet (rows are numbered from 1, like on screen):
 *      Carrier    A1-E1 (horizontal)
 *      Battleship A2-A5 (vertical)
 *      Destroyer  C3-E3 (horizontal)
 *      Submarine  G5-G6 (vertical)
 */
Player *NewTestPlayerWithFleet(char *name);

//Marks every cell of the ship as HIT and updates the sunk flags
void SinkShip(Player *p, ShipBounds *ship);

#endif
