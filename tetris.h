#ifndef TETRIS_H
#define TETRIS_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>


#define CELL_WIDTH 25
#define SCREEN_WIDTH (10 * CELL_WIDTH)
#define SCREEN_HEIGHT (20 * CELL_WIDTH)
#define BOTTOM_BORDER SCREEN_HEIGHT
#define RIGHT_BORDER SCREEN_WIDTH
#define TARGET_FPS 60
#define FRAME_DELAY (1000 / TARGET_FPS)
#define LOCK_DELAY 500 //ms

#define I_COLOR 0x00FFFF
#define J_COLOR 0x0000FF
#define L_COLOR 0xFF7308
#define O_COLOR 0xFFFF00
#define S_COLOR 0x00FF00
#define Z_COLOR 0xFF0000
#define T_COLOR 0xFF00FF

typedef struct {
    int x;
    int y;
} Point;

typedef enum {
    SHAPE_I,
    SHAPE_J,
    SHAPE_L,
    SHAPE_O,
    SHAPE_S,
    SHAPE_Z,
    SHAPE_T,
    SHAPE_COUNT
} ShapeType;

typedef struct {
    const Point (*rotations)[4];
    int maxRotations;
    uint32_t color;
} TetrominoDef;

typedef struct {
    Point coordinates;
    const Point (*shape)[4];
    uint32_t color;
    int rotationIndex;
    ShapeType type;
} Tetromino;

static const Point L_SHAPES[4][4] = {

    { {0,0}, {0,1}, {1,0}, {2,0} },
    { {0,0}, {1,0}, {1,1}, {1,2} },
    { {0,0}, {1,0}, {2,0}, {2,-1} },
    { {0,0}, {0,1}, {0,2}, {1,2} }
};

static const Point J_SHAPES[4][4] = {
    { {0,0}, {1,0}, {0,1}, {0,2} },
    { {0,0}, {1,0}, {2,0}, {2,1} },
    { {0,0}, {-1,0}, {0,-1}, {0,-2} },
    { {0,0}, {0,1}, {1,1}, {2,1} }
};

static const Point I_SHAPES[2][4] = {
    { {0,0}, {0,1}, {0,2}, {0,3} },
    { {0,0}, {1,0}, {2,0}, {3,0} }
};

static const Point T_SHAPES[4][4] = {
    { {0,0}, {1,0}, {2,0}, {1,-1} },
    { {0,0}, {1,0}, {1,1}, {1,-1} },
    { {0,0}, {1,0}, {1,1}, {2,0} },
    { {0,0}, {0,1}, {1,1}, {0,2} }
};

static const Point S_SHAPES[2][4] = {
    { {0,0}, {1,0}, {1,-1}, {2,-1} },
    { {0,0}, {0,1}, {1,1}, {1,2} }
};

static const Point Z_SHAPES[2][4] = {
    { {0,0}, {1,0}, {1,1}, {2,1} },
    { {0,0}, {0,-1}, {1,-1}, {1,-2} }
};

static const Point O_SHAPES[1][4] = {
    { {0,0}, {1,0}, {1,1}, {0,1} },
};

const TetrominoDef SHAPE_REGISTRY[SHAPE_COUNT] = {
    [SHAPE_S] = { S_SHAPES, 2, S_COLOR },
    [SHAPE_L] = { L_SHAPES, 4, L_COLOR },
    [SHAPE_J] = { J_SHAPES, 4, J_COLOR },
    [SHAPE_T] = { T_SHAPES, 4, T_COLOR },
    [SHAPE_I] = { I_SHAPES, 2, I_COLOR },
    [SHAPE_O] = { O_SHAPES, 1, O_COLOR },
    [SHAPE_Z] = { Z_SHAPES, 2, Z_COLOR }
};

int bag[SHAPE_COUNT];
int bagIndex = SHAPE_COUNT;

