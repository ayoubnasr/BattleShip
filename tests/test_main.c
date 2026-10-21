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
void test_smart_bot_wins_at_every_risk_level(void);
void test_smart_bot_fleet_in_corner(void);
void test_dumb_bot_wins_and_never_repeats(void);
void test_dumb_bot_skips_cells_that_cannot_hold_ships(void);
void test_bot_ship_placement(void);
void test_lowest_probability_placement(void);
void test_smart_bot_uses_artillery_after_sinking(void);
void test_smart_bot_uses_torpedo_after_third_ship(void);
void test_smart_bot_full_games_with_weapons(void);
void test_risk_factor(void);

//test_probabilities.c
void test_initial_probability_grid(void);
void test_initial_region_system(void);
void test_hash_region(void);
void test_probability_categories(void);
void test_miss_lowers_and_hit_raises_neighbours(void);
void test_sinking_recalculates_and_clears_tasks(void);
void test_sinking_keeps_tasks_for_other_damaged_ship(void);
void test_cell_can_hold_ship(void);
void test_fitting_placements_match_formula_on_empty_grid(void);
void test_probability_learns_from_empty_space(void);
void test_thresholds_scale_with_remaining_ships(void);
void test_late_game_still_has_high_regions(void);

//test_botview.c
void test_decision_category_rotation(void);
void test_decision_task(void);
void test_decision_risk_levels(void);
void test_decision_low_risk_exploring(void);
void test_decision_weapons_and_easy_bot(void);
void test_snapshot_round_trip(void);
void test_snapshot_rejects_bad_files(void);
void test_draw_bot_view(void);

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

    printf("\n-- Probability grid and regions --\n");
    RUN_TEST(test_initial_probability_grid);
    RUN_TEST(test_initial_region_system);
    RUN_TEST(test_hash_region);
    RUN_TEST(test_probability_categories);
    RUN_TEST(test_miss_lowers_and_hit_raises_neighbours);
    RUN_TEST(test_sinking_recalculates_and_clears_tasks);
    RUN_TEST(test_sinking_keeps_tasks_for_other_damaged_ship);
    RUN_TEST(test_cell_can_hold_ship);
    RUN_TEST(test_fitting_placements_match_formula_on_empty_grid);
    RUN_TEST(test_probability_learns_from_empty_space);
    RUN_TEST(test_thresholds_scale_with_remaining_ships);
    RUN_TEST(test_late_game_still_has_high_regions);

    printf("\n-- Bot --\n");
    RUN_TEST(test_bot_task_stack_order);
    RUN_TEST(test_bot_task_creation);
    RUN_TEST(test_smart_bot_wins_at_every_risk_level);
    RUN_TEST(test_smart_bot_fleet_in_corner);
    RUN_TEST(test_dumb_bot_wins_and_never_repeats);
    RUN_TEST(test_dumb_bot_skips_cells_that_cannot_hold_ships);
    RUN_TEST(test_bot_ship_placement);
    RUN_TEST(test_lowest_probability_placement);
    RUN_TEST(test_smart_bot_uses_artillery_after_sinking);
    RUN_TEST(test_smart_bot_uses_torpedo_after_third_ship);
    RUN_TEST(test_smart_bot_full_games_with_weapons);
    RUN_TEST(test_risk_factor);

    printf("\n-- Bot view --\n");
    RUN_TEST(test_decision_category_rotation);
    RUN_TEST(test_decision_task);
    RUN_TEST(test_decision_risk_levels);
    RUN_TEST(test_decision_low_risk_exploring);
    RUN_TEST(test_decision_weapons_and_easy_bot);
    RUN_TEST(test_snapshot_round_trip);
    RUN_TEST(test_snapshot_rejects_bad_files);
    RUN_TEST(test_draw_bot_view);

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
