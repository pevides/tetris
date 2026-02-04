#include "tetris.h"

int main() {
    printf("Tetris\n");
    SDL_Init(SDL_INIT_TIMER);
    bool isRunning = true;
    SDL_Event ev;
    int level = 1;

    SDL_Window* window = SDL_CreateWindow("Tetris!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    //SDL_Surface* new_surface = surface;
    
    ShapeType type = getRandomType();

    Tetromino* fallingTetromino = createTetromino(type, (Point) {RIGHT_BORDER / CELL_WIDTH / 2, 0});
    uint32_t lastTime = SDL_GetTicks();
    float timer = 0.0f;
    float lockDelayTimer = 0.0f;
    bool startLockTimer = false;
    paintTetromino(fallingTetromino, surface, window);
    while (isRunning) {
        uint32_t frameStart = SDL_GetTicks();
        float deltaTime = (frameStart - lastTime) / 1000.0f;
        lastTime = frameStart;

        if (startLockTimer) {
            lockDelayTimer += deltaTime;
            if (lockDelayTimer * 1000 >= LOCK_DELAY) {
                ShapeType type = getRandomType();
                fallingTetromino = createTetromino(type, (Point) {RIGHT_BORDER / CELL_WIDTH / 2, 0});
                startLockTimer = false;
                lockDelayTimer = 0.0f;
                //surface = SDL_ConvertSurface(surface, surface->format, 0);
            }
        }
        while(SDL_PollEvent(&ev) != 0) {
            switch (ev.type) {
                case SDL_KEYDOWN:
                    moveTetromino(ev.key.keysym.sym, fallingTetromino);
                    break;
                case SDL_QUIT:
                    isRunning = false;
                    break;
            }
        }
        int frameTime = SDL_GetTicks() - frameStart;
        timer += deltaTime;
        
        if (timer >= getTimeToFall(level)) {
            fallTetromino(fallingTetromino);
            timer = 0.0f;            
        }
        if (hasLanded(fallingTetromino)) startLockTimer = true;

        //UPDATE SCREEN
        SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_FillRect(surface, &rect, 0x000000);
        paintTetromino(fallingTetromino, surface, window);

        if (FRAME_DELAY > frameTime) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
