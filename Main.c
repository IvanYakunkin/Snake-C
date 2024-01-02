#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "SDL2-2.28.5/include/SDL.h"
#undef main

const int SCREEN_WIDTH = 681;
const int SCREEN_HEIGHT = 482; 
const int BLOCK_SIZE = 40;
const int SPEED = 2;
const int FPS = 7;
const int DEFAULT_SNAKE_POSITION[2] = { 4, 4 };
const int DEFAULT_SNAKE_MOVEMENT_DIRECTION_X = 1;
const int DEFAULT_SNAKE_MOVEMENT_DIRECTION_Y = 0;
// Delay after arrow button click 
const Uint32 ARROW_DELAY = 70;

void draw_grid(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    // Vertical lines
    for(int i = 0; i < SCREEN_WIDTH / BLOCK_SIZE + 1; i++){
        SDL_RenderDrawLine(renderer, i* BLOCK_SIZE, 0, i* BLOCK_SIZE, 480);
    }

    // Horizontal lines
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        SDL_RenderDrawLine(renderer, 0, i * BLOCK_SIZE, SCREEN_WIDTH, i * BLOCK_SIZE);
    }
}

void draw_snake(SDL_Renderer* renderer, int** snake, int snake_size, int xMoveFlag, int yMoveFlag) {

    SDL_Rect* snake_rects = malloc(snake_size * sizeof(SDL_Rect));

    for (int block = 0; block < snake_size; ++block) {
        snake_rects[block].x = BLOCK_SIZE * snake[block][0] + 1;
        snake_rects[block].y = BLOCK_SIZE * snake[block][1] + 1;
        snake_rects[block].w = BLOCK_SIZE - 1;
        snake_rects[block].h = BLOCK_SIZE - 1;
    }

    SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);
    for (int snake_el = 0; snake_el < snake_size; snake_el++) {
        SDL_RenderFillRect(renderer, &snake_rects[snake_el]);
    }
    
    int left_eye_pos_x;
    int left_eye_pos_y;
    int right_eye_pos_x;
    int right_eye_pos_y;
    
    if (xMoveFlag == 1) {
        left_eye_pos_x = 30;
        right_eye_pos_x = 30;
        left_eye_pos_y = 10;
        right_eye_pos_y = 23;
    }
    else if(xMoveFlag == -1) {
        left_eye_pos_x = 5;
        right_eye_pos_x = 5;
        left_eye_pos_y = 10;
        right_eye_pos_y = 23;
    }
    else if (yMoveFlag == 1) {
        left_eye_pos_x = 10;
        right_eye_pos_x = 26;
        left_eye_pos_y = 30;
        right_eye_pos_y = 30;
    }
    else if (yMoveFlag == -1) {
        left_eye_pos_x = 10;
        right_eye_pos_x = 26;
        left_eye_pos_y = 5;
        right_eye_pos_y = 5;
    }

    SDL_Rect left_eye = { snake_rects[0].x + left_eye_pos_x, snake_rects[0].y + left_eye_pos_y, 5, 5 };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &left_eye);
    SDL_Rect right_eye = { snake_rects[0].x + right_eye_pos_x, snake_rects[0].y + right_eye_pos_y, 5, 5 };
    SDL_RenderFillRect(renderer, &right_eye);

    free(snake_rects);

}

void move_snake(SDL_Renderer *renderer, int **snake, int snake_size, int xMoveFlag, int yMoveFlag) {
    
    // Head movement
    int pastHeadPos[2] = { snake[0][0], snake[0][1] };
    if (xMoveFlag != 0) {
        snake[0][0] += 1 * xMoveFlag;
    }
    else {
        snake[0][1] += 1 * yMoveFlag;
    }

    // Body movement
    if (snake_size > 1) {
        for (int body_el = 1; body_el < snake_size; body_el++) {
            int tempX = snake[body_el][0],
                tempY = snake[body_el][1];

            snake[body_el][0] = pastHeadPos[0];
            snake[body_el][1] = pastHeadPos[1];
            pastHeadPos[0] = tempX;
            pastHeadPos[1] = tempY;
        }
    }
}

// Generate and return random apple coords
int* setApple() {
    srand(time(NULL));
    static int apple[2];
    int min = 1;
    int max_x = SCREEN_WIDTH / BLOCK_SIZE - 1;
    int max_y = SCREEN_HEIGHT / BLOCK_SIZE - 1;
    apple[0] = min + rand() % (max_x - min + 1);
    apple[1] = min + rand() % (max_y - min + 1);
    

    return apple;
}


