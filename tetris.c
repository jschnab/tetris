#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"


#define SCREEN_FPS 60
#define SCREEN_TICKS_PER_FRAME (1000 / SCREEN_FPS)

#define PLAYFIELD_CELL_WIDTH 16
#define PLAYFIELD_CELL_HEIGHT 24
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CELL_WIDTH 16
#define PLAYFIELD_WIDTH (PLAYFIELD_CELL_WIDTH * CELL_WIDTH)
#define PLAYFIELD_HEIGHT (PLAYFIELD_CELL_HEIGHT * CELL_WIDTH)
#define PLAYFIELD_POSITION_X (SCREEN_WIDTH / 2 - PLAYFIELD_WIDTH / 2)
#define PLAYFIELD_POSITION_Y 50
#define PIECE_VELOCITY 10
#define PIECE_MATRIX_WIDTH 4
#define PIECE_MATRIX_HEIGHT 4


enum SHAPES {I = 1, J, L, O, S, T, Z};

typedef struct texture {
    SDL_Texture *texture;
    int width;
    int height;
} Texture;

typedef struct piece {
    int shape;
    int posx;
    int posy;
    int velx;
    int vely;
    int matrix[PIECE_MATRIX_HEIGHT][PIECE_MATRIX_WIDTH];
} Piece;

typedef struct timer {
    uint32_t start_ticks;
    bool started;
} Timer;


SDL_Window *gWindow = NULL;
SDL_Renderer *gRenderer = NULL;
Texture gCellTexture = {NULL, 0, 0};
int gPlayfield[PLAYFIELD_CELL_HEIGHT][PLAYFIELD_CELL_WIDTH];


void close_all();
bool initialize();
bool load_media();
void playfield_print();
void playfield_render();
void playfield_update(Piece *);
void texture_destroy(Texture *);
bool texture_from_file(Texture *, char *);
void texture_render(Texture *, int, int, SDL_Rect *);
uint32_t timer_get_ticks(Timer *);
void timer_start(Timer *);
void timer_stop(Timer *);


void close_all() {
    texture_destroy(&gCellTexture);
    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    IMG_Quit();
    SDL_Quit();
}


bool initialize() {
    check(
        SDL_Init(SDL_INIT_VIDEO) == 0,
        "Failed to initialize SDL: %s",
        SDL_GetError()
    );
    check(
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"),
        "Linear texture filtering not enabled"
    );
    gWindow = SDL_CreateWindow(
        "Tetris",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0
    );
    check_mem(gWindow);
    gRenderer = SDL_CreateRenderer(
        gWindow,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    check_mem(gRenderer);
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    int img_flags = IMG_INIT_PNG;
    check(
        IMG_Init(img_flags) & img_flags,
        "Failed to initialize SDL_image: %s",
        IMG_GetError()
    );

    for (int i = 0; i < PLAYFIELD_CELL_HEIGHT; i++) {
        for (int j = 0; j < PLAYFIELD_CELL_WIDTH; j++) {
            gPlayfield[i][j] = 0;
        }
    }
    return true;

    error:
        return false;
}


bool load_media() {
    check(texture_from_file(&gCellTexture, "cells.png"), "Failed to load cells texture");
    return true;

    error:
        return false;
}


void playfield_print() {
    for (int i = 0; i < PLAYFIELD_CELL_HEIGHT; i++) {
        for (int j = 0; j < PLAYFIELD_CELL_WIDTH; j++) {
            printf("%d", gPlayfield[i][j]);
        }
        printf("\n");
    }
}


void playfield_render() {
    int x, y;  /* pixel position on the screen */

    /* iterate over playfied area */
    for (int i = 0; i < PLAYFIELD_CELL_HEIGHT; i++) {
        for (int j = 0; j < PLAYFIELD_CELL_WIDTH; j++) {
            x = j * CELL_WIDTH + PLAYFIELD_POSITION_X;
            y = i * CELL_WIDTH + PLAYFIELD_POSITION_Y;
            /* draw cells with appropriate color */
            SDL_Rect clip = {
                gPlayfield[i][j] * CELL_WIDTH,
                0,
                CELL_WIDTH,
                CELL_WIDTH
            };
            texture_render(&gCellTexture, x, y, &clip);
        }
    }
}


void playfield_update(Piece *piece) {
    /* iterate over playfied area */
    for (int i = 0; i < PLAYFIELD_CELL_HEIGHT; i++) {
        for (int j = 0; j < PLAYFIELD_CELL_WIDTH; j++) {
            if (
                j >= piece->posx &&
                j < piece->posx + PIECE_MATRIX_WIDTH &&
                i >= piece->posy &&
                i < piece->posy + PIECE_MATRIX_HEIGHT &&
                piece->matrix[i - piece->posy][j - piece->posx] == 1
            ) {
                gPlayfield[i][j] = piece->shape;
            }
        }
    }
}


void texture_destroy(Texture *t) {
    if (t->texture != NULL) {
        SDL_DestroyTexture(t->texture);
        t->texture = NULL;
        t->width = 0;
        t->height = 0;
    }
}


bool texture_from_file(Texture *t, char *path) {
    texture_destroy(t);
    SDL_Surface *loaded_surface = IMG_Load(path);
    check_mem(loaded_surface);
    SDL_SetColorKey(
        loaded_surface,
        SDL_TRUE,
        SDL_MapRGB(loaded_surface->format, 0, 0xFF, 0xFF)
    );
    t->texture = SDL_CreateTextureFromSurface(gRenderer, loaded_surface);
    check_mem(t->texture);
    t->width = loaded_surface->w;
    t->height = loaded_surface->h;
    SDL_FreeSurface(loaded_surface);
    return true;

    error:
        SDL_FreeSurface(loaded_surface);
        return false;
}


void texture_render(Texture *t, int x, int y, SDL_Rect *clip) {
    SDL_Rect render_quad = {x, y, t->width, t->height};
    if (clip != NULL) {
        render_quad.w = clip->w;
        render_quad.h = clip->h;
    }
    SDL_RenderCopy(gRenderer, t->texture, clip, &render_quad);
}


uint32_t timer_get_ticks(Timer *t) {
    uint32_t time = 0;
    if (t->started) {
        time = SDL_GetTicks() - t->start_ticks;
    }
    return time;
}


void timer_start(Timer *t) {
    t->started = true;
    t->start_ticks = SDL_GetTicks();
}


void timer_stop(Timer *t) {
    t->started = false;
    t->start_ticks = 0;
}


int main(int argc, char *argv[]) {
    check(initialize(), "Failed to initialize");
    check(load_media(), "Failed to load media");
    bool quit = false;
    SDL_Event e;
    Timer cap_timer;

    while (!quit) {
        timer_start(&cap_timer);

        Piece current_piece = {
            L,
            1,
            1,
            0,
            0,
            {
                {1, 0, 0, 0},
                {1, 0, 0, 0},
                {1, 1, 0, 0},
                {0, 0, 0, 0}
            }
        };

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            /* handle events and collisions */
        }

        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);
        playfield_update(&current_piece);
        playfield_render();
        SDL_RenderPresent(gRenderer);

        /* cap frame rate */
        int frame_ticks = timer_get_ticks(&cap_timer);
        if (frame_ticks < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_ticks);
        }
    }

    close_all();
    return 0;

    error:
        close_all();
        return -1;
}
