/**
 * Tests for the input/coordinate helpers: coordslib.c, InputLib.c and ShortcutFuncs.c.
 * Based on the cases listed in docs/test-cases.docx.
 */
#include <stdlib.h>
#include "test.h"
#include "../include/coordslib.h"
#include "../include/InputLib.h"
#include "../include/ShortcutFuncs.h"

static void CheckCoords(char *input, int expectedRow, int expectedCol){
    char *msg = NULL;
    int *ij = alloc_ArrayCoordsFromUserCoords(input, &msg);
    CHECK(ij != NULL);
    if (ij != NULL){
        CHECK_EQ_INT(ij[0], expectedRow);
        CHECK_EQ_INT(ij[1], expectedCol);
        free(ij);
    }
    free(msg);
}

static void CheckInvalidCoords(char *input){
    char *msg = NULL;
    int *ij = alloc_ArrayCoordsFromUserCoords(input, &msg);
    if (ij != NULL) fprintf(stderr, "    '%s' was accepted as (%d,%d)\n", input, ij[0], ij[1]);
    CHECK(ij == NULL);
    CHECK(msg != NULL); //The player must be told why
    free(ij);
    free(msg);
}

void test_coords_valid(void){
    CheckCoords("A1", 0, 0);
    CheckCoords("B4", 3, 1);
    CheckCoords("b4", 3, 1);  //lowercase works too
    CheckCoords("E04", 3, 4); //zero padded rows, as shown on the grid
    CheckCoords("J10", GRIDSIZE - 1, GRIDSIZE - 1);
}

void test_coords_invalid(void){
    CheckInvalidCoords("");
    CheckInvalidCoords("A");     //no row
    CheckInvalidCoords("1");     //no column
    CheckInvalidCoords("A0");    //rows start at 1
    CheckInvalidCoords("A11");   //row past the grid
    CheckInvalidCoords("K1");    //column past the grid
    CheckInvalidCoords("Z9");
    CheckInvalidCoords("A!");    //out-of-range character
    CheckInvalidCoords("12A");   //wrong order
    CheckInvalidCoords("A1B");
}

void test_coords_index_range(void){
    CHECK(IndexWithinRange(0));
    CHECK(IndexWithinRange(GRIDSIZE - 1));
    CHECK(!IndexWithinRange(-1));
    CHECK(!IndexWithinRange(GRIDSIZE));
}

void test_coord_to_index(void){
    CHECK_EQ_INT(CoordToIndex("A", 0, 1, 'A', 'Z', 0), 0);
    CHECK_EQ_INT(CoordToIndex("Z", 0, 1, 'A', 'Z', 0), 25);
    CHECK_EQ_INT(CoordToIndex("BA", 0, 2, 'A', 'Z', 0), 26);
    CHECK_EQ_INT(CoordToIndex("FBC", 0, 3, 'A', 'Z', 0), 5 * 26 * 26 + 1 * 26 + 2); //example from the project overview
    CHECK_EQ_INT(CoordToIndex("10", 0, 2, '0', '9', 1), 9); //the shift turns the displayed 1..10 into 0..9
    CHECK_EQ_INT(CoordToIndex("A!", 0, 2, 'A', 'Z', 0), -1);
    CHECK_EQ_INT(CoordToIndex("A", 0, 0, 'A', 'Z', 0), -1); //empty range
}

void test_coords_every_cell_round_trips(void){
    //Converting indices to coordinates and back must give the same cell. The bot relies on this to fire.
    for (int i = 0; i < GRIDSIZE; i++){
        for (int j = 0; j < GRIDSIZE; j++){
            char *coords = alloc_GetCoordsFromIndices(i, j, GRIDSIZE, startingCoordinate_1, startingCoordinate_2,
                endingCoordinate_1, endingCoordinate_2, coord_1_shift, coord_2_shift);
            int *ij = alloc_ArrayCoordsFromUserCoords(coords, NULL);
            CHECK(ij != NULL);
            if (ij != NULL){
                CHECK_EQ_INT(ij[0], i);
                CHECK_EQ_INT(ij[1], j);
            }
            free(ij);
            free(coords);
        }
    }
}