void draw_apple(SDL_Renderer* renderer, int* apple) {
    SDL_Rect apple_rect = {apple[0] * BLOCK_SIZE + 1, apple[1] * BLOCK_SIZE + 1, BLOCK_SIZE-1, BLOCK_SIZE - 1};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &apple_rect);
}

bool check_eating(int** snake, int* apple) {
    if (snake[0][0] == apple[0] && snake[0][1] == apple[1]) {
        return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Square", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Set snake body-array
    int snake_size = 1;
    int** snake = (int**)malloc(snake_size * sizeof(int*));

    // Set default position for head
    for (int i = 0; i < snake_size; i++) {
        snake[i] = (int*)malloc(2 * sizeof(int));
    }
 
    snake[0][0] = DEFAULT_SNAKE_POSITION[0];
    snake[0][1] = DEFAULT_SNAKE_POSITION[1];

    // Apple coords
    int *apple = setApple();

    // Flag for lose
    bool isLose = false;
    // Movement position
    int xMoveFlag = DEFAULT_SNAKE_MOVEMENT_DIRECTION_X;  
    int yMoveFlag = DEFAULT_SNAKE_MOVEMENT_DIRECTION_Y;

    const int frameDelay = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;
    bool quit = false;
    SDL_Event event;

    // Delay properties for arrow buttons
    Uint32 lastTurnTime = 0;
    

    while (!quit) {
        frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            else if (event.type == SDL_KEYDOWN) {
                Uint32 currentTime = SDL_GetTicks();
                switch (event.key.keysym.sym) {   
                case SDLK_LEFT:
                    if (currentTime - lastTurnTime >= ARROW_DELAY) {
                        lastTurnTime = currentTime;
                        if (xMoveFlag != 1 || snake_size == 1) {
                            xMoveFlag = -1;
                            yMoveFlag = 0;
                        }
                        break;
                    }
                case SDLK_RIGHT:
                    if (currentTime - lastTurnTime >= ARROW_DELAY) {
                        lastTurnTime = currentTime;
                        if (xMoveFlag != -1 || snake_size == 1) {
                            xMoveFlag = 1;
                            yMoveFlag = 0;
                        }
                    }
                    break;
                case SDLK_UP:
                    if (currentTime - lastTurnTime >= ARROW_DELAY) {
                        lastTurnTime = currentTime;
                        if (yMoveFlag != 1 || snake_size == 1) {
                            yMoveFlag = -1;
                            xMoveFlag = 0;
                        }
                        break;
                    }
                case SDLK_DOWN:
                    if (currentTime - lastTurnTime >= ARROW_DELAY) {
                        lastTurnTime = currentTime;
                        if (yMoveFlag != -1 || snake_size == 1) {
                            yMoveFlag = 1;
                            xMoveFlag = 0;
                        }
                        break;
                    }
                }
            } 
        }
        
        SDL_SetRenderDrawColor(renderer, 170, 215, 81, 255);
        SDL_RenderClear(renderer);

        //draw_grid(renderer);
        // Check the edges
        if (snake[0][0] < 0) {
            snake[0][0] = SCREEN_WIDTH / BLOCK_SIZE - 1;
        }
        else if (snake[0][0] > SCREEN_WIDTH / BLOCK_SIZE - 1) {
            
            snake[0][0] = 0;
        }
        else if (snake[0][1] > SCREEN_HEIGHT / BLOCK_SIZE - 1) {
            snake[0][1] = 0;
        }
        else if (snake[0][1] < 0) {
            snake[0][1] = SCREEN_HEIGHT / BLOCK_SIZE - 1;
        }
        draw_snake(renderer, snake, snake_size, xMoveFlag, yMoveFlag);
        draw_apple(renderer, apple);

        // Check Collision
        for (int i = 1; i < snake_size; i++) {
            if (snake[0][0] == snake[i][0] && snake[0][1] == snake[i][1]) {
                isLose = true;
            }
        }

        if (!isLose) {
            move_snake(renderer, snake, snake_size, xMoveFlag, yMoveFlag);
        }

        if (check_eating(snake, apple)) {
            // Increase the size of the snake
            snake_size++;
            snake = (int**)realloc(snake, snake_size * sizeof(int*));
            snake[snake_size - 1] = (int*)malloc(2 * sizeof(int));
            snake[snake_size - 1][0] = snake[snake_size-1];
            snake[snake_size - 1][1] = snake[snake_size - 1];
            apple = setApple();
        }
        
        SDL_RenderPresent(renderer);
        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }
    free(snake);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
