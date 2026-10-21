/**
 * Tests for ship placement (ShipPlacement.c), the attacks (Attacks.c) and the sinking logic in Player.c.
 * Based on the cases listed in docs/test-cases.docx.
 */
#include <stdlib.h>
#include "test.h"
#include "fixtures.h"
#include "../include/Attacks.h"
#include "../include/ShipPlacement.h"

void test_place_ship_horizontal_and_vertical(void){
    Player *p = NewTestPlayer("placer", false, DUMB);

    CHECK_EQ_INT(PlaceShip(p, CARRIER_C, "A1", "h"), 1);
    for (int j = 0; j < CARRIER_LENGTH; j++) CHECK_EQ_INT(p->grid[0][j], CARRIER_C);
    CHECK_EQ_INT(p->grid[0][CARRIER_LENGTH], WATER_C);
    CHECK_EQ_INT(p->carrierBounds.startRow, 0);
    CHECK_EQ_INT(p->carrierBounds.endRow, 0);
    CHECK_EQ_INT(p->carrierBounds.startCol, 0);
    CHECK_EQ_INT(p->carrierBounds.endCol, CARRIER_LENGTH - 1);

    CHECK_EQ_INT(PlaceShip(p, SUBMARINE_C, "J9", "V"), 1); //uppercase orientation, last two rows of the last column
    CHECK_EQ_INT(p->grid[8][9], SUBMARINE_C);
    CHECK_EQ_INT(p->grid[9][9], SUBMARINE_C);
    CHECK_EQ_INT(p->submarineBounds.startRow, 8);
    CHECK_EQ_INT(p->submarineBounds.endRow, 9);
}

void test_place_ship_rejects_bad_input(void){
    Player *p = NewTestPlayer("placer", false, DUMB);

    CHECK_EQ_INT(PlaceShip(p, CARRIER_C, "A1", "h"), 1);
    CHECK_EQ_INT(PlaceShip(p, SUBMARINE_C, "B1", "v"), -1);  //overlaps the carrier
    CHECK_EQ_INT(PlaceShip(p, BATTLESHIP_C, "H1", "h"), -1); //H..K goes past the right edge
    CHECK_EQ_INT(PlaceShip(p, BATTLESHIP_C, "A8", "v"), -1); //rows 8..11 go past the bottom edge
    CHECK_EQ_INT(PlaceShip(p, BATTLESHIP_C, "Q1", "h"), -1); //invalid coordinate
    CHECK_EQ_INT(PlaceShip(p, BATTLESHIP_C, "C3", "x"), -1); //invalid orientation
    CHECK_EQ_INT(p->grid[2][2], WATER_C);                     //nothing placed by the failed attempts

    CHECK_EQ_INT(PlaceShip(p, BATTLESHIP_C, "A2", "vertical"), 1);
}

void test_fire_hit_miss_and_repeat(void){
    Player *p = NewTestPlayerWithFleet("target");
    char *msg = NULL;

    //Carrier is on row 1 (A1..E1)
    CHECK_EQ_INT(Fire("B1", p->grid, 0, &msg), 1);
    CHECK_EQ_STR(msg, "Hit!");
    CHECK_EQ_INT(p->grid[0][1], HIT);
    free(msg); msg = NULL;

    CHECK_EQ_INT(Fire("B1", p->grid, 0, &msg), -1); //same cell again: rejected, player keeps the turn
    CHECK_EQ_STR(msg, "This coordinate has already been hit.");
    free(msg); msg = NULL;

    CHECK_EQ_INT(Fire("J1", p->grid, 0, &msg), 1);  //easy mode records the miss
    CHECK_EQ_STR(msg, "Miss");
    CHECK_EQ_INT(p->grid[0][9], MISS);
    free(msg); msg = NULL;

    CHECK_EQ_INT(Fire("I1", p->grid, 1, &msg), 1);  //hard mode does not show misses
    CHECK_EQ_INT(p->grid[0][8], WATER_C);
    free(msg); msg = NULL;

    CHECK_EQ_INT(Fire("Z9", p->grid, 0, &msg), -1);
    CHECK(msg != NULL);
    free(msg); msg = NULL;

    CHECK_EQ_INT(Fire("12A", p->grid, 0, &msg), -1);
    free(msg);
}

void test_radar(void){
    Player *p = NewTestPlayerWithFleet("target");
    char *msg = NULL;

    CHECK_EQ_INT(performRadarSweep("A1", p, &msg), 1);
    CHECK_EQ_STR(msg, "Enemy ships found");
    CHECK_EQ_INT(p->sweepsLeft, 2);
    free(msg); msg = NULL;

    CHECK_EQ_INT(performRadarSweep("I9", p, &msg), 1);
    CHECK_EQ_STR(msg, "No enemy ships found");
    free(msg); msg = NULL;

    CHECK_EQ_INT(performRadarSweep("J10", p, &msg), 1); //corner: the 2x2 area is clipped to the grid
    free(msg); msg = NULL;
    CHECK_EQ_INT(p->sweepsLeft, 0);

    CHECK_EQ_INT(performRadarSweep("A1", p, &msg), 1); //no sweeps left: the turn is lost
    CHECK_EQ_STR(msg, "No more radar sweeps allowed! You lose your turn.");
    free(msg); msg = NULL;

    CHECK_EQ_INT(performRadarSweep("X10", p, &msg), -1);
    free(msg);
}

