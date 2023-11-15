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
    A = 1, B, C, D, E, F, G, H
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

#define board_at(row, col) board[row - 1][col - 1]
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

    board_at(8, A) = (Piece) {.type = ROOK, .player = BL, .col = A, .row = 8};
    board_at(8, B) = (Piece) {.type = KNIGHT, .player = BL, .col = B, .row = 8};
    board_at(8, C) = (Piece) {.type = BISHOP, .player = BL, .col = C, .row = 8};
    board_at(8, D) = (Piece) {.type = QUEEN, .player = BL, .col = D, .row = 8};
    board_at(8, E) = (Piece) {.type = KING, .player = BL, .col = E, .row = 8};
    board_at(8, F) = (Piece) {.type = BISHOP, .player = BL, .col = F, .row = 8};
    board_at(8, G) = (Piece) {.type = KNIGHT, .player = BL, .col = G, .row = 8};
    board_at(8, H) = (Piece) {.type = ROOK, .player = BL, .col = H, .row = 8};
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
                pos = (Vector2) {.x = (col - 1) * BOARD_SIZE / 8 + pad_x, .y = SCREEN_HEIGHT - (row * BOARD_SIZE / 8 + pad_y)};
            }
            DrawTextureRec(texture, rec, pos, WHITE);
        }
    }
}

