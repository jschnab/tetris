#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.h"


#define SCREEN_FPS 10
#define SCREEN_TICKS_PER_FRAME (1000 / SCREEN_FPS)
#define PLAYFIELD_CELL_WIDTH 16
#define PLAYFIELD_CELL_HEIGHT 24
#define CELL_WIDTH 16
#define PLAYFIELD_WIDTH (PLAYFIELD_CELL_WIDTH * CELL_WIDTH)
#define PLAYFIELD_HEIGHT (PLAYFIELD_CELL_HEIGHT * CELL_WIDTH)
#define SCREEN_WIDTH (PLAYFIELD_WIDTH * 2.5)
#define SCREEN_HEIGHT (PLAYFIELD_HEIGHT * 1.1)
#define INFOFIELD_WIDTH PLAYFIELD_WIDTH
#define INFOFIELD_HEIGHT PLAYFIELD_HEIGHT
#define PLAYFIELD_POSITION_X (SCREEN_WIDTH / 2 - PLAYFIELD_WIDTH)
#define PLAYFIELD_POSITION_Y (SCREEN_HEIGHT / 2 - PLAYFIELD_HEIGHT / 2)
#define INFOFIELD_POSITION_X (SCREEN_WIDTH / 2 + INFOFIELD_WIDTH / 4) 
#define INFOFIELD_POSITION_Y (SCREEN_HEIGHT / 2 - INFOFIELD_HEIGHT / 2)
#define FONTSIZE 16
#define PIECE_VELOCITY 1
#define PIECE_MATRIX_WIDTH 4
#define PIECE_MATRIX_HEIGHT 4
#define NPIECES 7
#define FULL_ROWS_PER_LEVEL 12


typedef struct texture {
    SDL_Texture *texture;
    int width;
    int height;
} Texture;

typedef struct piece {
    int shape; /* shape name */
    int posx;  /* x-position of top left corner of matrix */
    int posy;  /* y-position of top left corner of matrix */
    int velx;  /* velocity along the x-axis */
    int vely;  /* velocity along the y-axis */
    int matrix[PIECE_MATRIX_HEIGHT][PIECE_MATRIX_WIDTH];  /* shape representation */
    bool landed;  /* if piece lands on bottom of playfield or another piece */
} Piece;

typedef struct timer {
    uint32_t start_ticks;
    bool started;
} Timer;


char *CELL_TILES = "cells.png";
char *CLEAR_ROW_ONE = "sounds/clear_one.wav";
char *CLEAR_ROW_TWO = "sounds/clear_two.wav";
char *CLEAR_ROW_THREE = "sounds/clear_three.wav";
char *CLEAR_ROW_FOUR = "sounds/clear_four.wav";
char *PIECE_LANDED = "sounds/landed.wav";
int POINTS[5] = {0, 50, 150, 350, 1000};
enum SHAPES {I = 1, J, L, O, S, T, Z};

SDL_Window *gWindow = NULL;
SDL_Renderer *gRenderer = NULL;
Texture gCellTexture = {NULL, 0, 0};
int gPlayfield[PLAYFIELD_CELL_HEIGHT][PLAYFIELD_CELL_WIDTH];
Mix_Chunk *gPieceLanded = NULL;
Mix_Chunk *gClearRowOne = NULL;
Mix_Chunk *gClearRowTwo = NULL;
Mix_Chunk *gClearRowThree = NULL;
Mix_Chunk *gClearRowFour = NULL;
TTF_Font *gFont = NULL;
Texture gScoreInfoTexture = {NULL, 0, 0};
Texture gLevelInfoTexture = {NULL, 0, 0};
Texture gTotalRowsInfoTexture = {NULL, 0, 0};


Piece piece_I = {
    I,
    0,
    0,
    0,
    0,
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0}
    }
};

Piece piece_J = {
    J,
    0,
    0,
    0,
    0,
    {
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    }
};

Piece piece_L = {
    L,
    0,
    0,
    0,
    0,
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    }
};

Piece piece_O = {
    O,
    0,
    0,
    0,
    0,
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    }
};

Piece piece_S = {
    S,
    0,
    0,
    0,
    0,
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0}
    }
};

Piece piece_T = {
    T,
    0,
    0,
    0,
    0,
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    }
};