void test_grid_area(void){
    char *msg = NULL;
    int *area = alloc_GridAreaFromInput("A1", "h", 2, 0, &msg);
    CHECK(area != NULL);
    if (area){ CHECK_EQ_INT(area[0], 0); CHECK_EQ_INT(area[1], 0); CHECK_EQ_INT(area[2], 0); CHECK_EQ_INT(area[3], 2); }
    free(area);

    area = alloc_GridAreaFromInput("A1", "v", 2, 0, &msg);
    CHECK(area != NULL);
    if (area){ CHECK_EQ_INT(area[0], 0); CHECK_EQ_INT(area[1], 2); CHECK_EQ_INT(area[2], 0); CHECK_EQ_INT(area[3], 0); }
    free(area);

    area = alloc_GridAreaFromInput("A!", "h", 2, 0, &msg);
    CHECK(area == NULL);
    free(msg);
}

void test_integer_to_numeral(void){
    char *s = alloc_IntegerToNumeral(0, GRIDSIZE, 'A', 'Z');
    CHECK_EQ_STR(s, "A"); free(s);

    s = alloc_IntegerToNumeral(9, GRIDSIZE, 'A', 'Z');
    CHECK_EQ_STR(s, "J"); free(s);

    //Padded to the width of the largest number (1000 needs 3 letters), 27 = 1*26 + 1 = "BB"
    s = alloc_IntegerToNumeral(27, 1000, 'A', 'Z');
    CHECK_EQ_STR(s, "ABB"); free(s);

    s = alloc_IntegerToNumeral(675, 1000, 'A', 'Z'); //25*26 + 25
    CHECK_EQ_STR(s, "AZZ"); free(s);

    s = alloc_IntegerToNumeral(701, 1000, 'A', 'Z'); //1*26^2 + 0*26 + 25
    CHECK_EQ_STR(s, "BAZ"); free(s);

    s = alloc_IntegerToNumeral(7, 10, '0', '9');
    CHECK_EQ_STR(s, "07"); free(s);
}

void test_coords_from_indices(void){
    char *s = alloc_GetCoordsFromIndices(3, 4, GRIDSIZE, startingCoordinate_1, startingCoordinate_2, endingCoordinate_1, endingCoordinate_2, coord_1_shift, coord_2_shift);
    CHECK_EQ_STR(s, "E04");
    free(s);
}

void test_string_to_enum_index(void){
    char *arr[] = {"start", "quit", "next"};
    CHECK_EQ_INT(StringToEnumIndex("quit", arr, 3), 1);
    CHECK_EQ_INT(StringToEnumIndex("QUIT", arr, 3), 1); //case-insensitive
    CHECK_EQ_INT(StringToEnumIndex("stop", arr, 3), -1);
}

void test_next_word(void){
    char buffer[] = "fire artillery torpedo";
    char *ptr = buffer;

    char *w = next(&ptr);
    CHECK_EQ_STR(w, "fire");
    CHECK_EQ_STR(ptr, "artillery torpedo");
    free(w);

    char buffer2[] = "  smoke   screen deployed";
    ptr = buffer2;
    w = next(&ptr);
    CHECK_EQ_STR(w, "smoke");
    CHECK_EQ_STR(ptr, "screen deployed");
    free(w);

    char empty[] = "";
    ptr = empty;
    w = next(&ptr);
    CHECK(w != NULL);
    CHECK_EQ_STR(w, "");
    free(w);
}

void test_next_int(void){
    char buffer[] = "123 456 789";
    char *ptr = buffer;
    CHECK_EQ_INT(nextInt(&ptr), 123);
    CHECK_EQ_STR(ptr, "456 789");
    CHECK_EQ_INT(nextInt(&ptr), 456);
    CHECK_EQ_INT(nextInt(&ptr), 789);
}

void test_parse_int(void){
    CHECK_EQ_INT(ParseInt("0"), 0);
    CHECK_EQ_INT(ParseInt("7"), 7);
    CHECK_EQ_INT(ParseInt("42"), 42);
    CHECK_EQ_INT(ParseInt("1009"), 1009);
    CHECK_EQ_INT(ParseInt("12a"), -1);
    CHECK_EQ_INT(ParseInt(""), -1);
}

void test_create_string(void){
    char *s = CreateString_alloc(3, "Well done ", "Ayoub", "!");
    CHECK_EQ_STR(s, "Well done Ayoub!");
    free(s);

    s = CreateString_alloc(1, "");
    CHECK_EQ_STR(s, "");
    free(s);
}
