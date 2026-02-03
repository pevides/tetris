#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 900
#define CELL_WIDTH 25
#define TARGET_FPS 60
#define FRAME_DELAY (1000 / TARGET_FPS)
#define S_COLOR 0x00FF00
#define T_COLOR 0xFF00FF
#define L_COLOR 0x0000FF
#define I_COLOR 0x00FFFF
#define O_COLOR 0xFFFF00



typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point coordinates;
    const Point (*shape)[4];
    uint32_t color;
} Tetromino;

static const Point L_SHAPES[8][4] = {
    { {0,0}, {1,0}, {0,1}, {0,2} },
    { {0,0}, {0,1}, {1,1}, {2,1} },
    { {0,0}, {1,0}, {2,0}, {2,1} },
    { {0,0}, {-1,0}, {0,-1}, {0,-2} },
    { {0,0}, {1,0}, {1,1}, {1,2} },
    { {0,0}, {1,0}, {2,0}, {2,-1} },
    { {0,0}, {0,1}, {1,0}, {2,0} },
    { {0,0}, {0,1}, {0,2}, {1,2} }
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

static const Point S_SHAPES[4][4] = {
    { {0,0}, {1,0}, {1,1}, {2,1} },
    { {0,0}, {0,-1}, {1,-1}, {1,-2} },
    { {0,0}, {1,0}, {1,-1}, {2,-1} },
    { {0,0}, {0,1}, {1,1}, {1,2} }
};
static const Point O_SHAPES[1][4] = {
    { {0,0}, {1,0}, {1,1}, {0,1} },
};

double getTimeToFall(int level) {
    return pow((0.8 - ((level - 1) *0.007)), level -1);
}

void paintTetromino(Tetromino tetromino, SDL_Surface* surface, SDL_Window *window) {
    for (int i = 0; i < 4; i++) {
        SDL_Rect rr = { tetromino.coordinates.x + (*tetromino.shape)[i].x * CELL_WIDTH, tetromino.coordinates.y + (*tetromino.shape)[i].y * CELL_WIDTH, CELL_WIDTH, CELL_WIDTH};
        SDL_FillRect(surface, &rr, tetromino.color);
    }
    SDL_UpdateWindowSurface(window);
}

Tetromino* createTetromino(char T, int rotation, Point coordinates) {
    Tetromino* t = malloc(sizeof(Tetromino));
    switch(T) {
        case 'S':
            *t = (Tetromino){
                {coordinates.x, coordinates.y},
                &S_SHAPES[rotation],
                S_COLOR
            };
            break;
        case 'L':
            *t = (Tetromino){
                {coordinates.x, coordinates.y},
                &L_SHAPES[rotation],
                L_COLOR
            };
            break;
        case 'T':
            *t = (Tetromino){
                {coordinates.x, coordinates.y},
                &T_SHAPES[rotation],
                T_COLOR
            };
            break;
        case 'I':
            *t = (Tetromino){
                {coordinates.x, coordinates.y},
                &I_SHAPES[rotation],
                I_COLOR
            };
            break;
        case 'O':
            *t = (Tetromino){
                {coordinates.x, coordinates.y},
                &O_SHAPES[rotation],
                O_COLOR
            };
            break;
    }
    return t;
}
void fallPiece() {
    printf("CU\n");
}

int main() {
    printf("Tetris\n");
    SDL_Init(SDL_INIT_TIMER);
    bool isRunning = true;
    SDL_Event ev;
    int level = 1;

    SDL_Window* window = SDL_CreateWindow("Tetris!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Surface* surface = SDL_GetWindowSurface(window);    

    Tetromino* S = createTetromino('S', 0, (Point){0, 0});
    uint32_t lastTime = SDL_GetTicks();
    float timer = 0.0f;

    paintTetromino(*S, surface, window);
    while (isRunning) {
        uint32_t frameStart = SDL_GetTicks();
        float deltaTime = (frameStart - lastTime) / 1000.0f;
        lastTime = frameStart;

        while(SDL_PollEvent(&ev) != 0) {
            switch (ev.type) {
                case SDL_QUIT:
                    isRunning = false;
                    break;
            }
        }
        int frameTime = SDL_GetTicks() - frameStart;
        timer += deltaTime;
        if (timer >= getTimeToFall(level)) {
            fallPiece();
            timer = 0.0f;
        }
        if (FRAME_DELAY > frameTime) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
