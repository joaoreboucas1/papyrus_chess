#include <stdio.h>
#include <raylib.h>

typedef enum {
    WH, BL, NONE
} Player;

typedef enum {
    PAWN, ROOK, BISHOP, KNIGHT, QUEEN, KING, EMPTY
} PieceType;

typedef enum {
    A, B, C, D, E, F, G, H
} Column;

typedef int Row;

typedef struct {
    PieceType type;
    Player player;
    Row row;
    Column col;
} Piece;

#define board_at(row, col) board[row - 1][col]
void initialize_board(Piece board[8][8])
{
    for (int row = 1; row <= 8; row++) {
        for (int col = A; col <= H; col++) {
            board_at(row, col) = (Piece) {.type = EMPTY, .player = NONE, .row = row, .col = col};
        }
    }
    for (int col = A; col <= H; col++) {
        board_at(2, col) = (Piece) {.type = PAWN, .player = WH, .row = 2, .col = col};
        board_at(7, col) = (Piece) {.type = PAWN, .player = BL, .row = 7, .col = col};
    }
    board_at(1, A) = (Piece) {.type = ROOK, .player = WH, .col = A, .row = 1};
    board_at(1, B) = (Piece) {.type = KNIGHT, .player = WH, .col = B, .row = 1};
    board_at(1, C) = (Piece) {.type = BISHOP, .player = WH, .col = C, .row = 1};
    board_at(1, D) = (Piece) {.type = QUEEN, .player = WH, .col = D, .row = 1};
    board_at(1, E) = (Piece) {.type = KING, .player = WH, .col = E, .row = 1};
    board_at(1, F) = (Piece) {.type = BISHOP, .player = WH, .col = F, .row = 1};
    board_at(1, G) = (Piece) {.type = KNIGHT, .player = WH, .col = G, .row = 1};
    board_at(1, H) = (Piece) {.type = ROOK, .player = WH, .col = H, .row = 1};

    board_at(8, A) = (Piece) {.type = ROOK, .player = BL, .col = A, .row = 1};
    board_at(8, B) = (Piece) {.type = KNIGHT, .player = BL, .col = B, .row = 1};
    board_at(8, C) = (Piece) {.type = BISHOP, .player = BL, .col = C, .row = 1};
    board_at(8, D) = (Piece) {.type = QUEEN, .player = BL, .col = D, .row = 1};
    board_at(8, E) = (Piece) {.type = KING, .player = BL, .col = E, .row = 1};
    board_at(8, F) = (Piece) {.type = BISHOP, .player = BL, .col = F, .row = 1};
    board_at(8, G) = (Piece) {.type = KNIGHT, .player = BL, .col = G, .row = 1};
    board_at(8, H) = (Piece) {.type = ROOK, .player = BL, .col = H, .row = 1};
}

int main(void)
{
    const int board_len = 800;
    const int padding = 400;
    const int screen_height = board_len;
    const int screen_width = board_len + padding;

    InitWindow(screen_width, screen_height, "Chess");
    SetTargetFPS(60);

    Piece board[8][8];
    initialize_board(board);
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(BROWN);
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    float square_len = board_len / 8;
                    int x = i * square_len;
                    int y = j * square_len;
                    Color square_color;
                    if ((i + j) % 2 == 0) square_color = BLACK; else square_color = WHITE;
                    DrawRectangle(x, y, square_len, square_len, square_color);
                }
            }
        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context
    return 0;
}