void shuffleBag() {
    for (int i = 0; i < SHAPE_COUNT; i++) {
        bag[i] = i;
    }
    srand(SDL_GetTicks());
    for (int i = SHAPE_COUNT - 1; i > 0; i--) {
        int j = rand() % (SHAPE_COUNT + 1);
        int temp = bag[i];
        bag[i] = bag[j];
        bag[j] = temp;
    }
    bagIndex = 0;
}

ShapeType getRandomType() {
    if (bagIndex >= 7) shuffleBag();
    return (ShapeType)bag[bagIndex++];
}

double getTimeToFall(int level) {
    return pow((0.8 - ((level - 1) *0.007)), level -1);
}

void paintTetromino(Tetromino* tetromino, SDL_Surface* surface, SDL_Window *window) {
    uint32_t color = SHAPE_REGISTRY[tetromino->type].color;

    for (int i = 0; i < 4; i++) {
        SDL_Rect rr = { 
            tetromino->coordinates.x + (*tetromino->shape)[i].x * CELL_WIDTH, 
            tetromino->coordinates.y + (*tetromino->shape)[i].y * CELL_WIDTH, 
            CELL_WIDTH, CELL_WIDTH
        };
        SDL_FillRect(surface, &rr, color);
    }
    SDL_UpdateWindowSurface(window);
}

Tetromino* createTetromino(ShapeType type, Point coordinates) {
    Tetromino* t = malloc(sizeof(Tetromino));
    t->coordinates = coordinates;
    t->type = type;
    t->rotationIndex = 0;
    t->shape = &SHAPE_REGISTRY[type].rotations[0];

    return t;  
}

bool isValidMove(Tetromino* t) {
    Point coordinates = t->coordinates;
    const Point (*shape)[4] = t->shape;
    for (int i = 0; i < 4; i++) {
        int pixelX = coordinates.x + ((*shape)[i].x * CELL_WIDTH);
        int pixelY = coordinates.y + ((*shape)[i].y * CELL_WIDTH);

        if (pixelX < 0 || pixelX >= RIGHT_BORDER || pixelY >= BOTTOM_BORDER || pixelY < 0) return false; 
    }
    return true;
}

bool hasLanded(Tetromino* t) {
    t->coordinates.y += CELL_WIDTH;
    bool landed = !isValidMove(t);
    t->coordinates.y -= CELL_WIDTH;
    return landed;
}

void fallTetromino(Tetromino* t) {
    int oldY = t->coordinates.y;
    if (t->coordinates.y < BOTTOM_BORDER) t->coordinates.y += CELL_WIDTH;
    if (!isValidMove(t)) t->coordinates.y = oldY;
}

void moveTetromino(Sint32 key, Tetromino* t) {

    int oldX = t->coordinates.x;
    int oldY = t->coordinates.y;
    int oldRot = t->rotationIndex;
    const Point (*oldShape)[4] = t->shape;

    if (key == SDLK_SPACE) {
        while (true) {
            t->coordinates.y += CELL_WIDTH;
            if (!isValidMove(t)) {
                t->coordinates.y -= CELL_WIDTH;
                break;
            }
        }
    } 
    else if (key == SDLK_UP) {
        const TetrominoDef *def = &SHAPE_REGISTRY[t->type];
        if (t->rotationIndex < def->maxRotations - 1) t->rotationIndex++;
        else t->rotationIndex = 0;
        t->shape = &def->rotations[t->rotationIndex];

    } else if (key == SDLK_DOWN) {
        t->coordinates.y += CELL_WIDTH;
        if (!isValidMove(t)) t->coordinates.y -= CELL_WIDTH;
    }
    else {
        if (key == SDLK_RIGHT) t->coordinates.x += CELL_WIDTH;
        if (key == SDLK_LEFT) t->coordinates.x -= CELL_WIDTH;
    }

    if (!isValidMove(t)) {
         
        t->coordinates.x = oldX;
        t->coordinates.y = oldY;
        t->rotationIndex = oldRot;
        t->shape = oldShape;
    }
}

#endif 