void test_smoke_hides_ships_from_radar(void){
    Player *attacker = NewTestPlayerWithFleet("attacker");
    Player *defender = NewTestPlayerWithFleet("defender");
    char *msg = NULL;

    //No smoke screens before sinking a ship:
    CHECK_EQ_INT(applySmokeScreen("A1", defender, attacker, &msg), -1);
    free(msg); msg = NULL;

    //Defender sinks the attacker's submarine, earning one smoke screen:
    SinkShip(attacker, &attacker->submarineBounds);
    CHECK_EQ_INT(applySmokeScreen("A1", defender, attacker, &msg), 1);
    CHECK_EQ_STR(msg, "Smoke screen applied.");
    CHECK(defender->smokeGrid[0][0] && defender->smokeGrid[1][1]);
    free(msg); msg = NULL;

    CHECK_EQ_INT(applySmokeScreen("C3", defender, attacker, &msg), -1); //only one ship sunk
    free(msg); msg = NULL;

    CHECK_EQ_INT(performRadarSweep("A1", defender, &msg), 1);
    CHECK_EQ_STR(msg, "No enemy ships found");
    free(msg);
}

void test_artillery(void){
    Player *attacker = NewTestPlayerWithFleet("attacker");
    Player *defender = NewTestPlayerWithFleet("defender");
    char *msg = NULL;

    attacker->prevSunk = 0;
    CHECK_EQ_INT(Artillery("A1", attacker, defender, 0, &msg), -1);
    CHECK_EQ_STR(msg, "Artillery can only be used in the round right after sinking an opponent's ship!");
    free(msg); msg = NULL;

    attacker->prevSunk = 1;
    CHECK_EQ_INT(Artillery("A1", attacker, defender, 0, &msg), 1); //A1,B1 carrier; A2 battleship; B2 water
    CHECK_EQ_STR(msg, "Hit!");
    CHECK_EQ_INT(defender->grid[0][0], HIT);
    CHECK_EQ_INT(defender->grid[0][1], HIT);
    CHECK_EQ_INT(defender->grid[1][0], HIT);
    CHECK_EQ_INT(defender->grid[1][1], MISS);
    free(msg); msg = NULL;

    CHECK_EQ_INT(Artillery("I9", attacker, defender, 0, &msg), 1);
    CHECK_EQ_STR(msg, "Miss!");
    CHECK_EQ_INT(defender->grid[8][8], MISS);
    free(msg); msg = NULL;

    //Bottom-right corner: must not write outside the grid
    CHECK_EQ_INT(Artillery("J10", attacker, defender, 0, &msg), 1);
    CHECK_EQ_INT(defender->grid[9][9], MISS);
    free(msg); msg = NULL;

    CHECK_EQ_INT(Artillery("K1", attacker, defender, 0, &msg), -1);
    free(msg);
}

void test_torpedo(void){
    Player *attacker = NewTestPlayerWithFleet("attacker");
    Player *defender = NewTestPlayerWithFleet("defender");
    char *msg = NULL;

    attacker->prevSunk = 1;
    CHECK_EQ_INT(Torpedo("A", attacker, defender, 0, &msg), -1); //fewer than 3 ships sunk
    CHECK_EQ_STR(msg, "Torpedo can only be used after sinking the opponent's third ship!");
    free(msg); msg = NULL;

    SinkShip(defender, &defender->submarineBounds);
    SinkShip(defender, &defender->destroyerBounds);
    SinkShip(defender, &defender->battleshipBounds);

    //Column A: carrier at A1, the rest of the column is sunk ship cells (stay HIT) or water
    CHECK_EQ_INT(Torpedo("a", attacker, defender, 0, &msg), 1);
    CHECK_EQ_STR(msg, "Hit!");
    CHECK_EQ_INT(defender->grid[0][0], HIT);
    CHECK_EQ_INT(defender->grid[1][0], HIT); //already-hit battleship cell is not turned into a miss
    CHECK_EQ_INT(defender->grid[9][0], MISS);
    free(msg); msg = NULL;

    //Row 10
    CHECK_EQ_INT(Torpedo("10", attacker, defender, 0, &msg), 1);
    CHECK_EQ_STR(msg, "Miss.");
    CHECK_EQ_INT(defender->grid[9][5], MISS);
    free(msg); msg = NULL;

    //Column letter past the grid must be rejected, not read out of bounds
    CHECK_EQ_INT(Torpedo("Z", attacker, defender, 0, &msg), -1);
    free(msg); msg = NULL;

    CHECK_EQ_INT(Torpedo("22A", attacker, defender, 0, &msg), -1);
    free(msg);
}

void test_count_sunk_ships(void){
    Player *p = NewTestPlayerWithFleet("target");

    CHECK_EQ_INT(countSunkShips(p), 0);

    p->grid[0][0] = HIT; //one cell of the carrier
    CHECK_EQ_INT(countSunkShips(p), 0);
    CHECK(!p->carrierBounds.IsSunk);

    SinkShip(p, &p->submarineBounds);
    CHECK_EQ_INT(countSunkShips(p), 1);
    CHECK(p->submarineBounds.IsSunk);

    SinkShip(p, &p->carrierBounds);
    SinkShip(p, &p->battleshipBounds);
    SinkShip(p, &p->destroyerBounds);
    CHECK_EQ_INT(countSunkShips(p), SHIPCOUNT);
}
