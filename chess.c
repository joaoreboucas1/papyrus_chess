#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>

#define BOARD_SIZE 800
#define SQUARE_SIZE (BOARD_SIZE/8)
#define SCREEN_HORIZ_PAD 400
#define SCREEN_HEIGHT BOARD_SIZE
#define SCREEN_WIDTH (BOARD_SIZE + SCREEN_HORIZ_PAD)
#define POSSIBLE_MOVES_CAP 21

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
    Row row;
    Column col;
} Square;

typedef struct {
    PieceType type;
    Player player;
    Row row;
    Column col;
    bool selected;
} Piece;

#define board_at(row, col) board[row - 1][col]
void initialize_board(Piece board[8][8])
{
    for (Row row = 1; row <= 8; row++) {
        for (Column col = A; col <= H; col++) {
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

void DrawBackground()
{
    ClearBackground(BROWN);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float square_size = BOARD_SIZE / 8;
            int x = i * square_size;
            int y = j * square_size;
            Color square_color;
            if ((i + j) % 2 == 0) square_color = BLACK; else square_color = WHITE;
            DrawRectangle(x, y, square_size, square_size, square_color);
        }
    }
}

void DrawPieces(Piece board[8][8], Texture2D texture)
{
    int pad_x;
    int pad_y;
    Rectangle rec;
    Vector2 pos;
    for (Row row = 1; row <= 8; row++) {
        for (Column col = A; col <= H; col++) {
            Piece piece = board_at(row, col);
            if (piece.type == EMPTY) continue;
            if (piece.type == ROOK) {
                rec = (Rectangle) {
                    .x = 3,
                    .y = 179,
                    .width = 47,
                    .height = 243 - 179
                };
                pad_x = 27;
                pad_y = 22;
            } else if (piece.type == BISHOP) {
                rec = (Rectangle) {
                    .x = 139,
                    .y = 177,
                    .width = 70,
                    .height = 250 - 179
                };
                pad_x = 15;
                pad_y = 18;
            } else if (piece.type == KNIGHT) {
                rec = (Rectangle) {
                    .x = 364,
                    .y = 181,
                    .width = 65,
                    .height = 65
                };
                pad_x = 15;
                pad_y = 18;
            } else if (piece.type == QUEEN) {
                rec = (Rectangle) {
                    .x = 286,
                    .y = 180,
                    .width = 70,
                    .height = 65
                };
                pad_x = 15;
                pad_y = 18;
            } else if (piece.type == KING) {
                rec = (Rectangle) {
                    .x = 217,
                    .y = 178,
                    .width = 67,
                    .height = 62
                };
                pad_x = 15;
                pad_y = 18;
            } else if (piece.type == PAWN) {
                rec = (Rectangle) {
                    .x = 452,
                    .y = 180,
                    .width = 40,
                    .height = 70
                };
                pad_x = 31;
                pad_y = 16;
            }
            if (piece.player == BL) rec.y += 83;
            pad_y = -pad_y;
            if (piece.selected) {
                pos = GetMousePosition();
            } else {
                pos = (Vector2) {.x = col * BOARD_SIZE / 8 + pad_x, .y = SCREEN_HEIGHT - ((row) * BOARD_SIZE / 8 + pad_y)};
            }
            DrawTextureRec(texture, rec, pos, WHITE);
        }
    }

}

void calculate_possible_moves(Piece p, Piece board[8][8], Square *possible_moves, int *count)
{
    if (p.type == PAWN) {
        int direction = (p.player == WH) ? 1 : -1;
        Row starting_row = (p.player == WH) ? 2 : 7;
        if (p.row == 1 || p.row == 8) return;
        if (board_at(p.row + direction, p.col).type != EMPTY) return;
        possible_moves[*count] = (Square) {.row = p.row + direction, .col = p.col};
        *count += 1;
        if (board_at(p.row + 2*direction, p.col).type != EMPTY || p.row != starting_row) return;
        possible_moves[*count] = (Square) {.row = p.row + 2*direction, .col = p.col};
        *count += 1;
    } else {
        printf("Not implemented\n");
    }
    
}

void DrawPossibleMoves(Square possible_moves[POSSIBLE_MOVES_CAP], int possible_moves_count)
{
    const float r = 10.0f;
    for (int i = 0; i < possible_moves_count; i++) {
        int x = possible_moves[i].col * SQUARE_SIZE + SQUARE_SIZE/2;
        int y = BOARD_SIZE - (possible_moves[i].row - 1) * SQUARE_SIZE - SQUARE_SIZE/2;
        DrawCircle(x, y, r, GRAY);
    }
}


int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess");
    // TODO: figure out icon format for SetWindowIcon(Image image)
    SetTargetFPS(60);

    
    Texture2D piece_texture = LoadTexture("pieces-removebg-preview.png");
    int x = 1000;
    int y = 100;
    Piece board[8][8];
    Row target_row, sel_piece_row = 0;
    Column target_col, sel_piece_col = 0;
    bool selected_piece = false;
    Square possible_moves[POSSIBLE_MOVES_CAP];
    int possible_moves_count = 0;


    initialize_board(board);
    
    while (!WindowShouldClose())
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !selected_piece) {
            Vector2 mouse_pos = GetMousePosition();
            sel_piece_col = ((int) mouse_pos.x) / SQUARE_SIZE;
            sel_piece_row = 8 - ((int) mouse_pos.y) / SQUARE_SIZE;
            if (sel_piece_col >= 0 && sel_piece_col < 8 && sel_piece_row > 0 && sel_piece_row <= 8 && board_at(sel_piece_row, sel_piece_col).type != EMPTY) {
                selected_piece = true;
                board_at(sel_piece_row, sel_piece_col).selected = true;
                calculate_possible_moves(board_at(sel_piece_row, sel_piece_col), board, possible_moves, &possible_moves_count);
            }
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && selected_piece) {
            Vector2 mouse_pos = GetMousePosition();
            target_col = ((int) mouse_pos.x) / SQUARE_SIZE;
            target_row = 8 - ((int) mouse_pos.y) / SQUARE_SIZE;
            if (target_col >= 0 && target_col < 8 && target_row > 0 && target_row <= 8 && board_at(target_row, target_col).type == EMPTY) {
                board_at(target_row, target_col) = board_at(sel_piece_row, sel_piece_col);
                board_at(target_row, target_col).selected = false;
                board_at(sel_piece_row, sel_piece_col) = (Piece) {.type = EMPTY, .player = NONE, .row = sel_piece_row, .col = sel_piece_col, .selected = false};
            } else {
                board_at(sel_piece_row, sel_piece_col).selected = false;
            }
            selected_piece = false;
            possible_moves_count = 0;
        }

        BeginDrawing();
            DrawBackground();
            DrawPieces(board, piece_texture);
            if (selected_piece && possible_moves_count > 0) DrawPossibleMoves(possible_moves, possible_moves_count);
            // TODO: render possible moves
        EndDrawing();
    }

    CloseWindow();
    return 0;
}