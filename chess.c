#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;
const int SQUARE_SIZE = SCREEN_WIDTH / 8;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gPieceTextures[128] = {NULL};

char board[8][8] = {
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
};

bool init();
bool loadMedia();
void close();
void renderBoard();
void renderPieces();
SDL_Texture* loadTexture(const char* path);

int main(int argc, char* args[]) {
    if (!init()) {
        printf("Failed to initialize!\n");
        return 1;
    }
    if (!loadMedia()) {
        printf("Failed to load media!\n");
        return 1;
    }

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);
        renderBoard();
        renderPieces();
        SDL_RenderPresent(gRenderer);
    }

    close();
    return 0;
}


bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    gWindow = SDL_CreateWindow("chess board", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!gWindow) return false;
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!gRenderer) return false;
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) return false;
    return true;
}

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (!loadedSurface) return NULL;
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return newTexture;
}


void close() {
    for (int i = 0; i < 128; ++i) {
        if (gPieceTextures[i]) SDL_DestroyTexture(gPieceTextures[i]);
    }
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    IMG_Quit();
    SDL_Quit();
}

void renderBoard() {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            SDL_Rect squareRect = {c * SQUARE_SIZE, r * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
            if ((r + c) % 2 == 0) SDL_SetRenderDrawColor(gRenderer, 238, 238, 210, 255);
            else SDL_SetRenderDrawColor(gRenderer, 118, 150, 86, 255);
            SDL_RenderFillRect(gRenderer, &squareRect);
        }
    }
}

bool loadMedia() {
    const char* pieces[] = {"wP", "wR", "wN", "wB", "wQ", "wK", "bP", "bR", "bN", "bB", "bQ", "bK"};
    const char ids[] = {'P', 'R', 'N', 'B', 'Q', 'K', 'p', 'r', 'n', 'b', 'q', 'k'};
    for (int i = 0; i < 12; ++i) {
        char path[256];
        sprintf(path, "img/%s.png", pieces[i]);
        gPieceTextures[(int)ids[i]] = loadTexture(path);
        if (!gPieceTextures[(int)ids[i]]) return false;
    }
    return true;
}

void renderPieces() {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = board[r][c];
            if (piece != ' ') {
                SDL_Rect destRect = {c * SQUARE_SIZE, r * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                SDL_RenderCopy(gRenderer, gPieceTextures[(int)piece], NULL, &destRect);
            }
        }
    }
}