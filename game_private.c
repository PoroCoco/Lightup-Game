/**
 * @file game_private.c
 * @copyright University of Bordeaux. All rights reserved, 2021.
 **/

#include "game_private.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "queue.h"

/* ************************************************************************** */
/*                             STACK ROUTINES                                 */
/* ************************************************************************** */

void _stack_push_move(queue* q, move m)
{
  assert(q);
  move* pm = malloc(sizeof(move));
  assert(pm);
  *pm = m;
  queue_push_head(q, pm);
}

/* ************************************************************************** */

move _stack_pop_move(queue* q)
{
  assert(q);
  move* pm = queue_pop_head(q);
  assert(pm);
  move m = *pm;
  free(pm);
  return m;
}

/* ************************************************************************** */

bool _stack_is_empty(queue* q)
{
  assert(q);
  return queue_is_empty(q);
}

/* ************************************************************************** */

void _stack_clear(queue* q)
{
  assert(q);
  queue_clear_full(q, free);
  assert(queue_is_empty(q));
}

/* ************************************************************************** */
/*                          GAME PRIVATE ROUTINES                             */
/* ************************************************************************** */

static char image[255] = {
    [S_BLANK] = ' ', [S_BLACK] = '0', '1', '2', '3', '4', [S_BLACKU] = 'w', [S_LIGHTBULB] = '*', [S_MARK] = '-'};

/* ************************************************************************** */

char _square2str(square s)
{
  square state = s & S_MASK;
  square flags = s & F_MASK;
  if ((state == S_BLANK) && (flags & F_LIGHTED)) return '.';
  return image[state];
}

/* ************************************************************************** */

static int value[255] = {['b'] = S_BLANK,  ['0'] = S_BLACK0, ['1'] = S_BLACK1,    ['2'] = S_BLACK2, ['3'] = S_BLACK3,
                         ['4'] = S_BLACK4, ['w'] = S_BLACKU, ['*'] = S_LIGHTBULB, ['-'] = S_MARK};

/* ************************************************************************** */

int _str2square(char c)
{
  unsigned char uc = c;
  if (uc == 'b') return 0;
  return value[uc] == 0 ? -1 : value[uc];
}

/* ************************************************************************** */

bool _check_square(square s)
{
  square state = s & S_MASK;
  square flags = s & F_MASK;
  if (state < S_START || state > S_END) return false;
  if ((flags & ~(F_LIGHTED | F_ERROR)) != 0) return false;
  return true;
}

/* ************************************************************************** */
/*                                 NEIGHBORHOOD                               */
/* ************************************************************************** */

/* Nota Bene: https://en.cppreference.com/w/c/language/array_initialization */
static int i_offset[] = {[HERE] = 0,     [UP] = -1,       [DOWN] = +1,     [LEFT] = 0,      [RIGHT] = 0,
                         [UP_LEFT] = -1, [UP_RIGHT] = -1, [DOWN_LEFT] = 1, [DOWN_RIGHT] = 1};
static int j_offset[] = {[HERE] = 0,     [UP] = 0,       [DOWN] = 0,       [LEFT] = -1,     [RIGHT] = 1,
                         [UP_LEFT] = -1, [UP_RIGHT] = 1, [DOWN_LEFT] = -1, [DOWN_RIGHT] = 1};

/* ************************************************************************** */

bool _inside(cgame g, int i, int j)
{
  assert(g);
  if (game_is_wrapping(g)) {
    i = (i + game_nb_rows(g)) % game_nb_rows(g);
    j = (j + game_nb_cols(g)) % game_nb_cols(g);
  }
  if (i < 0 || j < 0 || i >= (int)g->nb_rows || j >= (int)g->nb_cols) return false;
  return true;
}

/* ************************************************************************** */

bool _inside_neigh(cgame g, int i, int j, direction dir) { return _inside(g, i + i_offset[dir], j + j_offset[dir]); }

/* ************************************************************************** */

