#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 900
#define CELL_WIDTH 25
#define TARGET_FPS 60
#define FRAME_DELAY (1000 / TARGET_FPS)

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
    SHAPE_COUNT // Útil para saber quantas peças existem (para randomizar depois)
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

void fallPiece(Tetromino* t) {
    t->coordinates.y += CELL_WIDTH;
}

void moveTetromino(Sint32 key, Tetromino* t) {
    if (key == SDLK_UP) {
        const TetrominoDef *def = &SHAPE_REGISTRY[t->type];
        if (t->rotationIndex < def->maxRotations - 1) t->rotationIndex++;
        else t->rotationIndex = 0;
        t->shape = &def->rotations[t->rotationIndex];

    } else if (key == SDLK_DOWN) t->coordinates.y += CELL_WIDTH;
    else {
        if (key == SDLK_RIGHT) t->coordinates.x += CELL_WIDTH;
        if (key == SDLK_LEFT) t->coordinates.x -= CELL_WIDTH;
    }
}

int main() {
    printf("Tetris\n");
    SDL_Init(SDL_INIT_TIMER);
    bool isRunning = true;
    SDL_Event ev;
    int level = 1;

    SDL_Window* window = SDL_CreateWindow("Tetris!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Surface* surface = SDL_GetWindowSurface(window);    

    Tetromino* S = createTetromino(SHAPE_J, (Point) {0, 0});
    uint32_t lastTime = SDL_GetTicks();
    float timer = 0.0f;

    paintTetromino(S, surface, window);
    while (isRunning) {
        uint32_t frameStart = SDL_GetTicks();
        float deltaTime = (frameStart - lastTime) / 1000.0f;
        lastTime = frameStart;

        while(SDL_PollEvent(&ev) != 0) {
            switch (ev.type) {
                case SDL_KEYDOWN:
                    moveTetromino(ev.key.keysym.sym, S);
                    break;
                case SDL_QUIT:
                    isRunning = false;
                    break;
            }
        }
        int frameTime = SDL_GetTicks() - frameStart;
        timer += deltaTime;
        
        if (timer >= getTimeToFall(level)) {
            fallPiece(S);
            timer = 0.0f;            
        }

        //UPDATE SCREEN
        SDL_Rect rect = {0, 0, SCREEN_HEIGHT, SCREEN_WIDTH};
        SDL_FillRect(surface, &rect, 0x000000);
        paintTetromino(S, surface, window);

        if (FRAME_DELAY > frameTime) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
