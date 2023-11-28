#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_private.h"
#include "game_tools.h"

int main(int argc, char* argv[])
{
  if (argc < 3) {  // check if the user gave the correct number of arguments
    printf("few arguments\n");
    return EXIT_FAILURE;
  }
  char* filename = NULL;
  if (argc == 3) {  // if the output file is not defined by the user, the game creates a new one instead
    filename = "default.txt";
  } else if (argc >= 4) {
    filename = argv[3];
  }

  game g = game_load(argv[2]);  // load the input game and place the solution inside the output file
  if (strcmp("-s", argv[1]) == 0) {
    if (game_solve(g)) {
      game_save(g, filename);
      if (argc == 3) {
        game_print(g);
      }
    } else {
      game_delete(g);
      return EXIT_FAILURE;
    }
  } else if (strcmp("-c", argv[1]) == 0) {  // store the number of solutions inside the output file
    FILE* file = fopen(filename, "w");
    uint solutions = game_nb_solutions(g);
    fprintf(file, "%u\n", solutions);
    if (argc == 3) {
      printf("we found %u solutions\n", solutions);
    }
    fclose(file);
  }
  game_delete(g);
  return EXIT_SUCCESS;
}