Piece piece_Z = {
    Z,
    0,
    0,
    0,
    0,
    {
        {0, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    }
};


void close_all();
bool initialize();
uint32_t level_timer_ticks(int);
bool load_media();
bool piece_collided(Piece *);
void piece_handle_event(Piece *, SDL_Event);
void piece_move(Piece *);
bool piece_rotate_anticlock(Piece *);
bool piece_rotate_clock(Piece *);
Piece *piece_spawn();
void playfield_add_piece(Piece *);
int playfield_drop_full_rows();
void playfield_print();
void playfield_remove_piece(Piece *);
void playfield_render();
void texture_destroy(Texture *);
bool texture_from_file(Texture *, char *);
bool texture_from_text(Texture *, char *, SDL_Color);
void texture_render(Texture *, int, int, SDL_Rect *);
uint32_t timer_get_ticks(Timer *);
void timer_start(Timer *);
void timer_stop(Timer *);
bool update_level(int);
int update_score(int, int);


void close_all() {
    texture_destroy(&gCellTexture);
    TTF_CloseFont(gFont);
    gFont = NULL;
    Mix_FreeChunk(gPieceLanded);
    gPieceLanded = NULL;
    Mix_FreeChunk(gClearRowOne);
    gClearRowOne = NULL;
    Mix_FreeChunk(gClearRowTwo);
    gClearRowTwo = NULL;
    Mix_FreeChunk(gClearRowThree);
    gClearRowThree = NULL;
    Mix_FreeChunk(gClearRowFour);
    gClearRowFour = NULL;
    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}


bool initialize() {
    check(
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0,
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
    check(
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0,
        "SDL_mixer could not initialize: %s",
        Mix_GetError()
    );
    check(
        TTF_Init() == 0,
        "SDL_ttf failed to initialize: %s",
        TTF_GetError()
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


uint32_t level_timer_ticks(int level) {
    uint32_t ticks = 1000 - (level - 1) * 100;
    if (ticks < 0) {
        return 50;
    }
    return ticks;
}


bool load_media() {
    check(
        texture_from_file(&gCellTexture, CELL_TILES),
        "Failed to load cells texture"
    );
    gPieceLanded = Mix_LoadWAV(PIECE_LANDED);
    gClearRowOne = Mix_LoadWAV(CLEAR_ROW_ONE);
    gClearRowTwo = Mix_LoadWAV(CLEAR_ROW_TWO);
    gClearRowThree = Mix_LoadWAV(CLEAR_ROW_THREE);
    gClearRowFour = Mix_LoadWAV(CLEAR_ROW_FOUR);
    gFont = TTF_OpenFont("fonts/OpenSans-Regular.ttf", FONTSIZE);
    check_mem(gFont);
    return true;

    error:
        return false;
}


bool piece_collided(Piece *p) {
    for (int i = 0; i < PIECE_MATRIX_HEIGHT; i++) {
        for (int j = 0; j < PIECE_MATRIX_WIDTH; j++) {
            if (
                p->matrix[i][j] == 1 &&
                (
                    j + p->posx < 0 ||
                    j + p->posx >= PLAYFIELD_CELL_WIDTH ||
                    i + p->posy >= PLAYFIELD_CELL_HEIGHT ||
                    gPlayfield[i + p->posy][j + p->posx] != 0
                )
            ) {
                return true;
            }
        }
    }
    return false;
}


void piece_handle_event(Piece *p, SDL_Event e) {
    bool collided = false;
    /* if a key was pressed */
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        /* adjust velocity */
        switch (e.key.keysym.sym) {
            case SDLK_DOWN:
                p->vely += PIECE_VELOCITY;
                break;
            case SDLK_LEFT:
                p->velx -= PIECE_VELOCITY;
                break;
            case SDLK_RIGHT:
                p->velx += PIECE_VELOCITY;
                break;
            case SDLK_q:
                if ((collided = piece_rotate_anticlock(p))) {
                    piece_rotate_clock(p);
                }
                break;
            case SDLK_w:
                if ((collided = piece_rotate_clock(p))) {
                    piece_rotate_anticlock(p);
                }
                break;
        }
    }

    /* if a key was released */
    else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_DOWN:
                p->vely -= PIECE_VELOCITY;
                /* make sure that piece will not go up */
                if (p->vely < 0) {
                    p->vely = 0;
                }
                break;
            case SDLK_LEFT:
                p->velx += PIECE_VELOCITY;
                break;
            case SDLK_RIGHT:
                p->velx -= PIECE_VELOCITY;
                break;
        }
    }
}


void piece_move(Piece *p) {
    /* move the piece left or right */
    p->posx += p->velx;
    /* if the piece collided or went too far left or right, move back */
    if (piece_collided(p)) {
        p->posx -= p->velx;
    }
    /* move the piece down */
    p->posy += p->vely;
    /* if the piece collided or went too far down, move back and record it as
     * 'landed' (will be blocked and new piece will be spawned) */
    if (piece_collided(p)) {
        p->posy -= p->vely;
        p->landed = true;
        Mix_PlayChannel(-1, gPieceLanded, 0);
    }
}


Piece *piece_spawn(Piece *pieces) {
    int r = rand() % NPIECES;
    Piece *p = &pieces[r];
    p->posx = 6;
    p->posy = 0;
    p->velx = 0;
    p->vely = 0;
    p->landed = false;
    return p;
}


bool piece_rotate_anticlock(Piece *p) {
    int new_matrix[PIECE_MATRIX_HEIGHT][PIECE_MATRIX_WIDTH];
    for (int i = 0; i < PIECE_MATRIX_HEIGHT; i++) {
        for (int j = 0; j < PIECE_MATRIX_WIDTH; j++) {
            new_matrix[PIECE_MATRIX_WIDTH - 1 - j][i] = p->matrix[i][j];
        }
    }
    for (int i = 0; i < PIECE_MATRIX_HEIGHT; i++) {
        for (int j = 0; j < PIECE_MATRIX_WIDTH; j++) {
            p->matrix[i][j] = new_matrix[i][j];
        }
    }
    return piece_collided(p);
}


bool piece_rotate_clock(Piece *p) {
    int new_matrix[PIECE_MATRIX_HEIGHT][PIECE_MATRIX_WIDTH];
    for (int i = 0; i < PIECE_MATRIX_WIDTH; i++) {
        for (int j = 0; j < PIECE_MATRIX_HEIGHT; j++) {
            new_matrix[j][PIECE_MATRIX_WIDTH - 1 - i] = p->matrix[i][j];
        }
    }
    for (int i = 0; i < PIECE_MATRIX_HEIGHT; i++) {
        for (int j = 0; j < PIECE_MATRIX_WIDTH; j++) {
            p->matrix[i][j] = new_matrix[i][j];
        }
    }
    return piece_collided(p);
}


void playfield_add_piece(Piece *piece) {
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


int playfield_drop_full_rows() {
    bool flag = false;  /* true if full rows detected */
    int i, j, k;
    int nrows = 0;  /* number of full rows, for score keeping */

    /* full_rows: array to store index of full rows */
    int full_rows[PLAYFIELD_CELL_HEIGHT], ptr;
    for (ptr = 0; ptr < PLAYFIELD_CELL_HEIGHT; ptr++) {
        full_rows[ptr] = -99; /* sentinel value */
    }

    /* get index of full rows */
    ptr = 0;
    for (i = PLAYFIELD_CELL_HEIGHT - 1; i >= 0; i--) {
        for (j = 0; j < PLAYFIELD_CELL_WIDTH; j++) {
            if (gPlayfield[i][j] == 0) {
                break;
            }
        }
        /* if we arrived at the end of the row without breaking then this is a
         * full row */
        if (j == PLAYFIELD_CELL_WIDTH) {
            flag = true;
            full_rows[ptr++] = i;
        }
    }
    nrows = ptr;

    switch (nrows) {
        case 1:
            Mix_PlayChannel(-1, gClearRowOne, 0);
            break;
        case 2:
            Mix_PlayChannel(-1, gClearRowTwo, 0);
            break;
        case 3:
            Mix_PlayChannel(-1, gClearRowThree, 0);
            break;
        case 4:
            Mix_PlayChannel(-1, gClearRowFour, 0);
            break;
    }

    /* fill playfield and skip full rows if there are any */
    if (flag == false) {
        return nrows;
    }
    /*
    printf("there are full rows: ");
    for (ptr = 0; ptr < PLAYFIELD_CELL_HEIGHT; ptr++) {
        if (full_rows[ptr] == -99) {
            break;
        }
        printf("%d ", full_rows[ptr]);
    }
    printf("\n");
    */
    ptr = 0;
    k = PLAYFIELD_CELL_HEIGHT - 1;
    for (i = PLAYFIELD_CELL_HEIGHT - 1; i >= 0; i--, k--) {
        /* skip full rows */
        while (ptr < PLAYFIELD_CELL_HEIGHT && k == full_rows[ptr]) {
            k--;
            ptr++;
        }
        for (j = 0; j < PLAYFIELD_CELL_WIDTH; j++) {
            if (k < 0) {
                gPlayfield[i][j] = 0;
            }
            else {
                gPlayfield[i][j] = gPlayfield[k][j];
            }
        }
    }

    return nrows;
}


void playfield_print() {
    printf("\n");
    for (int i = 0; i < PLAYFIELD_CELL_HEIGHT; i++) {
        for (int j = 0; j < PLAYFIELD_CELL_WIDTH; j++) {
            printf("%d", gPlayfield[i][j]);
        }
        printf("\n");
    }
}


void playfield_remove_piece(Piece *piece) {
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
                gPlayfield[i][j] = 0;
            }
        }
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


bool texture_from_text(Texture *t, char *text, SDL_Color color) {
    texture_destroy(t);
    SDL_Surface *text_surface = TTF_RenderText_Solid(gFont, text, color);
    check_mem(text_surface);
    t->texture = SDL_CreateTextureFromSurface(gRenderer, text_surface);
    check_mem(t->texture);
    t->width = text_surface->w;
    t->height = text_surface->h;
    SDL_FreeSurface(text_surface);
    return true;

    error:
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


bool update_level(int total_rows) {
    if (total_rows < FULL_ROWS_PER_LEVEL) {
        return false;
    }
    return total_rows % FULL_ROWS_PER_LEVEL <= 3;
}


int update_score(int level, int nrows) {
    return level * POINTS[nrows];
}


int main(int argc, char *argv[]) {
    srand(time(NULL));
    check(initialize(), "Failed to initialize");
    check(load_media(), "Failed to load media");
    bool quit = false;
    SDL_Event e;
    Timer frame_timer;
    Timer game_timer;
    int score = 0;
    int level = 1;
    int nrows = 0;  /* number of full rows achieved when a piece lands */
    int total_rows = 0;  /* total number of full rows made in the game */
    char score_text[40];
    char level_text[40];
    char total_rows_text[40];
    SDL_Color text_color = {0, 0, 0, 255};
    Piece pieces[NPIECES] = {piece_I, piece_J, piece_L, piece_O, piece_S, piece_T, piece_Z};
    Piece *current_piece = piece_spawn(pieces);

    timer_start(&game_timer);

    while (!quit) {
        timer_start(&frame_timer);

        /* handle events and movements */
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            piece_handle_event(current_piece, e);
        }

        /* descend piece on playfield */
        if (timer_get_ticks(&game_timer) > level_timer_ticks(level)) {
            timer_start(&game_timer);
            /* descend only if piece is not already moving down */
            if (current_piece->vely == 0) {
                current_piece->vely += PIECE_VELOCITY;
                piece_move(current_piece);
                current_piece->vely -= PIECE_VELOCITY;
            }
        }

        piece_move(current_piece);

        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);

        /* update the playfield and draw it */
        playfield_add_piece(current_piece);
        playfield_render();
        if (current_piece->landed) {
            nrows = playfield_drop_full_rows();
            total_rows += nrows;
            score += update_score(level, nrows);
            if (nrows != 0 && update_level(total_rows)) {
                level++;
            }
            current_piece = piece_spawn(pieces);
        }
        else {
            playfield_remove_piece(current_piece);
        }

        /* print information (score, ...) */
        sprintf(score_text, "Score: %d", score);
        sprintf(level_text, "Level: %d", level);
        sprintf(total_rows_text, "Total rows: %d", total_rows);
        check(
            texture_from_text(&gScoreInfoTexture, score_text, text_color),
            "Failed to render score info texture"
        );
        check(
            texture_from_text(&gLevelInfoTexture, level_text, text_color),
            "Failed to render level info texture"
        );
        check(
            texture_from_text(&gTotalRowsInfoTexture, total_rows_text, text_color),
            "Failed to render total rows info texture"
        );
        texture_render(
            &gScoreInfoTexture,
            INFOFIELD_POSITION_X,
            INFOFIELD_POSITION_Y,
            NULL
        );
        texture_render(
            &gLevelInfoTexture,
            INFOFIELD_POSITION_X,
            INFOFIELD_POSITION_Y + FONTSIZE * 1.25,
            NULL
        );
        texture_render(
            &gTotalRowsInfoTexture,
            INFOFIELD_POSITION_X,
            INFOFIELD_POSITION_Y + 2 * FONTSIZE * 1.25,
            NULL
        );

        SDL_RenderPresent(gRenderer);

        /* cap frame rate */
        int frame_ticks = timer_get_ticks(&frame_timer);
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
