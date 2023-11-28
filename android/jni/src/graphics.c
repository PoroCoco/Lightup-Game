// SDL2 Demo by aurelien.esnard@u-bordeaux.fr

#include "graphics.h"

#include <SDL.h>
#include <SDL_image.h>  // required to load transparent texture from PNG
#include <SDL_ttf.h>    // required to use TTF fonts
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_private.h"
#include "game_tools.h"

/* **************************************************************** */
static char* _texts_strings[] = {"0", "1", "2", "3", "4", "CONGRATS YOU MADE IT HOMIE!", "LIGHTUP CRAFT"};
static char* _textures_names[] = {"redstone_lamp_off.png",
                                  "redstone_lamp_on.png",
                                  "redstone_torch_on.png",
                                  "redstone_torch_off.png",
                                  "brick.png",
                                  "redstone_dust_cross.png",
                                  "destroy_stage_7.png",
                                  "background_screen.png",
                                  "lightup_btn_undo.png",
                                  "lightup_btn_restart.png",
                                  "lightup_btn_mark.png",
                                  "lightup_btn_solve.png",
                                  "lightup_btn_redo.png"};

/* **************************************************************** */

Env* init(SDL_Window* win, SDL_Renderer* ren, int argc, char* argv[])
{
  Env* env = malloc(sizeof(struct Env_t));
  env->scale = 1;

  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  env->textures = malloc(sizeof(SDL_Texture*) * (TEXTURE_COUNT + BUTTON_COUNT));
  /* init all textures from PNG images */
  for (int i = 0; i < TEXTURE_COUNT + BUTTON_COUNT; i++) {
    env->textures[i] = IMG_LoadTexture(ren, _textures_names[i]);
    if (!env->textures[i]) ERROR("IMG_LoadTexture: %s : %s\n", _textures_names[i], SDL_GetError());
  }

  /* init all text textures */
  SDL_Color color_gray = {204, 201, 221, 0};
  SDL_Color color_black = {0, 0, 0, 0};
  SDL_Color color;
  TTF_Font* font_arial = TTF_OpenFont(FONT, FONTSIZE);
  if (!font_arial) ERROR("TTF_OpenFont: %s\n", FONT);
  TTF_Font* font_minecraft = TTF_OpenFont(FONT_MC, FONTSIZE);
  if (!font_minecraft) ERROR("TTF_OpenFont: %s\n", FONT_MC);
  TTF_Font* font;
  TTF_SetFontStyle(font_arial, TTF_STYLE_BOLD);
  env->texts = malloc(sizeof(SDL_Texture*) * TEXT_COUNT);
  for (int i = 0; i < TEXT_COUNT; i++) {
    if (i == TEXT_WINNING) {
      color = color_gray;
    } else {
      color = color_black;
    }
    if (i == TEXT_WINNING || i == TEXT_TITLE) {
      font = font_minecraft;
    } else {
      font = font_arial;
    }
    SDL_Surface* surf = TTF_RenderText_Blended(font, _texts_strings[i], color);
    env->texts[i] = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);
  }

  TTF_CloseFont(font_arial);
  TTF_CloseFont(font_minecraft);

  // game init
  game g = NULL;
  if (argc == 2)
    g = game_load(argv[1]);
  else {
    srand(time(NULL));  // initialize radom seed with current time
    g = game_random(7, 7, false, 10, false);
  }

  env->g = g;
  env->btn_mark_switch = false;
  // _reserve_audio(env);
  SDL_Surface* icon = IMG_Load("textures/icon.png");
  SDL_SetWindowIcon(win, icon);
  env->woncount = 1;
  return env;
}

/* **************************************************************** */

void render(SDL_Window* win, SDL_Renderer* ren, Env* env)
{
  game g = env->g;
  /* get current window size */
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  SDL_RenderCopy(ren, env->textures[TEXTURE_BACKGROUND], NULL, NULL);

  int square_width = (w - ((w * PADDING_X) / 100)) / g->nb_cols;
  int square_height = (h - ((h * PADDING_Y) / 100)) / g->nb_rows;
  square_width = square_width - (square_width % TEXTURES_SIZE);
  square_height = square_height - (square_height % TEXTURES_SIZE);
  int square_size;
  if (square_width < TEXTURES_SIZE || square_height < TEXTURES_SIZE) {
    square_size = TEXTURES_SIZE;
  } else if (square_width > square_height) {
    square_size = square_height;
  } else {
    square_size = square_width;
  }
  int padding_w = w - (square_size * g->nb_cols);
  int padding_h = h - (square_size * g->nb_rows);

  env->padding_w = padding_w;
  env->padding_h = padding_h;
  env->scale = square_size / TEXTURES_SIZE;

  int texture_level[4];
  for (uint i = 0; i < g->nb_rows; i++) {
    for (uint j = 0; j < g->nb_cols; j++) {
      _get_square_texture(g, i, j, texture_level);
      _render_square(env, ren, i, j, square_size, texture_level);
    }
  }
  _bar_render(win, env, ren);

  _title_render(win, env, ren);

  if (game_is_over(g)) {
    _render_game_win(win, env, ren);
    // if (env->woncount == 1) {
    //   Mix_PlayChannel(-1, env->won, 0);
    //   env->woncount--;
    // }
  }
}

