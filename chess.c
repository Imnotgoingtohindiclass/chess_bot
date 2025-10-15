#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

char currentPlayer = 'w';
bool isDragging = false;
char draggedPiece = ' ';
SDL_Point dragStartPosition = {-1, -1};
SDL_Point mousePosition = {0, 0};

bool wKingMoved = false;
bool bKingMoved = false;
bool wRookAMoved = false, wRookHMoved = false;
bool bRookAMoved = false, bRookHMoved = false;

bool init();
bool loadMedia();
void close();
void renderBoard();
void renderPieces();
void handleMouseDown(int x, int y);
void handleMouseMotion(int x, int y);
void handleMouseUp(int x, int y);
SDL_Texture* loadTexture(const char* path);
bool isValidMove(char boardState[8][8], char piece, int fromX, int fromY, int toX, int toY, char player);
bool isSquareAttacked(char boardState[8][8], int x, int y, char attackerColor);
bool isKingInCheck(char boardState[8][8], char kingColor);

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
            } else {
                switch (e.type) {
                    case SDL_MOUSEBUTTONDOWN:
                        handleMouseDown(e.button.x, e.button.y);
                        break;
                    case SDL_MOUSEMOTION:
                        handleMouseMotion(e.motion.x, e.motion.y);
                        break;
                    case SDL_MOUSEBUTTONUP:
                        handleMouseUp(e.button.x, e.button.y);
                        break;
                }
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
    if (isDragging && draggedPiece != ' ') {
        SDL_Rect destRect = {mousePosition.x - SQUARE_SIZE / 2, mousePosition.y - SQUARE_SIZE / 2, SQUARE_SIZE, SQUARE_SIZE};
        SDL_RenderCopy(gRenderer, gPieceTextures[(int)draggedPiece], NULL, &destRect);
    }
}

void handleMouseDown(int x, int y) {
    int boardX = x / SQUARE_SIZE;
    int boardY = y / SQUARE_SIZE;
    char piece = board[boardY][boardX];

    if (piece != ' ' && ((currentPlayer == 'w' && isupper(piece)) || (currentPlayer == 'b' && islower(piece)))) {
        isDragging = true;
        draggedPiece = piece;
        dragStartPosition.x = boardX; // start position
        dragStartPosition.y = boardY; // start position
        mousePosition.x = x;
        mousePosition.y = y;
        board[boardY][boardX] = ' ';
    }
}

void handleMouseMotion(int x, int y) {
    if (isDragging) {
        mousePosition.x = x;
        mousePosition.y = y;
    }
}

void handleMouseUp(int x, int y) {
    if (!isDragging) return;

    int toX = x / SQUARE_SIZE;
    int toY = y / SQUARE_SIZE;
    bool moveIsLegal = false;

    if (isValidMove(board, draggedPiece, dragStartPosition.x, dragStartPosition.y, toX, toY, currentPlayer)) {
        char tempBoard[8][8];
        memcpy(tempBoard, board, sizeof(board));
        tempBoard[toY][toX] = draggedPiece;
        tempBoard[dragStartPosition.y][dragStartPosition.x] = ' ';

        if (!isKingInCheck(tempBoard, currentPlayer)) {
            moveIsLegal = true;
        }
    }

    if (moveIsLegal) {
        board[toY][toX] = draggedPiece;

        if (toupper(draggedPiece) == 'K' && abs(toX - dragStartPosition.x) == 2) {
            if (toX == 6) { board[toY][5] = board[toY][7]; board[toY][7] = ' '; }
            else { board[toY][3] = board[toY][0]; board[toY][0] = ' '; }
        }

        // castling
        if (draggedPiece == 'K') wKingMoved = true;
        if (draggedPiece == 'k') bKingMoved = true;
        if (draggedPiece == 'R' && dragStartPosition.x == 0 && dragStartPosition.y == 7) wRookAMoved = true;
        if (draggedPiece == 'R' && dragStartPosition.x == 7 && dragStartPosition.y == 7) wRookHMoved = true;
        if (draggedPiece == 'r' && dragStartPosition.x == 0 && dragStartPosition.y == 0) bRookAMoved = true;
        if (draggedPiece == 'r' && dragStartPosition.x == 7 && dragStartPosition.y == 0) bRookHMoved = true;

        currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';
    } else {
        board[dragStartPosition.y][dragStartPosition.x] = draggedPiece;
    }

    isDragging = false;
    draggedPiece = ' ';
}

