// SDL2 Model by aurelien.esnard@u-bordeaux.fr

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "game.h"

struct Env_t {
  game g;
  SDL_Texture** texts;
  SDL_Texture** textures;
  int scale;
  int padding_w;
  int padding_h;
  int bar_start_w;
  int bar_start_h;
  int bar_scale;
  uint woncount;
  Mix_Music* ost;
  Mix_Chunk* won;
};

typedef struct Env_t Env;

/* **************************************************************** */

#ifdef __ANDROID__
#define PRINT(STR, ...)          \
  do {                           \
    SDL_Log(STR, ##__VA_ARGS__); \
  } while (0)
#define ERROR(STR, ...)          \
  do {                           \
    SDL_Log(STR, ##__VA_ARGS__); \
    exit(EXIT_FAILURE);          \
  } while (0)
#else
#define PRINT(STR, ...)         \
  do {                          \
    printf(STR, ##__VA_ARGS__); \
  } while (0)
#define ERROR(STR, ...)                  \
  do {                                   \
    fprintf(stderr, STR, ##__VA_ARGS__); \
    exit(EXIT_FAILURE);                  \
  } while (0)
#endif

/* **************************************************************** */

#define APP_NAME "LightUpCraft"
#define SCREEN_WIDTH 700
#define SCREEN_HEIGHT 600
#define DELAY 30
#define PADDING_X 20  // padding (in percentage) arount the game grid
#define PADDING_Y 23

/* **************************************************************** */

#define FONT "arial.ttf"
#define FONT_MC "Minecraft.ttf"
#define FONTSIZE 64

#define TEXT_COUNT 7
#define TEXT_0 0
#define TEXT_1 1
#define TEXT_2 2
#define TEXT_3 3
#define TEXT_4 4
#define TEXT_WINNING 5
#define TEXT_TITLE 6

/* **************************************************************** */

#define TEXTURES_SIZE 16

#define TEXTURE_COUNT 8
#define TEXTURE_BLANK 0
#define TEXTURE_BLANK_LIGHTED 1
#define TEXTURE_LIGHTBULB 2
#define TEXTURE_LIGHTBULB_ERROR 3
#define TEXTURE_WALL 4
#define TEXTURE_MARK 5
#define TEXTURE_ERROR 6
#define TEXTURE_BACKGROUND 7

#define BUTTON_COUNT 5
#define TEXTURE_BTN_UNDO TEXTURE_COUNT
#define TEXTURE_BTN_RESTART TEXTURE_COUNT + 1
#define TEXTURE_BTN_SAVE TEXTURE_COUNT + 2
#define TEXTURE_BTN_SOLVE TEXTURE_COUNT + 3
#define TEXTURE_BTN_REDO TEXTURE_COUNT + 4

/* **************************************************************** */

/**
 * @brief initialize the environment variables
 *
 * @param win the SDL_window
 * @param ren the renderer
 * @param argc arg count
 * @param argv arg values
 * @returns the dynamically allocated environment
 */
Env* init(SDL_Window* win, SDL_Renderer* ren, int argc, char* argv[]);

/**
 * @brief render the current game state on screen
 *
 * @param win the SDL_window
 * @param ren the renderer
 * @param env the environment with the variables
 */
void render(SDL_Window* win, SDL_Renderer* ren, Env* env);

/**
 * @brief frees all the dynamically allocated variables
 *
 * @param win the SDL_window
 * @param ren the renderer
 * @param env the environment with the variables
 */
void clean(SDL_Window* win, SDL_Renderer* ren, Env* env);

/**
 * @brief frees all the dynamically allocated variables
 *
 * @param win the SDL_window
 * @param ren the renderer
 * @param env the environment with the variables
 * @param e the SDL_event to process
 * @returns true if event should close the game, false otherwise
 */
bool process(SDL_Window* win, SDL_Renderer* ren, Env* env, SDL_Event* e);

/**
 * @brief updates the texture_level array with the corresponding textures for a square
 *
 * @param g the game
 * @param i row
 * @param j column
 * @param texture_level int array of size 4
 * @pre @p texture_level must be at least of size 4
 */
void _get_square_texture(cgame g, uint i, uint j, int* texture_level);

/**
 * @brief updates the renderer with one of the square texture
 *
 * @param env the environment with the variables
 * @param ren the renderer
 * @param i row
 * @param j column
 * @param square_size size in pixel of the square
 * @param texture_level int array of size 4 with the textures indexes
 * @pre @p texture_level must be at least of size 4 and already have the textures indexes loaded
 */
void _render_square(Env* env, SDL_Renderer* ren, uint i, uint j, int square_size, int* texture_level);

/**
 * @brief renders the menu bar with the buttons icons
 *
 * @details renders a bar of BUTTON_SIZE buttons under the game grid
 * @param win the SDL_window
 * @param env the environment with the variables
 * @param ren the renderer
 */
void _bar_render(SDL_Window* win, Env* env, SDL_Renderer* ren);

/**
 * @brief renders the winning text on top of the game
 *
 * @param win the SDL_window
 * @param env the environment with the variables
 * @param ren the renderer
 */
void _render_game_win(SDL_Window* win, Env* env, SDL_Renderer* ren);

/**
 * @brief renders the winning text on top of the game
 *
 * @param win the SDL_window
 * @param env the environment with the variables
 * @param ren the renderer
 */
void _title_render(SDL_Window* win, Env* env, SDL_Renderer* ren);

/**
 * @brief renders the given text texture
 *
 * @details different from _render_texture as the rendered text is always width centered
 * @param ren the renderer
 * @param text_texture the text to render
 * @param w width of the text
 * @param h height of the text
 * @param x x top corner
 * @param y y top corner
 */
void _render_text(SDL_Renderer* ren, SDL_Texture* text_texture, int w, int h, int x, int y);

/**
 * @brief renders the given texture
 *
 * @param ren the renderer
 * @param texture the texture to render
 * @param width desired width of the texture
 * @param height desired height of the texture
 * @param x x top corner
 * @param y y top corner
 */
void _render_texture(SDL_Renderer* ren, SDL_Texture* texture, int width, int height, int x, int y);

/**
 * @brief associate the correct audio file with game winning event, background music etc ...
 *
 * @param env the environment with variables
 */
void _reserve_audio(Env* env);
/* **************************************************************** */
#endif
