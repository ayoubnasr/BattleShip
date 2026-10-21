#include <stdlib.h>
#include "fixtures.h"
#include "../include/ShipPlacement.h"

Player *NewTestPlayer(char *name, bool isBot, BotIQ botIQ){
    Player *p = NULL;
    alloc_InitializePlayer(&p, name, isBot, botIQ);
    return p;
}

int PlaceShip(Player *p, char shipChar, char *coords, char *orientation){
    int length = 0;
    switch (shipChar){
        case CARRIER_C: length = CARRIER_LENGTH; break;
        case BATTLESHIP_C: length = BATTLESHIP_LENGTH; break;
        case DESTROYER_C: length = DESTROYER_LENGTH; break;
        case SUBMARINE_C: length = SUBMARINE_LENGTH; break;
    }
    int size[2] = {length, 0};
    char *msg = NULL;
    int res = PlaceShipOnGridHelper(p, shipChar, p->grid, coords, orientation, size, &msg);
    free(msg);
    return res;
}

Player *NewTestPlayerWithFleet(char *name){
    Player *p = NewTestPlayer(name, false, DUMB);
    PlaceShip(p, CARRIER_C, "A1", "h");
    PlaceShip(p, BATTLESHIP_C, "A2", "v");
    PlaceShip(p, DESTROYER_C, "C3", "h");
    PlaceShip(p, SUBMARINE_C, "G5", "v");
    return p;
}

void SinkShip(Player *p, ShipBounds *ship){
    for (int i = ship->startRow; i <= ship->endRow; i++){
        for (int j = ship->startCol; j <= ship->endCol; j++){
            p->grid[i][j] = HIT;
        }
    }
    countSunkShips(p);
}