bool isValidMove(char boardState[8][8], char piece, int fromX, int fromY, int toX, int toY, char player) {
    if (fromX < 0 || fromX > 7 || fromY < 0 || fromY > 7 || toX < 0 || toX > 7 || toY < 0 || toY > 7) return false;
    if (fromX == toX && fromY == toY) return false;

    char destPiece = boardState[toY][toX];

    if (destPiece != ' ' && ((isupper(piece) && isupper(destPiece)) || (islower(piece) && islower(destPiece)))) return false;

    int dx = toX - fromX;
    int dy = toY - fromY;

    switch (toupper(piece)) {
        case 'P':
            if (player == 'w') {
                if (dx == 0 && dy == -1 && destPiece == ' ') return true;
                if (dx == 0 && dy == -2 && fromY == 6 && destPiece == ' ' && boardState[fromY - 1][fromX] == ' ') return true;

                if (abs(dx) == 1 && dy == -1 && destPiece != ' ' && islower(destPiece)) return true;
            } else {
                if (dx == 0 && dy == 1 && destPiece == ' ') return true;
                if (dx == 0 && dy == 2 && fromY == 1 && destPiece == ' ' && boardState[fromY + 1][fromX] == ' ') return true;

                if (abs(dx) == 1 && dy == 1 && destPiece != ' ' && isupper(destPiece)) return true;
            }
            return false;

        case 'N': return (abs(dx) == 1 && abs(dy) == 2) || (abs(dx) == 2 && abs(dy) == 1);

        case 'B':
            if (abs(dx) != abs(dy)) return false;
            int stepX_b = (dx > 0) ? 1 : -1;
            int stepY_b = (dy > 0) ? 1 : -1;
            for (int i = 1; i < abs(dx); ++i) {
                if (boardState[fromY + i * stepY_b][fromX + i * stepX_b] != ' ') return false;
            }
            return true;

        case 'R':
            if (dx != 0 && dy != 0) return false;
            if (dx == 0) {
                int stepY_r = (dy > 0) ? 1 : -1;
                for (int i = 1; i < abs(dy); ++i) {
                    if (boardState[fromY + i * stepY_r][fromX] != ' ') return false;
                }
            } else {
                int stepX_r = (dx > 0) ? 1 : -1;
                for (int i = 1; i < abs(dx); ++i) {
                    if (boardState[fromY][fromX + i * stepX_r] != ' ') return false;
                }
            }
            return true;

        case 'Q':
             if (abs(dx) != abs(dy) && dx != 0 && dy != 0) return false;

             int stepX_q = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
             int stepY_q = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);
             int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
             for(int i = 1; i < steps; ++i) if (boardState[fromY + i*stepY_q][fromX + i*stepX_q] != ' ') return false;
             return true;

        case 'K':
            if (abs(dx) <= 1 && abs(dy) <= 1) return true;

            // castling
            if (abs(dx) == 2 && dy == 0) {
                 if (isSquareAttacked(boardState, fromX, fromY, (player == 'w' ? 'b' : 'w'))) return false;

                 if (player == 'w') {
                     if (wKingMoved) return false;
                     if (dx == 2) {
                         if (wRookHMoved || boardState[7][5] != ' ' || boardState[7][6] != ' ') return false;
                         return !isSquareAttacked(boardState, 5, 7, 'b') && !isSquareAttacked(boardState, 6, 7, 'b');
                     } else {
                         if (wRookAMoved || boardState[7][1] != ' ' || boardState[7][2] != ' ' || boardState[7][3] != ' ') return false;
                         return !isSquareAttacked(boardState, 2, 7, 'b') && !isSquareAttacked(boardState, 3, 7, 'b');
                     }
                 } else {
                     if (bKingMoved) return false;
                     if (dx == 2) {
                         if (bRookHMoved || boardState[0][5] != ' ' || boardState[0][6] != ' ') return false;
                         return !isSquareAttacked(boardState, 5, 0, 'w') && !isSquareAttacked(boardState, 6, 0, 'w');
                     } else {
                         if (bRookAMoved || boardState[0][1] != ' ' || boardState[0][2] != ' ' || boardState[0][3] != ' ') return false;
                         return !isSquareAttacked(boardState, 2, 0, 'w') && !isSquareAttacked(boardState, 3, 0, 'w');
                     }
                 }
             }
             return false;
    }
    return false;
}

bool isSquareAttacked(char boardState[8][8], int x, int y, char attackerColor) {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = boardState[r][c];
            if (piece != ' ' && ((attackerColor == 'w' && isupper(piece)) || (attackerColor == 'b' && islower(piece)))) {
                if (toupper(piece) == 'P') {
                    int dy = y - r;
                    int dx = abs(x - c);
                    if (attackerColor == 'w' && dy == -1 && dx == 1) return true;
                    if (attackerColor == 'b' && dy == 1 && dx == 1) return true;
                } else if (isValidMove(boardState, piece, c, r, x, y, attackerColor)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool isKingInCheck(char boardState[8][8], char kingColor) {
    int kingX = -1, kingY = -1;
    char kingChar = (kingColor == 'w') ? 'K' : 'k';
    char attackerColor = (kingColor == 'w') ? 'b' : 'w';

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (boardState[r][c] == kingChar) {
                kingX = c;
                kingY = r;
                break;
            }
        }
    }
    if (kingX == -1) return false;

    return isSquareAttacked(boardState, kingX, kingY, attackerColor);
}