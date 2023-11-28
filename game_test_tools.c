/**
 * @file game_test_aux.c
 * @brief Game Tests Aux.
 * @copyright University of Bordeaux. All rights reserved, 2021.
 *
 **/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "game.h"
#include "game_aux.h"
#include "game_examples.h"
#include "game_ext.h"
#include "game_test.h"
#include "game_tools.h"

/* ************************************************************************** */
/*                                TOOLS TESTS                                   */
/* ************************************************************************** */

int test_load(void)
{
  bool test0 = true;
  bool test1 = true;
  game g = game_default();
  game_play_move(g, 0, 0, S_LIGHTBULB);
  game_play_move(g, 1, 1, S_MARK);
  game_save(g, "loadTest.txt");
  game gLoad = game_load("loadTest.txt");
  if (!game_equal(g, gLoad)) test0 = false;

  game gWrap = game_new_ext(5, 3, ext_5x3w_squares, true);
  game_play_move(gWrap, 0, 0, S_LIGHTBULB);
  game_play_move(gWrap, 1, 1, S_LIGHTBULB);
  game_play_move(gWrap, 2, 0, S_MARK);
  game_save(gWrap, "loadTestWrap.txt");
  game gWrapLoad = game_load("loadTestWrap.txt");
  if (!game_equal(gWrap, gWrapLoad)) test0 = false;

  game gLoadInvalidFile = game_load("DoesntExist.txt");
  if (gLoadInvalidFile != NULL) {
    test1 = false;
    game_delete(gLoadInvalidFile);
  }
  gLoadInvalidFile = game_load("badSave.txt");
  if (gLoadInvalidFile != NULL) {
    test1 = false;
    game_delete(gLoadInvalidFile);
  }

  game_delete(g);
  game_delete(gLoad);
  game_delete(gWrap);
  game_delete(gWrapLoad);
  if (test0 && test1) return EXIT_SUCCESS;
  return EXIT_FAILURE;
}

/* ************************************************************************** */
int test_save(void)
{
  game g = game_default();
  game_play_move(g, 0, 0, S_LIGHTBULB);
  game_save(g, "savetest.txt");
  if (access("savetest.txt", F_OK) != 0) {
    return EXIT_FAILURE;
  }
  game g2 = game_load("savetest.txt");
  if (!game_equal(g, g2)) {
    return EXIT_FAILURE;
  }

  game gWrap = game_new_ext(5, 3, ext_5x3w_squares, true);
  game_play_move(gWrap, 0, 0, S_LIGHTBULB);
  game_play_move(gWrap, 1, 1, S_LIGHTBULB);
  game_play_move(gWrap, 2, 0, S_MARK);
  game_save(gWrap, "saveTestWrap.txt");
  if (access("saveTestWrap.txt", F_OK) != 0) {
    return EXIT_FAILURE;
  }
  game gWrap2 = game_load("saveTestWrap.txt");
  if (!game_equal(gWrap, gWrap2)) {
    return EXIT_FAILURE;
  }

  game_delete(g);
  game_delete(g2);
  game_delete(gWrap);
  game_delete(gWrap2);
  return EXIT_SUCCESS;
}
/* ************************************************************************** */
int test_game_solve(void)
{
  game g = game_default();
  bool solved = game_solve(g);
  if (!game_is_over(g) || !solved) {
    return EXIT_FAILURE;
  }
  game x3 = game_new_empty_ext(3, 3, false);
  game_set_square(x3, 1, 1, S_BLACK2);
  bool x3solve = game_solve(x3);
  if (!game_is_over(x3) || !x3solve) {
    return EXIT_FAILURE;
  }
  game wrap = game_new_ext(5, 3, ext_5x3w_squares, true);
  bool solvewrap = game_solve(wrap);
  if (!game_is_over(wrap) || !solvewrap) {
    return EXIT_FAILURE;
  }

  game no_sol = game_new_ext(7, 7, ext_7x7w_sqaures, true);
  bool nosol = game_solve(no_sol);
  if (game_is_over(no_sol) || nosol) {
    return EXIT_FAILURE;
  }

  game_delete(g);
  game_delete(x3);
  game_delete(wrap);
  game_delete(no_sol);
  return EXIT_SUCCESS;
}
/* ************************************************************************** */
int test_game_nb_solutions(void)
{
  game g = game_default();
  uint solution = game_nb_solutions(g);
  if (solution != 1) {
    return EXIT_FAILURE;
  }
  game x3 = game_new_empty_ext(3, 3, false);
  game_set_square(x3, 1, 1, S_BLACK2);
  solution = game_nb_solutions(x3);
  if ((solution != 4)) {
    return EXIT_FAILURE;
  }
  game wrap = game_new_ext(5, 3, ext_5x3w_squares, true);
  solution = game_nb_solutions(wrap);
  if (solution != 5) {
    return EXIT_FAILURE;
  }
  game no_sol = game_new_ext(7, 7, ext_7x7w_sqaures, true);
  solution = game_nb_solutions(no_sol);
  if (solution != 0) {
    return EXIT_FAILURE;
  }
  game_delete(g);
  game_delete(x3);
  game_delete(wrap);
  game_delete(no_sol);
  return EXIT_SUCCESS;
}