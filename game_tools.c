/**
 * @file game_tools.c
 * @copyright University of Bordeaux. All rights reserved, 2021.
 **/

#include "game_tools.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "game_ext.h"
#include "game_private.h"
#include "queue.h"

// TODO : delete
#include "game_aux.h"

/* ************************************************************************** */
/*                                 GAME TOOLS                                   */
/* ************************************************************************** */

game game_load(char* filename)
{
  unsigned char c;

  FILE* saveFile = fopen(filename, "r");
  if (saveFile == NULL) {
    fprintf(stderr, "No file named %s\n", filename);
    return NULL;
  }
  uint rows, columns, wrapping;
  if (fscanf(saveFile, "%d %d %d ", &rows, &columns, &wrapping) != 3) {
    fclose(saveFile);
    fprintf(stderr, "Invalid file format\n");
    return NULL;
  }
  game g = game_new_empty_ext(rows, columns, wrapping);
  int charScanned = 0;

  for (uint i = 0; i < rows; i++) {
    for (uint j = 0; j < columns; j++) {
      charScanned = fscanf(saveFile, "%c", &c);
      int s = _str2square(c);
      if (s == -1 || charScanned != 1) {
        game_delete(g);
        fclose(saveFile);
        fprintf(stderr, "Invalid file format\n");
        return NULL;
      }
      game_set_square(g, i, j, s);
    }
    charScanned = fscanf(saveFile, "%c", &c);
    if (c != '\n' || charScanned != 1) {
      game_delete(g);
      fclose(saveFile);
      fprintf(stderr, "Invalid file format\n");
      return NULL;
    }
  }
  fclose(saveFile);
  game_update_flags(g);
  return g;
}

/********************************************************************************/

static char image_state[255] = {
    [S_BLANK] = 'b', [S_BLACK] = '0', '1', '2', '3', '4', [S_BLACKU] = 'w', [S_LIGHTBULB] = '*', [S_MARK] = '-'};
void game_save(cgame g, char* filename)
{
  assert(g);
  assert(filename);
  FILE* file = fopen(filename, "w");
  fprintf(file, "%d %d ", g->nb_rows, g->nb_cols);
  if (game_is_wrapping(g)) {
    fprintf(file, "%d\n", 1);
  } else {
    fprintf(file, "%d\n", 0);
  }
  for (uint i = 0; i < g->nb_rows; i++) {
    for (uint j = 0; j < g->nb_cols; j++) {
      fprintf(file, "%c", image_state[game_get_state(g, i, j)]);
    }
    fprintf(file, "\n");
  }
  fclose(file);
}

/********************************************************************************/

static void remove_mark(game g)
{
  for (uint i = 0; i < g->nb_rows; i++) {
    for (uint j = 0; j < g->nb_cols; j++) {
      if (game_get_state(g, i, j) == S_MARK) {
        game_set_square(g, i, j, S_BLANK);
      }
    }
  }
  game_update_flags(g);
}

bool game_solve(game g)
{
  game_restart(g);
  game_solve_preprocessing(g);
  int len = (game_nb_cols(g) * game_nb_rows(g));
  bool playableSquares[len + 1];
  game_solve_update_playable_squares(g, playableSquares);
  unsigned int count = 0;
  bool worked = game_solve_rec(g, 0, len, &count, false, playableSquares);
  if (worked) {
    remove_mark(g);
    return worked;
  }
  game_restart(g);
  fprintf(stderr, "No solutions for this game\n");
  return false;
}

/********************************************************************************/

uint game_nb_solutions(cgame g)
{
  game copy = game_copy(g);
  game_solve_preprocessing(copy);
  int len = (game_nb_cols(g) * game_nb_rows(g));
  bool playableSquares[len + 1];
  game_solve_update_playable_squares(copy, playableSquares);
  unsigned int count = 0;
  game_solve_rec(copy, 0, len, &count, true, playableSquares);
  game_delete(copy);
  return count;
}
static uint nb_neigh_lightbulbs(cgame g, uint i, uint j)
{
  assert(g);
  int nb_rows = game_nb_rows(g);
  int nb_cols = game_nb_cols(g);
  assert(i < nb_rows);
  assert(j < nb_cols);

  int i_up = i - 1;
  int i_down = i + 1;
  int j_left = j - 1;
  int j_right = j + 1;
  if (game_is_wrapping(g)) {
    i_up = (i_up + nb_rows) % nb_rows;
    i_down = (i_down + nb_rows) % nb_rows;
    j_left = (j_left + nb_cols) % nb_cols;
    j_right = (j_right + nb_cols) % nb_cols;
  }

  uint count = 0;
  if ((i_up >= 0) && game_is_lightbulb(g, i_up, j)) count++;
  if ((i_down < nb_rows) && game_is_lightbulb(g, i_down, j)) count++;
  if ((j_left >= 0) && game_is_lightbulb(g, i, j_left)) count++;
  if ((j_right < nb_cols) && game_is_lightbulb(g, i, j_right)) count++;

  return count;
}

/* ************************************************************************** */

static uint nb_unlit_squares(cgame g)
{
  int nb = 0;
  for (uint i = 0; i < game_nb_rows(g); i++)
    for (uint j = 0; j < game_nb_cols(g); j++)
      if (!game_is_lighted(g, i, j) && game_is_blank(g, i, j)) nb++;
  return nb;
}

/* ************************************************************************** */

/**
 * Create a random game with a given size and number of walls
 *
 * @param nb_rows the number of rows of the game
 * @param nb_cols the number of columns of the game
 * @param wrapping wrapping option
 * @param nb_walls the number of walls to add
 * @param with_solution if true, the game contains the solution, otherwise only walls
 *
 * @return the generated random game
 */

game game_random(uint nb_rows, uint nb_cols, bool wrapping, uint nb_walls, bool with_solution)
{
  assert(nb_walls <= nb_rows * nb_cols);

  // step 0: create an empty game
  game g = game_new_empty_ext(nb_rows, nb_cols, wrapping);

  // step 1: add random black walls
  uint k = 0;
  while (k < nb_walls) {
    uint i = rand() % nb_rows;
    uint j = rand() % nb_cols;
    if (!game_is_black(g, i, j)) {
      game_set_square(g, i, j, S_BLACKU);
      k++;
    }
  }
  game_update_flags(g);

  // step 2: add lightbulbs until every squares are lighted
  uint nb_unlit = nb_unlit_squares(g);
  while (nb_unlit != 0) {
    uint random_unlit_num = rand() % nb_unlit;
    uint num = 0;
    for (uint i = 0; i < game_nb_rows(g); i++)
      for (uint j = 0; j < game_nb_cols(g); j++) {
        if (!game_is_lighted(g, i, j) && game_is_blank(g, i, j)) {
          if (num == random_unlit_num) {
            game_set_square(g, i, j, S_LIGHTBULB);
            game_update_flags(g);
          }
          num++;
        }
      }
    nb_unlit = nb_unlit_squares(g);
  }

  // step 3 : set some black wall numbers
  for (uint i = 0; i < game_nb_rows(g); i++)
    for (uint j = 0; j < game_nb_cols(g); j++) {
      if (game_is_black(g, i, j)) {
        if (rand() % 2 == 0) {
          int nb_lightbulbs = nb_neigh_lightbulbs(g, i, j);
          game_set_square(g, i, j, S_BLACK + nb_lightbulbs);
        }
      }
    }

  // check
  assert(game_is_over(g));

  if (!with_solution) game_restart(g);
  return g;
}