bool _next(cgame g, int* pi, int* pj, direction dir)
{
  assert(g);
  assert(pi && pj);
  int i = *pi;
  int j = *pj;
  assert(i >= 0 && j >= 0 && i < (int)g->nb_rows && j < (int)g->nb_cols);

  // move to the next square in a given direction
  i += i_offset[dir];
  j += j_offset[dir];

  if (game_is_wrapping(g)) {
    i = (i + game_nb_rows(g)) % game_nb_rows(g);
    j = (j + game_nb_cols(g)) % game_nb_cols(g);
  }
  if (!_inside(g, i, j)) return false;

  // update square coords
  *pi = i;
  *pj = j;

  return true;
}

/* ************************************************************************** */

bool _test(cgame g, int i, int j, square s, uint m)
{
  assert(g);
  assert(s >= S_START && s < S_END);
  if (game_is_wrapping(g)) {
    i = (i + game_nb_rows(g)) % game_nb_rows(g);
    j = (j + game_nb_cols(g)) % game_nb_cols(g);
  }
  if (!_inside(g, i, j)) return false;
  return ((SQUARE(g, i, j) & m) == s);
}

/* ************************************************************************** */

bool _test_neigh(cgame g, int i, int j, square s, uint m, direction dir)
{
  return _test(g, i + i_offset[dir], j + j_offset[dir], s, m);
}

/* ************************************************************************** */

bool _neigh(cgame g, uint i, uint j, square s, uint m, bool diag)
{
  assert(g);
  assert(s >= S_START && s < S_END);

  // orthogonally
  if (_test_neigh(g, i, j, s, m, UP) || _test_neigh(g, i, j, s, m, DOWN) || _test_neigh(g, i, j, s, m, LEFT) ||
      _test_neigh(g, i, j, s, m, RIGHT))
    return true;

  // diagonally
  return diag && (_test_neigh(g, i, j, s, m, UP_LEFT) || _test_neigh(g, i, j, s, m, UP_RIGHT) ||
                  _test_neigh(g, i, j, s, m, DOWN_LEFT) || _test_neigh(g, i, j, s, m, DOWN_RIGHT));
}

/* ************************************************************************** */

uint _neigh_size(cgame g, uint i, uint j, bool diag)
{
  assert(g);

  return _neigh_count(g, i, j, 0, 0, diag);
}

/* ************************************************************************** */

uint _neigh_count(cgame g, uint i, uint j, square s, uint m, bool diag)
{
  assert(g);

  // orthogonally
  uint count = _test_neigh(g, i, j, s, m, UP) + _test_neigh(g, i, j, s, m, DOWN) + _test_neigh(g, i, j, s, m, LEFT) +
               _test_neigh(g, i, j, s, m, RIGHT);

  // diagonally
  if (diag) {
    count += _test_neigh(g, i, j, s, m, UP_LEFT) + _test_neigh(g, i, j, s, m, UP_RIGHT) +
             _test_neigh(g, i, j, s, m, DOWN_LEFT) + _test_neigh(g, i, j, s, m, DOWN_RIGHT);
  }

  return count;
}

/* ************************************************************************** */
/*                                 SOLVE                                      */
/* ************************************************************************** */

// transforms the index into row and column indexes
static void _pos_to_row_col(game g, uint index, int* prow, int* pcolumn)
{
  int nb_col = game_nb_cols(g);
  *prow = index / nb_col;
  *pcolumn = index % nb_col;
}

// true if at least 1 square has an error, false otherwise
static bool _game_has_error_full(game g)
{
  for (uint i = 0; i < g->nb_rows; i++)
    for (uint j = 0; j < g->nb_cols; j++) {
      if (game_get_flags(g, i, j) == F_ERROR) {
        return true;
      }
    }
  return false;
}