/* **************************************************************** */

bool process(SDL_Window* win, SDL_Renderer* ren, Env* env, SDL_Event* e)
{
  if (e->type == SDL_QUIT) {
    ren = ren;
    return true;
  }
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  game g = env->g;
  if (e->type == SDL_MOUSEBUTTONDOWN) {
    SDL_Point mouse;
    SDL_GetMouseState(&mouse.x, &mouse.y);
    int square_size = env->scale * TEXTURES_SIZE;
    uint col = (mouse.x - (env->padding_w / 2)) / square_size;
    uint row = (mouse.y - (env->padding_h / 2)) / square_size;
    switch (e->button.button) {
      case SDL_BUTTON_LEFT:
        if (mouse.y >= env->bar_start_h && mouse.y < env->bar_start_h + TEXTURES_SIZE * env->bar_scale) {
          int button_index = (mouse.x - env->bar_start_w) / (TEXTURES_SIZE * env->bar_scale);
          switch (button_index) {
            case 0:
              game_undo(g);
              break;
            case 1:
              env->woncount = 1;
              game_restart(g);
              break;
            case 2:
              if (env->btn_mark_switch) {
                env->btn_mark_switch = false;
              } else {
                env->btn_mark_switch = true;
              }
              break;
            case 3:
              game_solve(g);
              break;
            case 4:
              game_redo(g);
              break;
            default:
              break;
          }
        }
        if (row >= env->g->nb_rows || col >= env->g->nb_cols) break;
        square chosenState;
        if (env->btn_mark_switch) {
          chosenState = S_MARK;
        } else {
          chosenState = S_LIGHTBULB;
        }
        switch (game_get_state(env->g, row, col)) {
          case S_BLANK:
            if (game_check_move(env->g, row, col, chosenState)) game_play_move(env->g, row, col, chosenState);
            break;
          case S_LIGHTBULB:
            if (chosenState == S_LIGHTBULB) {
              if (game_check_move(env->g, row, col, S_BLANK)) game_play_move(env->g, row, col, S_BLANK);
            } else {
              if (game_check_move(env->g, row, col, chosenState)) game_play_move(env->g, row, col, chosenState);
            }
            break;
          case S_MARK:
            if (chosenState == S_MARK) {
              if (game_check_move(env->g, row, col, S_BLANK)) game_play_move(env->g, row, col, S_BLANK);
            } else {
              if (game_check_move(env->g, row, col, chosenState)) game_play_move(env->g, row, col, chosenState);
            }
            break;
          default:
            break;
        }
        break;
      case SDL_BUTTON_RIGHT:
        switch (game_get_state(env->g, row, col)) {
          case S_BLANK:
            if (game_check_move(env->g, row, col, S_MARK)) game_play_move(env->g, row, col, S_MARK);
            break;
          case S_LIGHTBULB:
            if (game_check_move(env->g, row, col, S_MARK)) game_play_move(env->g, row, col, S_MARK);
            break;
          case S_MARK:
            if (game_check_move(env->g, row, col, S_BLANK)) game_play_move(env->g, row, col, S_BLANK);
          default:
            break;
        }
        break;
      default:
        break;
    }
  }
  if (e->type == SDL_KEYDOWN) {
    switch (e->key.keysym.sym) {
      case SDLK_ESCAPE:
        return true;
        break;
      case SDLK_r:
        env->woncount = 1;
        game_restart(env->g);
        break;
      case SDLK_s:
        game_solve(env->g);
        break;
      case SDLK_z:
        game_undo(env->g);
        break;
      case SDLK_y:

        game_redo(env->g);
        break;
      case SDLK_w:
        game_save(env->g, "GameSave.txt");
        break;
    }
  }
  return false;
}
/* **************************************************************** */

void clean(SDL_Window* win, SDL_Renderer* ren, Env* env)
{
  for (int i = 0; i < TEXTURE_COUNT + BUTTON_COUNT; i++) {
    SDL_DestroyTexture(env->textures[i]);
  }
  free(env->textures);
  for (int i = 0; i < TEXT_COUNT; i++) {
    SDL_DestroyTexture(env->texts[i]);
  }
  free(env->texts);
  game_delete(env->g);
  // Mix_FreeMusic(env->ost);
  // Mix_FreeChunk(env->won);
  // Mix_CloseAudio();
  free(env);
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
}

/* ************************************************************************** */
/*                                 AUX FUNC                                   */
/* ************************************************************************** */