void calculate_possible_moves(Piece p, Piece board[8][8], Square *possible_moves, int *count)
{
    Row r;
    Column c;
    if (p.type == PAWN) {
        int direction = (p.player == WH) ? 1 : -1;
        Row starting_row = (p.player == WH) ? 2 : 7;
        if (p.row == 1 || p.row == 8) return;
        if (board_at(p.row + direction, p.col).type != EMPTY) return;
        possible_moves[*count] = (Square) {.row = p.row + direction, .col = p.col};
        (*count)++;
        if (board_at(p.row + 2*direction, p.col).type != EMPTY || p.row != starting_row) return;
        possible_moves[*count] = (Square) {.row = p.row + 2*direction, .col = p.col};
        (*count)++;
    } else if (p.type == KNIGHT) {
        // NOTE: the enum literals are of type unsigned int. If you have Column c = 0; and you decrement its value, it goes to the max value
        // Therefore, before comparing them to the minimum values, if they might be negative, you must cast to int before the comparison
        if (p.row + 2 <= 8 && p.col + 1 <= H && board_at(p.row + 2, p.col + 1).type == EMPTY) {
            possible_moves[*count] = (Square) {.row = p.row + 2, .col = p.col + 1};
            (*count)++;
        }
        if (p.row + 2 <= 8 && (int) p.col - 1 >= A && board_at(p.row + 2, p.col - 1).type == EMPTY) {
            possible_moves[*count] = (Square) {.row = p.row + 2, .col = p.col - 1};
            (*count)++;
        }
        if ((int) p.row - 2 >= 1 && p.col + 1 <= H && board_at(p.row - 2, p.col + 1).type == EMPTY) {
            possible_moves[*count] = (Square) {.row = p.row - 2, .col = p.col + 1};
            (*count)++;
        }
        if ((int) p.row - 2 >= 1 && (int) p.col - 1 >= A && board_at(p.row - 2, p.col - 1).type == EMPTY) {
            possible_moves[*count] = (Square) {.row = p.row - 2, .col = p.col - 1};
            (*count)++;
        }
        if (p.row + 1 <= 8 && p.col + 2 <= H && board_at(p.row + 1, p.col + 2).type == EMPTY) {
            possible_moves[*count] = (Square) {.row = p.row + 1, .col = p.col + 2};
            (*count)++;
        }
        if (p.row + 1 <= 8 && (int) p.col - 2 >= A && board_at(p.row + 1, p.col - 2).type == EMPTY) {
            possible_moves[*count] = (Square) {.row = p.row + 1, .col = p.col - 2};
            (*count)++;
        }
        if ((int) p.row - 1 >= 1 && p.col + 2 <= H && board_at(p.row - 1, p.col + 2).type == EMPTY) {
            possible_moves[*count] = (Square) {.row = p.row - 1, .col = p.col + 2};
            (*count)++;
        }
        if ((int) p.row - 1 >= 1 && (int) p.col - 2 >= A && board_at(p.row - 1, p.col - 2).type == EMPTY) {
            possible_moves[*count] = (Square) {.row = p.row - 1, .col = p.col - 2};
            (*count)++;
        }
    } else if (p.type == BISHOP) {
        for (int i = 1; i < 8; i++) {
            r = p.row + i;
            c = p.col + i;
            if (r > 8 || c > H) break;
            if (board_at(r, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = c};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            r = p.row - i;
            c = p.col + i;
            if (r < 1 || c > H) break;
            if (board_at(r, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = c};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            r = p.row + i;
            c = p.col - i;
            if (r > 8 || c < A) break;
            if (board_at(r, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = c};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            r = p.row - i;
            c = p.col - i;
            if (r < 1 || c < A) break;
            if (board_at(r, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = c};
            (*count)++;
        }
    } else if (p.type == ROOK) {
        for (int i = 1; i < 8; i++) {
            r = p.row + i;
            if (r > 8) break;
            if (board_at(r, p.col).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = p.col};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            r = p.row - i;
            if (r < 1) break;
            if (board_at(r, p.col).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = p.col};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            c = p.col + i;
            if (c > H) break;
            if (board_at(p.row, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = p.row, .col = c};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            c = p.col - i;
            if (c < A) break;
            if (board_at(p.row, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = p.row, .col = c};
            (*count)++;
        }        
    } else if (p.type == QUEEN) {
        for (int i = 1; i < 8; i++) {
            r = p.row + i;
            c = p.col + i;
            if (r > 8 || c > H) break;
            if (board_at(r, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = c};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            r = p.row - i;
            c = p.col + i;
            if (r < 1 || c > H) break;
            if (board_at(r, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = c};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            r = p.row + i;
            c = p.col - i;
            if (r > 8 || c < A) break;
            if (board_at(r, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = c};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            r = p.row - i;
            c = p.col - i;
            if (r < 1 || c < A) break;
            if (board_at(r, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = c};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            r = p.row + i;
            if (r > 8) break;
            if (board_at(r, p.col).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = p.col};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            r = p.row - i;
            if (r < 1) break;
            if (board_at(r, p.col).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = r, .col = p.col};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            c = p.col + i;
            if (c > H) break;
            if (board_at(p.row, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = p.row, .col = c};
            (*count)++;
        }
        for (int i = 1; i < 8; i++) {
            c = p.col - i;
            if (c < A) break;
            if (board_at(p.row, c).type != EMPTY) break;
            possible_moves[*count] = (Square) {.row = p.row, .col = c};
            (*count)++;
        }
    } else if (p.type == KING) {
        for (int drow = -1; drow <= 1; drow++) {
            for (int dcol = -1; dcol <= 1; dcol++) {
                if (drow == 0 && dcol == 0) continue;
                r = p.row + drow;
                c = p.col + dcol;
                if (r >= 1 && r <= 8 && c >= A && c <= H && board_at(r, c).type == EMPTY) {
                    possible_moves[*count] = (Square) {.row = r, .col = c};
                    (*count)++;
                }
            }
        }        
    } else {
        printf("Unknown piece type");
    }
}

void DrawPossibleMoves(Square possible_moves[POSSIBLE_MOVES_CAP], int possible_moves_count)
{
    const float r = 10.0f;
    for (int i = 0; i < possible_moves_count; i++) {
        int x = (possible_moves[i].col - 1) * SQUARE_SIZE + SQUARE_SIZE/2;
        int y = BOARD_SIZE - (possible_moves[i].row - 1) * SQUARE_SIZE - SQUARE_SIZE/2;
        DrawCircle(x, y, r, GRAY);
    }
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess");
    // TODO: figure out icon format for SetWindowIcon(Image image)
    SetTargetFPS(60);

    Texture2D piece_texture = LoadTexture("pieces.png");
    Piece board[8][8];
    Row target_row, sel_piece_row;
    Column target_col, sel_piece_col;
    bool selected_piece = false;
    Square possible_moves[POSSIBLE_MOVES_CAP];
    int possible_moves_count = 0;

    initialize_board(board);
    
    while (!WindowShouldClose())
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !selected_piece) {
            Vector2 mouse_pos = GetMousePosition();
            sel_piece_col = ((int) mouse_pos.x) / SQUARE_SIZE + 1;
            sel_piece_row = 8 - ((int) mouse_pos.y) / SQUARE_SIZE;
            if (sel_piece_col >= A && sel_piece_col <= H && sel_piece_row >= 1 && sel_piece_row <= 8 && board_at(sel_piece_row, sel_piece_col).type != EMPTY) {
                selected_piece = true;
                board_at(sel_piece_row, sel_piece_col).selected = true;
                calculate_possible_moves(board_at(sel_piece_row, sel_piece_col), board, possible_moves, &possible_moves_count);
            }
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && selected_piece) {
            Vector2 mouse_pos = GetMousePosition();
            target_col = ((int) mouse_pos.x) / SQUARE_SIZE + 1;
            target_row = 8 - ((int) mouse_pos.y) / SQUARE_SIZE;
            if (target_col >= A && target_col <= H && target_row >= 1 && target_row <= 8 && board_at(target_row, target_col).type == EMPTY) {
                board_at(target_row, target_col) = board_at(sel_piece_row, sel_piece_col);
                board_at(target_row, target_col).row = target_row;
                board_at(target_row, target_col).col = target_col;
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
        EndDrawing();
    }

    CloseWindow();
    return 0;
}