// marks all the squares neighbouring 0 walls
static void _game_solve_mark_wall_0(game g)
{
  for (uint i = 0; i < g->nb_rows; i++)
    for (uint j = 0; j < g->nb_cols; j++) {
      if (game_is_black(g, i, j) && game_get_black_number(g, i, j) == 0) {
        int newI = i;
        int newJ = j;
        if (_next(g, &newI, &newJ, UP) && game_check_move(g, newI, newJ, S_MARK))
          game_set_square(g, newI, newJ, S_MARK);

        newI = i;
        newJ = j;
        if (_next(g, &newI, &newJ, DOWN) && game_check_move(g, newI, newJ, S_MARK))
          game_set_square(g, newI, newJ, S_MARK);

        newI = i;
        newJ = j;
        if (_next(g, &newI, &newJ, LEFT) && game_check_move(g, newI, newJ, S_MARK))
          game_set_square(g, newI, newJ, S_MARK);

        newI = i;
        newJ = j;
        if (_next(g, &newI, &newJ, RIGHT) && game_check_move(g, newI, newJ, S_MARK))
          game_set_square(g, newI, newJ, S_MARK);
      }
    }
}

// place mandatory lights to solve a game
static bool _game_solve_necessary_light(game g)
{
  bool played = false;
  for (uint i = 0; i < g->nb_rows; i++)
    for (uint j = 0; j < g->nb_cols; j++) {
      if (game_is_black(g, i, j)) {
        if (game_get_black_number(g, i, j) - _neigh_count(g, i, j, S_LIGHTBULB, 0xF, false) ==
            _neigh_count(g, i, j, S_BLANK, 0xFFFF, false)) {
          for (int offset = -1; offset < 2; offset += 2) {
            if (game_check_move(g, i + offset, j, S_LIGHTBULB) && game_get_square(g, i + offset, j) == S_BLANK) {
              played = true;
              game_set_square(g, i + offset, j, S_LIGHTBULB);
            }
            if (game_check_move(g, i, j + offset, S_LIGHTBULB) && game_get_square(g, i, j + offset) == S_BLANK) {
              played = true;
              game_set_square(g, i, j + offset, S_LIGHTBULB);
            }
          }
        }
        if (played) game_update_flags(g);
      }
    }
  if (played) _game_solve_necessary_light(g);
  return played;
}

void game_solve_preprocessing(game g)
{
  _game_solve_mark_wall_0(g);
  _game_solve_necessary_light(g);
}

bool game_solve_rec(game g, int pos, int len, unsigned int* count, bool isCounting, bool* playableSquares)
{
  // stops the recursive branch if it generates a game with errors
  if (_game_has_error_full(g)) {
    return false;
  }

  // increment the position until it's a non lighted playable square
  int row, column;
  _pos_to_row_col(g, pos, &row, &column);
  while (!playableSquares[pos] || game_get_flags(g, row, column) == F_LIGHTED) {
    if (pos == len) break;
    pos++;
    _pos_to_row_col(g, pos, &row, &column);
  }

  // checks if a game is over once it has been generated
  if (pos == len) {
    if (game_is_over(g)) {
      (*count)++;
      return true;
    }
    return false;
  }

  // extend the game with a lightbulb at the next position if possible
  if (game_check_move(g, row, column, S_LIGHTBULB)) {
    game_set_square(g, row, column, S_LIGHTBULB);
    game_update_flags(g);
  }
  if (game_solve_rec(g, pos + 1, len, count, isCounting, playableSquares) && !isCounting) {
    return true;
  }

  // extend the game with a blank square at the next position
  if (game_check_move(g, row, column, S_BLANK)) {
    game_set_square(g, row, column, S_BLANK);
    game_update_flags(g);
  }
  if (game_solve_rec(g, pos + 1, len, count, isCounting, playableSquares) && !isCounting) {
    return true;
  }
  return false;
}

void game_solve_update_playable_squares(game g, bool* tab)
{
  for (uint i = 0; i < g->nb_rows; i++)
    for (uint j = 0; j < g->nb_cols; j++) {
      if (game_get_flags(g, i, j) == F_LIGHTED || game_is_black(g, i, j) || game_get_state(g, i, j) == S_MARK) {
        tab[INDEX(g, i, j)] = false;
      } else {
        tab[INDEX(g, i, j)] = true;
      }
    }
  tab[INDEX(g, g->nb_rows, 0)] = false;
}
