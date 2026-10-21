/**
 * Test runner. Build and run everything with:  make test
 */
#include <stdlib.h>
#include <time.h>
#include "test.h"

int tests_run = 0;
int tests_failed = 0;
int checks_failed = 0;

//test_utils.c
void test_coords_valid(void);
void test_coords_invalid(void);
void test_coords_index_range(void);
void test_coord_to_index(void);
void test_coords_every_cell_round_trips(void);
void test_grid_area(void);
void test_integer_to_numeral(void);
void test_coords_from_indices(void);
void test_string_to_enum_index(void);
void test_next_word(void);
void test_next_int(void);
void test_parse_int(void);
void test_create_string(void);

//test_structures.c
void test_list_add_remove(void);
void test_list_remove_node_anywhere(void);
void test_heap_find_and_delete_in_order(void);
void test_heap_with_duplicates_and_every_size(void);
void test_heap_construct_from_array_and_union(void);

//test_gameplay.c
void test_place_ship_horizontal_and_vertical(void);
void test_place_ship_rejects_bad_input(void);
void test_fire_hit_miss_and_repeat(void);
void test_radar(void);
void test_smoke_hides_ships_from_radar(void);
void test_artillery(void);
void test_torpedo(void);
void test_count_sunk_ships(void);

//test_bot.c
void test_bot_task_stack_order(void);
void test_bot_task_creation(void);

int main(void){
    srand((unsigned)time(NULL));

    printf("\n-- Coordinates and input --\n");
    RUN_TEST(test_coords_valid);
    RUN_TEST(test_coords_invalid);
    RUN_TEST(test_coords_index_range);
    RUN_TEST(test_coord_to_index);
    RUN_TEST(test_coords_every_cell_round_trips);
    RUN_TEST(test_grid_area);
    RUN_TEST(test_integer_to_numeral);
    RUN_TEST(test_coords_from_indices);
    RUN_TEST(test_string_to_enum_index);
    RUN_TEST(test_next_word);
    RUN_TEST(test_next_int);
    RUN_TEST(test_parse_int);
    RUN_TEST(test_create_string);

    printf("\n-- Data structures --\n");
    RUN_TEST(test_list_add_remove);
    RUN_TEST(test_list_remove_node_anywhere);
    RUN_TEST(test_heap_find_and_delete_in_order);
    RUN_TEST(test_heap_with_duplicates_and_every_size);
    RUN_TEST(test_heap_construct_from_array_and_union);

    printf("\n-- Ship placement and attacks --\n");
    RUN_TEST(test_place_ship_horizontal_and_vertical);
    RUN_TEST(test_place_ship_rejects_bad_input);
    RUN_TEST(test_fire_hit_miss_and_repeat);
    RUN_TEST(test_radar);
    RUN_TEST(test_smoke_hides_ships_from_radar);
    RUN_TEST(test_artillery);
    RUN_TEST(test_torpedo);
    RUN_TEST(test_count_sunk_ships);

    printf("\n-- Bot --\n");
    RUN_TEST(test_bot_task_stack_order);
    RUN_TEST(test_bot_task_creation);

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