void _get_square_texture(cgame g, uint i, uint j, int* texture_level)
{
  texture_level[0] = -1;  // lighted or blank
  texture_level[1] = -1;  // state (lightbulb mark wall)
  texture_level[2] = -1;  // texts
  texture_level[3] = -1;  // error texture

  if (game_is_lighted(g, i, j)) {
    texture_level[0] = TEXTURE_BLANK_LIGHTED;
  } else {
    texture_level[0] = TEXTURE_BLANK;
  }

  square s = game_get_state(g, i, j);
  if (s == S_LIGHTBULB && game_has_error(g, i, j)) {
    texture_level[1] = TEXTURE_LIGHTBULB_ERROR;
  } else if (s == S_LIGHTBULB) {
    texture_level[1] = TEXTURE_LIGHTBULB;
  } else if (s == S_MARK) {
    texture_level[1] = TEXTURE_MARK;
  } else if (game_is_black(g, i, j)) {
    texture_level[1] = TEXTURE_WALL;
    int wallNumber = game_get_black_number(g, i, j);
    if (wallNumber == 0) {
      texture_level[2] = TEXT_0;
    } else if (wallNumber == 1) {
      texture_level[2] = TEXT_1;
    } else if (wallNumber == 2) {
      texture_level[2] = TEXT_2;
    } else if (wallNumber == 3) {
      texture_level[2] = TEXT_3;
    } else if (wallNumber == 4) {
      texture_level[2] = TEXT_4;
    }
  }

  if (game_has_error(g, i, j) && game_is_black(g, i, j)) {
    texture_level[3] = TEXTURE_ERROR;
  }
}

void _render_square(Env* env, SDL_Renderer* ren, uint i, uint j, int square_size, int* texture_level)
{
  for (int level = 0; level < 4; level++) {
    if (texture_level[level] != -1) {
      int x = (env->padding_w / 2) + (square_size * j);
      int y = (env->padding_h / 2) + (square_size * i);
      if (level == 2) {
        _render_texture(ren, env->texts[texture_level[level]], TEXTURES_SIZE * env->scale, TEXTURES_SIZE * env->scale,
                        x, y);
      } else {
        _render_texture(ren, env->textures[texture_level[level]], TEXTURES_SIZE * env->scale,
                        TEXTURES_SIZE * env->scale, x, y);
      }
    }
  }
}

void _bar_render(SDL_Window* win, Env* env, SDL_Renderer* ren)
{
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  int grid_size = env->scale * TEXTURES_SIZE * env->g->nb_cols;
  int button_size = grid_size / BUTTON_COUNT;
  button_size = button_size - button_size % TEXTURES_SIZE;
  int padding_bar = grid_size - (button_size * BUTTON_COUNT);
  int bar_scale = button_size / TEXTURES_SIZE;
  env->bar_scale = bar_scale;
  env->bar_start_w = env->padding_w / 2 + (padding_bar / 2);
  env->bar_start_h = (h - (env->padding_h / 2));

  // for each button_count adds a bar button under the game grid, textures must be loaded in the same order as the
  // buttons.
  for (uint i = 0; i < BUTTON_COUNT; i++) {
    int x = (env->padding_w / 2 + (padding_bar / 2)) + i * TEXTURES_SIZE * bar_scale;
    int y = (h - (env->padding_h / 2));
    _render_texture(ren, env->textures[TEXTURE_COUNT + i], TEXTURES_SIZE * bar_scale, TEXTURES_SIZE * bar_scale, x, y);
  }
}

void _render_text(SDL_Renderer* ren, SDL_Texture* text_texture, int w, int h, int x, int y)
{
  SDL_Rect txt;
  SDL_QueryTexture(text_texture, NULL, NULL, &txt.w, &txt.h);
  txt.w = w;
  txt.h = h;
  txt.x = x - txt.w / 2;
  txt.y = y;
  SDL_RenderCopy(ren, text_texture, NULL, &txt);
}
void _render_game_win(SDL_Window* win, Env* env, SDL_Renderer* ren)
{
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  _render_text(ren, env->texts[TEXT_WINNING], (w - ((w * PADDING_X) / 100)), h / 10, w / 2, h / 2 - 10);
}

void _title_render(SDL_Window* win, Env* env, SDL_Renderer* ren)
{
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  _render_text(ren, env->texts[TEXT_TITLE], (w - ((w * PADDING_X) / 100)), (h * (PADDING_Y / 2)) / 100, w / 2, 5);
}

// void _reserve_audio(Env* env)
// {
//   env->ost = Mix_LoadMUS("ost.mp3");
//   env->won = Mix_LoadWAV("orb.wav");
//   Mix_PlayMusic(env->ost, -1);
// }

void _render_texture(SDL_Renderer* ren, SDL_Texture* texture, int width, int height, int x, int y)
{
  SDL_Rect rect;
  SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;
  SDL_RenderCopy(ren, texture, NULL, &rect);
}