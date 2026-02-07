#ifndef TETRIS_H
#define TETRIS_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>


#define CELL_WIDTH 25
#define BOARD_ROWS 20
#define BOARD_COLS 10
#define SCREEN_WIDTH (BOARD_COLS * CELL_WIDTH)
#define SCREEN_HEIGHT (BOARD_ROWS * CELL_WIDTH)
#define TARGET_FPS 60
#define FRAME_DELAY (1000 / TARGET_FPS)
#define LOCK_DELAY 500 //ms
#define ROWS_TO_LEVEL_UP 10

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
uint32_t board[BOARD_ROWS][BOARD_COLS] = {0};

enum { CELL_EMPTY = 0, CELL_SHAPE_OFFSET = 1 };

static inline bool isCellOccupied(int row, int col) {
    return board[row][col] != CELL_EMPTY;
}

static inline uint32_t toCellValue(ShapeType type) {
    return (uint32_t)type + CELL_SHAPE_OFFSET;
}

void shuffleBag() {
    for (int i = 0; i < SHAPE_COUNT; i++) {
        bag[i] = i;
    }
    srand(SDL_GetTicks());
    for (int i = SHAPE_COUNT - 1; i > 0; i--) {
        int j = rand() % SHAPE_COUNT;
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

void paintTetromino(Tetromino* tetromino, SDL_Surface* surface) {
    uint32_t color = SHAPE_REGISTRY[tetromino->type].color;

    for (int i = 0; i < 4; i++) {
        SDL_Rect rr = { 
            tetromino->coordinates.x + (*tetromino->shape)[i].x * CELL_WIDTH, 
            tetromino->coordinates.y + (*tetromino->shape)[i].y * CELL_WIDTH, 
            CELL_WIDTH, CELL_WIDTH
        };
        SDL_FillRect(surface, &rr, color);
    }
}

Tetromino* createTetromino(ShapeType type, Point coordinates) {
    Tetromino* t = malloc(sizeof(Tetromino));
    t->coordinates = coordinates;
    t->type = type;
    t->rotationIndex = 0;
    t->shape = &SHAPE_REGISTRY[type].rotations[0];

    return t;  
}

void lockIn(Tetromino* t) {
    int gridX = t->coordinates.x / CELL_WIDTH;
    int gridY = t->coordinates.y / CELL_WIDTH;
    int col, row;
    for (int i = 0; i < 4; i++) {
        col = (*t->shape)[i].x + gridX;
        row = (*t->shape)[i].y + gridY;
        board[row][col] = toCellValue(t->type);
    }
}

bool isValidMove(Tetromino* t) {
    Point coordinates = t->coordinates;
    const Point (*shape)[4] = t->shape;
    for (int i = 0; i < 4; i++) {
        int gridX = coordinates.x  / CELL_WIDTH + (*shape)[i].x;
        int gridY = coordinates.y  / CELL_WIDTH + (*shape)[i].y;

        if (gridX < 0 || gridX >= BOARD_COLS || gridY >= BOARD_ROWS || gridY < 0) return false;
        if (isCellOccupied(gridY, gridX)) return false;
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
    if (t->coordinates.y / CELL_WIDTH < BOARD_ROWS) t->coordinates.y += CELL_WIDTH;
    if (!isValidMove(t)) t->coordinates.y = oldY;
}

bool moveTetromino(Sint32 key, Tetromino* t) {

    int oldX = t->coordinates.x;
    int oldY = t->coordinates.y;
    int oldRot = t->rotationIndex;
    const Point (*oldShape)[4] = t->shape;

    bool hardDrop = false;

    if (key == SDLK_SPACE) {
        while (true) {
            t->coordinates.y += CELL_WIDTH;
            if (!isValidMove(t)) {
                t->coordinates.y -= CELL_WIDTH;
                break;
            }
        }
        hardDrop = true;
    } 
    else if (key == SDLK_UP) {
        const TetrominoDef *def = &SHAPE_REGISTRY[t->type];
        if (t->rotationIndex < def->maxRotations - 1) t->rotationIndex++;
        else t->rotationIndex = 0;
        t->shape = &def->rotations[t->rotationIndex];

    } else if (key == SDLK_DOWN) {
        t->coordinates.y += CELL_WIDTH;
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

    return hardDrop;
}
void clearRow(int rowToClear) {
    if (rowToClear > BOARD_ROWS) rowToClear = BOARD_ROWS - 1;

    for (int row = rowToClear; row > 0 ; row--) {
        for (int col = 0; col < BOARD_COLS; col++) {
            board[row][col] = board[row - 1][col];
        }
    }
    for (int col = 0; col < BOARD_COLS; col++) {
            board[0][col] = CELL_EMPTY;
        }
}
void paintBoard(SDL_Surface* surface) {
    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            uint32_t cell = board[row][col];
            if (cell == CELL_EMPTY) continue;

            ShapeType type = (ShapeType)(cell - CELL_SHAPE_OFFSET);
            uint32_t color = SHAPE_REGISTRY[type].color;
            SDL_Rect rr = {
                col * CELL_WIDTH,
                row * CELL_WIDTH,
                CELL_WIDTH,
                CELL_WIDTH
            };
            SDL_FillRect(surface, &rr, color);
        }
    }
}

void printBoard() {
    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            uint32_t cell = board[row][col];
            char ch = '.';
            if (cell != CELL_EMPTY) {
                ShapeType type = (ShapeType)(cell - CELL_SHAPE_OFFSET);
                switch (type) {
                    case SHAPE_I: ch = 'I'; break;
                    case SHAPE_J: ch = 'J'; break;
                    case SHAPE_L: ch = 'L'; break;
                    case SHAPE_O: ch = 'O'; break;
                    case SHAPE_S: ch = 'S'; break;
                    case SHAPE_Z: ch = 'Z'; break;
                    case SHAPE_T: ch = 'T'; break;
                    default: ch = '?'; break;
                }
            }
            putchar(ch);
        }
        putchar('\n');
    }
    putchar('\n');
    fflush(stdout);
}

int clearFullRows() {
    int numberOfRowsToClear = 0;
    for (int i = BOARD_ROWS - 1; i >= 0; i --) {
        bool rowIsFull = true;
        for (int j = 0; j < BOARD_COLS; j++) {
            if (!isCellOccupied(i, j)) {
                rowIsFull = false;
                break;
            }
        }
        if (rowIsFull) {
            clearRow(i);
            numberOfRowsToClear++;
        }
    }
    return numberOfRowsToClear;
}

#endif 