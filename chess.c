#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define BOARD_SIZE 800
#define SQUARE_SIZE (BOARD_SIZE/8)
#define SCREEN_HORIZ_PAD 400
#define SCREEN_HEIGHT BOARD_SIZE
#define SCREEN_WIDTH (BOARD_SIZE + SCREEN_HORIZ_PAD)
#define MOVE_BUFFER_CAP 30
#define MOVE_HISTORY_CAP 200

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

typedef enum {
    MOVE, CAPTURE, EN_PASSANT, CASTLES_SHORT, CASTLES_LONG
} MoveType;

typedef struct {
    Row row;
    Column col;
} Square;

typedef struct {
    Square from;
    Square to;
    MoveType type;
} Move;

typedef struct {
    PieceType type;
    Player player;
    Row row;
    Column col;
    bool selected;
} Piece;

typedef struct {
    Piece board[8][8];
    Player turn;
    bool check;
    bool mate;
    Move last_move;
    bool can_castle_short[2];
    bool can_castle_long[2];
    bool accept_move;
    bool promotion;
    size_t moves;
} GameContext;

typedef struct {
    Move moves[MOVE_BUFFER_CAP];
    unsigned int count;
} MoveBuffer;

static inline void flush_move_buffer(MoveBuffer *buf)
{
    buf->count = 0;
}

bool is_threatened(Row, Column, Player, GameContext);

#define board_at(row, col) board[row - 1][col - 1]
#define EMPTY_PIECE(r, c) (Piece) {.type = EMPTY, .player = NONE, .row = r, .col = c}
void initialize_board(GameContext *ctx)
{
    for (Row row = 1; row <= 8; row++) {
        for (Column col = A; col <= H; col++) {
            ctx->board_at(row, col) = EMPTY_PIECE(row, col);
        }
    }
    for (int col = A; col <= H; col++) {
        ctx->board_at(2, col) = (Piece) {.type = PAWN, .player = WH, .row = 2, .col = col};
        ctx->board_at(7, col) = (Piece) {.type = PAWN, .player = BL, .row = 7, .col = col};
    }
    ctx->board_at(1, A) = (Piece) {.type = ROOK, .player = WH, .col = A, .row = 1};
    ctx->board_at(1, B) = (Piece) {.type = KNIGHT, .player = WH, .col = B, .row = 1};
    ctx->board_at(1, C) = (Piece) {.type = BISHOP, .player = WH, .col = C, .row = 1};
    ctx->board_at(1, D) = (Piece) {.type = QUEEN, .player = WH, .col = D, .row = 1};
    ctx->board_at(1, E) = (Piece) {.type = KING, .player = WH, .col = E, .row = 1};
    ctx->board_at(1, F) = (Piece) {.type = BISHOP, .player = WH, .col = F, .row = 1};
    ctx->board_at(1, G) = (Piece) {.type = KNIGHT, .player = WH, .col = G, .row = 1};
    ctx->board_at(1, H) = (Piece) {.type = ROOK, .player = WH, .col = H, .row = 1};

    ctx->board_at(8, A) = (Piece) {.type = ROOK, .player = BL, .col = A, .row = 8};
    ctx->board_at(8, B) = (Piece) {.type = KNIGHT, .player = BL, .col = B, .row = 8};
    ctx->board_at(8, C) = (Piece) {.type = BISHOP, .player = BL, .col = C, .row = 8};
    ctx->board_at(8, D) = (Piece) {.type = QUEEN, .player = BL, .col = D, .row = 8};
    ctx->board_at(8, E) = (Piece) {.type = KING, .player = BL, .col = E, .row = 8};
    ctx->board_at(8, F) = (Piece) {.type = BISHOP, .player = BL, .col = F, .row = 8};
    ctx->board_at(8, G) = (Piece) {.type = KNIGHT, .player = BL, .col = G, .row = 8};
    ctx->board_at(8, H) = (Piece) {.type = ROOK, .player = BL, .col = H, .row = 8};
}

void initialize_game(GameContext *ctx)
{
    initialize_board(ctx);
    ctx->turn = WH;
    ctx->check = false;
    ctx->mate = false;
    ctx->can_castle_short[WH] = true;
    ctx->can_castle_short[BL] = true;
    ctx->can_castle_long[WH] = true;
    ctx->can_castle_long[BL] = true;
    ctx->accept_move = true;
    ctx->promotion = false;
    ctx->moves = 0;
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

void DrawPieces(GameContext ctx, Texture2D texture)
{
    // TODO: functions to convert between (row, col) and (screen_x, screen_y)
    // TODO: accept `BoardRect`
    int pad_x;
    int pad_y;
    Rectangle rec;
    Vector2 pos;
    for (Row row = 1; row <= 8; row++) {
        for (Column col = A; col <= H; col++) {
            Piece piece = ctx.board_at(row, col);
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

void allocate_move(Square from, Square to, MoveType type, MoveBuffer *buf)
{
    buf->moves[buf->count].from = from;
    buf->moves[buf->count].to = to;    
    buf->moves[buf->count].type = type;
    (buf->count)++;
}

void calculate_diagonal_moves(Piece p, MoveBuffer *possible_moves, GameContext ctx)
{
    Row r;
    Column c;
    for (int i = 1; i < 8; i++) {
        r = p.row + i;
        c = p.col + i;
        if (r > 8 || c > H) break;
        if (ctx.board_at(r, c).type != EMPTY) {
            if (ctx.board_at(r, c).player == p.player) {break;}
            else {
                allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = c}, CAPTURE, possible_moves);
                break;
            }
        }
        allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = c}, MOVE, possible_moves);
    }
    for (int i = 1; i < 8; i++) {
        r = p.row - i;
        c = p.col + i;
        if (r < 1 || c > H) break;
        if (ctx.board_at(r, c).type != EMPTY) {
            if (ctx.board_at(r, c).player == p.player) {break;}
            else {
                allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = c}, CAPTURE, possible_moves);
                break;
            }
        }
        allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = c}, MOVE, possible_moves);
    }
    for (int i = 1; i < 8; i++) {
        r = p.row + i;
        c = p.col - i;
        if (r > 8 || c < A) break;
        if (ctx.board_at(r, c).type != EMPTY) {
            if (ctx.board_at(r, c).player == p.player) {break;}
            else {
                allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = c}, CAPTURE, possible_moves);
                break;
            }
        }
        allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = c}, MOVE, possible_moves);
    }
    for (int i = 1; i < 8; i++) {
        r = p.row - i;
        c = p.col - i;
        if (r < 1 || c < A) break;
        if (ctx.board_at(r, c).type != EMPTY) {
            if (ctx.board_at(r, c).player == p.player) {break;}
            else {
                allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = c}, CAPTURE, possible_moves);
                break;
            }
        }
        allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = c}, MOVE, possible_moves);
    }
}

void calculate_orthogonal_moves(Piece p, MoveBuffer *possible_moves, GameContext ctx)
{
    Row r;
    Column c;
    for (int i = 1; i < 8; i++) {
        r = p.row + i;
        if (r > 8) break;
        if (ctx.board_at(r, p.col).type != EMPTY) {
            if (ctx.board_at(r, p.col).player == 1 - p.player) {
                allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = p.col}, CAPTURE, possible_moves);
            } 
            break;
        }
        allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = p.col}, MOVE, possible_moves);
    }
    for (int i = 1; i < 8; i++) {
        r = p.row - i;
        if (r < 1) break;
        if (ctx.board_at(r, p.col).type != EMPTY) {
            if (ctx.board_at(r, p.col).player == 1 - p.player) {
                allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = p.col}, CAPTURE, possible_moves);
            } 
            break;
        }
        allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = p.col}, MOVE, possible_moves);
    }
    for (int i = 1; i < 8; i++) {
        c = p.col + i;
        if (c > H) break;
        if (ctx.board_at(p.row, c).type != EMPTY) {
            if (ctx.board_at(p.row, c).player == 1 - p.player) {
                allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row, .col = c}, CAPTURE, possible_moves);
            } 
            break;
        }
        allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row, .col = c}, MOVE, possible_moves);
    }
    for (int i = 1; i < 8; i++) {
        c = p.col - i;
        if (c < A) break;
        if (ctx.board_at(p.row, c).type != EMPTY) {
            if (ctx.board_at(p.row, c).player == 1 - p.player) {
                allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row, .col = c}, CAPTURE, possible_moves);
            } 
            break;
        }
        allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row, .col = c}, MOVE, possible_moves);
    }
}

void calculate_possible_moves(Piece p, MoveBuffer *possible_moves, GameContext ctx)
{
    Row r;
    Column c;
    if (p.type == PAWN) {
        int direction = (p.player == WH) ? 1 : -1;
        Row starting_row = (p.player == WH) ? 2 : 7;
        if (p.col + 1 <= H && ctx.board_at(p.row + direction, p.col + 1).player == 1 - p.player) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row + direction, .col = p.col + 1}, CAPTURE, possible_moves);
        }
        if (p.col - 1 >= A && ctx.board_at(p.row + direction, p.col - 1).player == 1 - p.player) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row + direction, .col = p.col - 1}, CAPTURE, possible_moves);
        }
        if (p.col + 1 <= H && ctx.board_at(p.row, p.col + 1).type == PAWN && ctx.board_at(p.row, p.col + 1).player == 1 - p.player && ctx.board_at(ctx.last_move.to.row, ctx.last_move.to.col).type == PAWN && abs(ctx.last_move.from.row - ctx.last_move.to.row) == 2 && ctx.last_move.to.row == p.row && ctx.last_move.to.col == p.col + 1) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row + direction, .col = p.col + 1}, EN_PASSANT, possible_moves);
        }
        if (p.col - 1 >= A && ctx.board_at(p.row, p.col - 1).type == PAWN && ctx.board_at(p.row, p.col - 1).player == 1 - p.player && ctx.board_at(ctx.last_move.to.row, ctx.last_move.to.col).type == PAWN && abs(ctx.last_move.from.row - ctx.last_move.to.row) == 2 && ctx.last_move.to.row == p.row && ctx.last_move.to.col == p.col - 1) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row + direction, .col = p.col - 1}, EN_PASSANT, possible_moves);
        }
        if (p.row == 1 || p.row == 8) return;
        if (ctx.board_at(p.row + direction, p.col).type != EMPTY) return;
        allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row + direction, .col = p.col}, MOVE, possible_moves);
        
        if (ctx.board_at(p.row + 2*direction, p.col).type != EMPTY || p.row != starting_row) return;
        allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row + 2*direction, .col = p.col}, MOVE, possible_moves);        
    } else if (p.type == KNIGHT) {
        // NOTE: the enum literals are of type unsigned int. If you have `Column c = 0;` and you decrement its value `c--;`, it goes to the maximum value of unsigned int
        // Therefore, before comparing them to the minimum values, if they might be negative (which is the case for col - 2), you must cast to int before the comparison
        if (p.row + 2 <= 8 && p.col + 1 <= H && ctx.board_at(p.row + 2, p.col + 1).player != p.player) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row + 2, .col = p.col + 1}, (ctx.board_at(p.row + 2, p.col + 1).player == NONE) ? MOVE : CAPTURE, possible_moves);
        }
        if (p.row + 2 <= 8 && (int) p.col - 1 >= A && ctx.board_at(p.row + 2, p.col - 1).player != p.player) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row + 2, .col = p.col - 1}, (ctx.board_at(p.row + 2, p.col - 1).player == NONE) ? MOVE : CAPTURE, possible_moves);
        }
        if ((int) p.row - 2 >= 1 && p.col + 1 <= H && ctx.board_at(p.row - 2, p.col + 1).player != p.player) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row - 2, .col = p.col + 1}, (ctx.board_at(p.row - 2, p.col + 1).player == NONE) ? MOVE : CAPTURE, possible_moves);
        }
        if ((int) p.row - 2 >= 1 && (int) p.col - 1 >= A && ctx.board_at(p.row - 2, p.col - 1).player != p.player) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row - 2, .col = p.col - 1}, (ctx.board_at(p.row - 2, p.col - 1).player == NONE) ? MOVE : CAPTURE, possible_moves);
        }
        if (p.row + 1 <= 8 && p.col + 2 <= H && ctx.board_at(p.row + 1, p.col + 2).player != p.player) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row + 1, .col = p.col + 2}, (ctx.board_at(p.row + 1, p.col + 2).player == NONE) ? MOVE : CAPTURE, possible_moves);
        }
        if (p.row + 1 <= 8 && (int) p.col - 2 >= A && ctx.board_at(p.row + 1, p.col - 2).player != p.player) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row + 1, .col = p.col - 2}, (ctx.board_at(p.row + 1, p.col - 2).player == NONE) ? MOVE : CAPTURE, possible_moves);
        }
        if ((int) p.row - 1 >= 1 && p.col + 2 <= H && ctx.board_at(p.row - 1, p.col + 2).player != p.player) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row - 1, .col = p.col + 2}, (ctx.board_at(p.row - 1, p.col + 2).player == NONE) ? MOVE : CAPTURE, possible_moves);
        }
        if ((int) p.row - 1 >= 1 && (int) p.col - 2 >= A && ctx.board_at(p.row - 1, p.col - 2).player != p.player) {
            allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row - 1, .col = p.col - 2}, (ctx.board_at(p.row - 1, p.col - 2).player == NONE) ? MOVE : CAPTURE, possible_moves);
        }
    } else if (p.type == BISHOP) {
        calculate_diagonal_moves(p, possible_moves, ctx);
    } else if (p.type == ROOK) {
        calculate_orthogonal_moves(p, possible_moves, ctx);
    } else if (p.type == QUEEN) {
        calculate_diagonal_moves(p, possible_moves, ctx);
        calculate_orthogonal_moves(p, possible_moves, ctx);
    } else if (p.type == KING) {
        if (ctx.can_castle_short[p.player]) {
            // TODO: we might wanna make an assertion that the king and the rook are on their initial squares
            if (ctx.board_at(p.row, p.col + 1).type == EMPTY && ctx.board_at(p.row, p.col + 2).type == EMPTY && !is_threatened(p.row, p.col + 1, 1 - p.player, ctx) && !is_threatened(p.row, p.col + 2, 1 - p.player, ctx)) {
                allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row, .col = p.col + 2}, CASTLES_SHORT, possible_moves);
            }
        }
        if (ctx.can_castle_long[p.player]) {
            // TODO: we might wanna make an assertion that the king and the rook are on their initial squares
            if (ctx.board_at(p.row, p.col - 1).type == EMPTY && ctx.board_at(p.row, p.col - 2).type == EMPTY && ctx.board_at(p.row, p.col - 3).type == EMPTY && !is_threatened(p.row, p.col - 1, 1 - p.player, ctx) && !is_threatened(p.row, p.col - 2, 1 - p.player, ctx) && !is_threatened(p.row, p.col - 3, 1 - p.player, ctx)) {
                allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = p.row, .col = p.col - 2}, CASTLES_LONG, possible_moves);
            }
        }
        for (int drow = -1; drow <= 1; drow++) {
            for (int dcol = -1; dcol <= 1; dcol++) {
                if (drow == 0 && dcol == 0) continue;
                r = p.row + drow;
                c = p.col + dcol;
                if (r >= 1 && r <= 8 && c >= A && c <= H && ctx.board_at(r, c).player != p.player) {
                    allocate_move((Square) { .row = p.row, .col = p.col}, (Square) {.row = r, .col = c}, (ctx.board_at(r, c).player == NONE) ? MOVE : CAPTURE, possible_moves);
                }
            }
        }        
    } else {
        printf("Unknown piece type");
    }
}

void DrawPossibleMoves(MoveBuffer possible_moves)
{
    const float r = 10.0f;
    for (unsigned int i = 0; i < possible_moves.count; i++) {
        int x = (possible_moves.moves[i].to.col - 1) * SQUARE_SIZE + SQUARE_SIZE/2;
        int y = BOARD_SIZE - (possible_moves.moves[i].to.row - 1) * SQUARE_SIZE - SQUARE_SIZE/2;
        if (possible_moves.moves[i].type == MOVE || possible_moves.moves[i].type == CASTLES_SHORT || possible_moves.moves[i].type == CASTLES_LONG) DrawCircle(x, y, r, GRAY);
        else if (possible_moves.moves[i].type == CAPTURE || possible_moves.moves[i].type == EN_PASSANT) DrawCircle(x, y, r, RED);
    }
}

bool is_possible(Row r, Column c, MoveBuffer possible_moves, unsigned int *index)
{
    for (unsigned int i = 0; i < possible_moves.count; i++) {
        if (possible_moves.moves[i].to.row == r && possible_moves.moves[i].to.col == c) {
            *index = i;
            return true;   
        }
    }
    return false;
}

void find_king(GameContext ctx, Player p, Square *king_square)
{
    for (int row = 1; row <= 8; row++) {
        for (int col = A; col <= H; col++) {
            if (ctx.board_at(row, col).player == p && ctx.board_at(row, col).type == KING) {
                *king_square = (Square) {.row = row, .col = col};
                return;
            } 
        }
    }
}

bool is_check(GameContext ctx)
{
    MoveBuffer possible_moves = {0};
    Square king_square;
    unsigned int move_index;
    
    find_king(ctx, ctx.turn, &king_square);
    for (int row = 1; row <= 8; row++) {
        for (int col = A; col <= H; col++) {
            if (ctx.board_at(row, col).player != 1 - ctx.turn) continue;
            if (ctx.board_at(row, col).type == KING) continue;
            calculate_possible_moves(ctx.board_at(row, col), &possible_moves, ctx);
            if (is_possible(king_square.row, king_square.col, possible_moves, &move_index)) return true;
            flush_move_buffer(&possible_moves);
        }
    }
    return false;
}

void apply_move(Move move, GameContext *ctx)
{
    ctx->board_at(move.to.row, move.to.col) = ctx->board_at(move.from.row, move.from.col);
    ctx->board_at(move.to.row, move.to.col).row = move.to.row;
    ctx->board_at(move.to.row, move.to.col).col = move.to.col;
    ctx->board_at(move.to.row, move.to.col).selected = false;
    ctx->board_at(move.from.row, move.from.col) = EMPTY_PIECE(move.from.row, move.from.col);
    if (move.type == EN_PASSANT) {
        ctx->board_at(move.from.row, move.to.col) = EMPTY_PIECE(move.from.row, move.to.col);
    } else if (move.type == CASTLES_SHORT) {
        ctx->board_at(move.to.row, move.to.col - 1) = ctx->board_at(move.to.row, move.to.col + 1);
        ctx->board_at(move.to.row, move.to.col - 1).col = move.to.col - 1;
        ctx->board_at(move.to.row, move.to.col + 1) =  EMPTY_PIECE(move.to.row, move.to.col + 1);
    } else if (move.type == CASTLES_LONG) {
        ctx->board_at(move.to.row, move.to.col + 1) = ctx->board_at(move.to.row, A);
        ctx->board_at(move.to.row, move.to.col + 1).col = move.to.col + 1;
        ctx->board_at(move.to.row, A) =  EMPTY_PIECE(move.to.row, A);;
    }
}

void validate_possible_moves(Piece p, MoveBuffer *possible_moves, GameContext ctx)
{
    int new_count = 0;
    GameContext next_ctx = ctx;
    Piece piece_at_target;
    Move move;
    bool check;

    if (possible_moves->count == 0) return;

    for (unsigned int i = 0; i < possible_moves->count; i++) {
        // Perform the move in the next_board
        move = possible_moves->moves[i];
        piece_at_target = next_ctx.board_at(move.to.row, move.to.col);
        apply_move(move, &next_ctx);
        check = is_check(next_ctx);
        if (!check) {
            possible_moves->moves[new_count] = move;
            new_count++;
        }
        // TODO: this might not be enough to deapply the move
        next_ctx.board_at(p.row, p.col) = p;
        next_ctx.board_at(move.to.row, move.to.col) = piece_at_target;
    }
    possible_moves->count = new_count;
}

bool is_threatened(Row row, Column col, Player p, GameContext ctx)
{
    MoveBuffer possible_moves = {0};
    unsigned int move_index;
    for (Row r = 1; r <= 8; r++) {
        for (Column c = A; c <= H; c++) {
            if (ctx.board_at(r, c).player != p) continue;
            if (ctx.board_at(r, c).type == KING) continue;
            calculate_possible_moves(ctx.board_at(r, c), &possible_moves, ctx);
            validate_possible_moves(ctx.board_at(r, c), &possible_moves, ctx);
            if (is_possible(row, col, possible_moves, &move_index)) return true;
            flush_move_buffer(&possible_moves);
        }
    }
    return false;
}

bool is_mate(GameContext ctx)
{
    MoveBuffer possible_moves = {0};
    for (Row r = 1; r <= 8; r++) {
        for (Column c = A; c <= H; c++) {
            if (ctx.board_at(r, c).player != ctx.turn) continue;
            calculate_possible_moves(ctx.board_at(r, c), &possible_moves, ctx);
            validate_possible_moves(ctx.board_at(r, c), &possible_moves, ctx);
            if (possible_moves.count > 0) {
                return false;
            }
            flush_move_buffer(&possible_moves);
        }
    }
    return true;
}

// Render Functions
void DrawTextCentered_(const char* text, float x, float y, int font_size, Font font)
{
    Vector2 title_render_sizes = MeasureTextEx(font, text, font_size, 0);
    Vector2 title_pos = { .x = x - title_render_sizes.x/2, .y = y - title_render_sizes.y/2};
    DrawTextEx(font, text, title_pos, font_size, 0, WHITE);
}
#define DrawTextCentered(text, x, y, font_size) DrawTextCentered_(text, x, y, font_size, papyrus)

void DrawButtonWithText_(Rectangle button, const char* text, int font_size, Color color, Font font)
{
    DrawRectangleRec(button, color);
    DrawTextCentered_(text, button.x + button.width/2, button.y + button.height/2, font_size, font);
}
#define DrawButtonWithText(button, text, font_size, color) DrawButtonWithText_(button, text, font_size, color, papyrus)

void DrawTextInRect(Rectangle r, const char* text, int font_size, Font font)
{
    const char* begin = text;
    const char* end = text;
    char word[256];
    char line[1024];
    size_t line_len = 0;
    line[line_len] = '\0';
    size_t lines = 0;
    do {
        // Trim left
        while (*begin == ' ') begin++;
        if (*begin == '\0') return;
        end = begin;

        // Find next space
        while (*end != ' ' && *end != '\0') end++;

        // Build string
        for (const char* c = begin; c < end; c++) {
            word[c - begin] = *c;
        }
        word[end - begin] = '\0';

        // Measure current line and word separately
        Vector2 measures_line = MeasureTextEx(font, line, font_size, 0);
        Vector2 measures_word = MeasureTextEx(font, word, font_size, 0);

        if (measures_line.x + measures_word.x < r.width || *end == '\0') {
            // Append word to line
            for (size_t i = 0; i < (size_t) (end - begin); i++) {
                line[line_len + i] = word[i];
            }
            line[line_len + (end - begin)] = ' ';
            line_len += (end - begin + 1);
            line[line_len] = '\0';
            // Go to next word
            begin = end;
        }
        if (measures_line.x + measures_word.x > r.width || *end == '\0') {
            // Draw line
            DrawTextEx(font, line, (Vector2) { r.x, r.y + lines*measures_line.y }, font_size, 0, WHITE);
            lines += 1;

            // Flush word
            line_len = 0;
            line[line_len] = '\0';
        }
    } while (*end != '\0');
}

bool is_move_ambiguous(Move move, GameContext ctx, Square* other_piece_square)
{
    // Find out if the move is ambiguous
    Piece piece = ctx.board_at(move.from.row, move.from.col);
    MoveBuffer buf = {0};
    unsigned int index;
    if (piece.type != PAWN && piece.type != KING) {
        for (size_t row = 1; row <= 8; row++) {
            for (size_t col = A; col <= H; col++) {
                if (row == (size_t) piece.row && col == (size_t) piece.col) continue;
                Piece other_piece = ctx.board_at(row, col);
                if (other_piece.player != piece.player || other_piece.type != piece.type) continue;
                calculate_possible_moves(other_piece, &buf, ctx);
                if (is_possible(move.to.row, move.to.col, buf, &index)) {
                    *other_piece_square = (Square) { .row = row, .col = col };
                    return true;
                }
                flush_move_buffer(&buf);
            }
        }
    }
    return false;
}

void algebraic_notation(Move move, GameContext ctx, char* notation)
{
    if (move.type == CASTLES_SHORT) {
        notation = "o-o";
        return;
    } else if (move.type == CASTLES_LONG) {
        notation = "o-o-o";
        return;
    }
    
    size_t index = 0;
    Piece piece = ctx.board_at(move.from.row, move.from.col);
    if (piece.type == PAWN && move.type == CAPTURE) {
        notation[index++] = 'a' + move.from.col - 1;
    } else if (piece.type == KNIGHT) {
        notation[index++] = 'N';
    } else if (piece.type == BISHOP) {
        notation[index++] = 'B';
    } else if (piece.type == ROOK) {
        notation[index++] = 'R';
    } else if (piece.type == KING) {
        notation[index++] = 'K';
    } else if (piece.type == QUEEN) {
        notation[index++] = 'Q';
    }

    Square other_piece_square;
    bool ambiguous = is_move_ambiguous(move, ctx, &other_piece_square);

    if (ambiguous) {
        if (piece.col == other_piece_square.col) {
            notation[index++] = '1' + piece.row - 1;
        } else {
            notation[index++] = 'a' + piece.col - 1;
        }
    }

    if (move.type == CAPTURE) notation[index++] = 'x';

    notation[index++] = 'a' + move.to.col - 1;
    notation[index++] = '1' + move.to.row - 1;  

    // TODO: make sure this is a good idea
    // TODO: check notation is not working
    apply_move(move, &ctx);
    ctx.turn = 1 - ctx.turn;
    if (is_check(ctx)) {
        notation[index++] = '+';
    }

    notation[index] = '\0';
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess");
    InitAudioDevice();
    // TODO: figure out icon format for SetWindowIcon(Image image)
    SetTargetFPS(60);

    // TODO: Maybe create a struct `GameAssets` to capsule all of this and 
    // a function `RenderGameState(GameContext ctx, GameAssets assets)`
    // and maybe even `GameAssets LoadGameAssets(void)`
    Texture2D piece_texture = LoadTexture("assets/pieces.png");
    Font papyrus = LoadFont("assets/papyrus.ttf");
    SetTextureFilter(papyrus.texture, TEXTURE_FILTER_BILINEAR);
    Sound move_sound = LoadSound("assets/move.mp3");
    Sound capture_sound = LoadSound("assets/capture.mp3");
    Music menu_music =  LoadMusicStream("assets/menu.mp3");

    // Program metadata to know what is the program state
    bool playing = false;
    bool tutorial = false;
    char notation[7];

    // Variables related to chess game
    Row target_row, selected_row;
    Column target_col, selected_col;
    bool selected_piece = false;
    bool calculated_moves = false;
    MoveBuffer possible_moves = {0};
    unsigned int move_index;
    const Row back_row[2] = {1, 8};
    Move move;

    // TODO: implement move history
    // TODO: function to revert move, this needs a move history
    GameContext ctx;
    #define MOVE_HISTORY_CAP 200
    #define CTX_HISTORY_CAP 201
    GameContext ctx_history[CTX_HISTORY_CAP];
    unsigned int current_move = 0;
    
    while (!WindowShouldClose()) {
        if (!playing) {
            // Menu state
            if (!IsMusicStreamPlaying(menu_music)) PlayMusicStream(menu_music);
            UpdateMusicStream(menu_music);

            // Draw menu screen
            BeginDrawing();
                ClearBackground(BROWN);
                
                int font_size = 160;
                float title_up_offset = 180.0f;
                DrawTextCentered_("Papyrus Chess", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - title_up_offset, font_size, papyrus);

                // TODO: implement button functionality
                float button_width = 500.0f;
                float button_height = 150.0f;
                float button_down_offset = 40.0f;
                Rectangle play_button = {
                    .x = SCREEN_WIDTH/2 - button_width/2,
                    .y = SCREEN_HEIGHT/2 - button_height/2 + button_down_offset,
                    .width = button_width,
                    .height = button_height
                };
                DrawButtonWithText(play_button, "Play", 80, DARKBROWN);

                button_down_offset += button_height + 40.0f;
                Rectangle tutorial_button = {
                    .x = SCREEN_WIDTH/2 - button_width/2,
                    .y = SCREEN_HEIGHT/2 - button_height/2 + button_down_offset,
                    .width = button_width,
                    .height = button_height
                };
                DrawButtonWithText(tutorial_button, "How to play", 80, DARKBROWN);

                const float tutorial_box_width = SCREEN_WIDTH*0.7;
                const float tutorial_box_height = SCREEN_HEIGHT*0.85;
                const Rectangle tutorial_box = {
                    .x = SCREEN_WIDTH/2 - tutorial_box_width/2,
                    .y = SCREEN_HEIGHT/2 - tutorial_box_height/2,
                    .width = tutorial_box_width,
                    .height = tutorial_box_height,
                };
                const float tutorial_close_button_padding = tutorial_box.width*0.05;
                    Rectangle tutorial_close_button = {
                        .x = tutorial_box.x + tutorial_close_button_padding,
                        .y = tutorial_box.y + tutorial_close_button_padding,
                        .width = 150,
                        .height = 100
                };
                if (tutorial) {
                    // Draw tutorial box
                    DrawRectangleRec(tutorial_box, DARKBROWN);
                    DrawButtonWithText(tutorial_close_button, "Close", 80, DARKGRAY);
                    static const char* text = "When it's your turn, drag and drop pieces to the squares you want to place them. Visual indicators will show where you can place the piece. Alternate turns with a friend!";
                    Rectangle text_area = {
                        .x = tutorial_box.x + tutorial_close_button_padding,
                        .y = tutorial_close_button.y + tutorial_close_button.height + tutorial_close_button_padding,
                        .width = tutorial_box_width - 2*tutorial_close_button_padding,
                        .height = tutorial_box_height - tutorial_close_button.height - 3*tutorial_close_button_padding
                    };
                    DrawRectangleRec(text_area, DARKBROWN);
                    DrawTextInRect(text_area, text, 60, papyrus);
                }
            EndDrawing();

            // Handle button events
            Vector2 mouse = GetMousePosition();
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                if (!tutorial && CheckCollisionPointRec(mouse, play_button)) {
                    playing = true;
                    initialize_game(&ctx);
                    current_move = 0;
                    ctx_history[current_move] = ctx;
                    flush_move_buffer(&possible_moves); // Just to assure that we don't have junk data from a previous game
                    StopMusicStream(menu_music);
                }
                if (!tutorial) {
                    tutorial = CheckCollisionPointRec(mouse, tutorial_button);
                }
                if (tutorial) {
                    tutorial = !CheckCollisionPointRec(mouse, tutorial_close_button);
                }
            }
        } else {
            // Playing state
            if (ctx.accept_move) {
                // Read user input
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !selected_piece) {
                    Vector2 mouse_pos = GetMousePosition();
                    selected_col = ((int) mouse_pos.x) / SQUARE_SIZE + 1;
                    selected_row = 8 - ((int) mouse_pos.y) / SQUARE_SIZE;
                    if (selected_col >= A && selected_col <= H && selected_row >= 1 && selected_row <= 8 && ctx.board_at(selected_row, selected_col).type != EMPTY && ctx.board_at(selected_row, selected_col).player == ctx.turn) {
                        selected_piece = true;
                        ctx.board_at(selected_row, selected_col).selected = true;
                        if (!calculated_moves) {
                            calculate_possible_moves(ctx.board_at(selected_row, selected_col), &possible_moves, ctx);
                            validate_possible_moves(ctx.board_at(selected_row, selected_col), &possible_moves, ctx);
                        }
                    }
                } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && selected_piece) {
                    // Process user input
                    Vector2 mouse_pos = GetMousePosition();
                    target_col = ((int) mouse_pos.x) / SQUARE_SIZE + 1;
                    target_row = 8 - ((int) mouse_pos.y) / SQUARE_SIZE;
                    if (target_col >= A && target_col <= H && target_row >= 1 && target_row <= 8 && is_possible(target_row, target_col, possible_moves, &move_index)) {
                        ctx.moves += 1;
                        move = possible_moves.moves[move_index];
                        algebraic_notation(move, ctx, notation);
                        apply_move(move, &ctx);
                        if (move.type == CAPTURE || move.type == EN_PASSANT) {
                            PlaySound(capture_sound);
                        } else {
                            PlaySound(move_sound);
                        }
                        ctx.last_move = move;
                        // Update castling privileges
                        // TODO: maybe this context updating should be moved to `apply_move`?
                        if (ctx.board_at(selected_row, selected_col).type == KING || move.type == CASTLES_SHORT || move.type == CASTLES_LONG) {
                            ctx.can_castle_short[ctx.turn] = false;
                            ctx.can_castle_long[ctx.turn] = false;
                        } else if (ctx.board_at(selected_row, selected_col).type == ROOK && selected_row == back_row[ctx.turn] && selected_col == A) {
                            ctx.can_castle_long[ctx.turn] = false;
                        } else if (ctx.board_at(selected_row, selected_col).type == ROOK && selected_row == back_row[ctx.turn] && selected_col == H) {
                            ctx.can_castle_short[ctx.turn] = false;
                        }
                        
                        if (move.to.row == back_row[1 - ctx.turn] && ctx.board_at(move.to.row, move.to.col).type == PAWN) {
                            ctx.promotion = true;
                            ctx.accept_move = false;
                        }
                        
                        if (!ctx.promotion) {
                            // Remember: all the things here must be done after the user chooses the promotion piece
                            ctx.turn = 1 - ctx.turn;
                            // Register context in history
                            if (current_move < MOVE_HISTORY_CAP) {
                                current_move += 1;
                                ctx_history[current_move] = ctx;
                            }
                            ctx.check = is_check(ctx);
                            if (ctx.check) {
                                ctx.mate = is_mate(ctx);
                            }
                        }
                    } else {
                        ctx.board_at(selected_row, selected_col).selected = false;
                    }
                    selected_piece = false;
                    flush_move_buffer(&possible_moves);
                } else if (IsKeyPressed(KEY_B)) {
                    // Revert move
                    if (current_move > 0) current_move -= 1;
                    ctx = ctx_history[current_move];
                }
                if (ctx.mate) ctx.accept_move = false;
            } else {
                // Handle cases where we don't accept standard user input
                if (ctx.mate && IsKeyPressed(KEY_ENTER)) playing = false;
                if (ctx.promotion) {
                    if (IsKeyPressed(KEY_Q)) {
                        ctx.board_at(move.to.row, move.to.col) = (Piece) {.type = QUEEN, .row = move.to.row, .col = move.to.col, .player = ctx.turn};
                        ctx.promotion = false;
                    } else if (IsKeyPressed(KEY_R)) {
                        ctx.board_at(move.to.row, move.to.col) = (Piece) {.type = ROOK, .row = move.to.row, .col = move.to.col, .player = ctx.turn};
                        ctx.promotion = false;
                    } else if (IsKeyPressed(KEY_N)) {
                        ctx.board_at(move.to.row, move.to.col) = (Piece) {.type = KNIGHT, .row = move.to.row, .col = move.to.col, .player = ctx.turn};
                        ctx.promotion = false;
                    } else if (IsKeyPressed(KEY_B)) {
                        ctx.board_at(move.to.row, move.to.col) = (Piece) {.type = BISHOP, .row = move.to.row, .col = move.to.col, .player = ctx.turn};
                        ctx.promotion = false;
                    }
                    if (!ctx.promotion) {
                        ctx.accept_move = true;
                        ctx.turn = 1 - ctx.turn;
                        ctx.check = is_check(ctx);
                        if (ctx.check) {
                            ctx.mate = is_mate(ctx);
                        }
                    }
                }
            }

            // Render playing state
            BeginDrawing();
                DrawBackground();
                DrawPieces(ctx, piece_texture);
                if (ctx.mate) {
                    char* win_msg = (ctx.turn == WH) ? "Black wins!" : "White wins!";
                    int pad = 50;
                    Vector2 win_msg_pos = { .x = BOARD_SIZE + pad, .y = SCREEN_HEIGHT / 2 - 30};
                    float size = 60.0f;
                    float spacing = 5.0f;
                    DrawTextEx(papyrus, win_msg, win_msg_pos, size, spacing, WHITE);
                    char* prompt = "Press ENTER to go";
                    char* prompt2 = "back to menu";
                    Vector2 prompt_pos = { .x = BOARD_SIZE + 10, .y = SCREEN_HEIGHT / 2 + 150};
                    Vector2 prompt_pos2 = { .x = BOARD_SIZE + 10, .y = SCREEN_HEIGHT / 2 + 200};
                    DrawTextEx(papyrus, prompt, prompt_pos, 50.0f, spacing, WHITE);
                    DrawTextEx(papyrus, prompt2, prompt_pos2, 50.0f, spacing, WHITE);
                } else {
                    if (selected_piece && possible_moves.count > 0) DrawPossibleMoves(possible_moves);
                    char last_move_msg[64];
                    if (ctx.turn == WH){
                        strcpy(last_move_msg, "Black played ");
                    } else {
                        strcpy(last_move_msg, "White played ");
                    }
                    strcpy(&last_move_msg[13], notation);
                    char* turn_msg = (ctx.turn == WH) ? "White to play" : "Black to play";
                    int pad = 50;
                    Vector2 turn_msg_pos = { .x = BOARD_SIZE + 1.3*pad, .y = 0.85*SCREEN_HEIGHT};
                    Vector2 last_move_msg_pos = { .x = BOARD_SIZE + 0.8*pad, .y = 0.75*SCREEN_HEIGHT};
                    float size = 60.0f;
                    float spacing = 5.0f;
                    if (ctx.moves > 0) {
                        DrawTextEx(papyrus, last_move_msg, last_move_msg_pos, size, spacing, WHITE);
                    }
                    DrawTextEx(papyrus, turn_msg, turn_msg_pos, size, spacing, WHITE);

                    if (ctx.check) {
                        char* check_msg = "In check";
                        pad = 100;
                        float y_check_msg = (ctx.turn == WH) ? SCREEN_HEIGHT / 2 + 250 : SCREEN_HEIGHT / 2 - 250;
                        Vector2 check_msg_pos = { .x = BOARD_SIZE + pad, .y = y_check_msg};
                        DrawTextEx(papyrus, check_msg, check_msg_pos, size, spacing, WHITE);
                    } else if (ctx.promotion) {
                        char* promotion_msgs[4] = {
                            "Press Q for queen",
                            "Press R for rook",
                            "Press N for knight",
                            "Press B for bishop"
                        };
                        for (size_t i = 0; i < 4; i++) {
                            Vector2 promotion_msg_loc = { .x = BOARD_SIZE + 10, .y = SCREEN_HEIGHT / 2 + 30 * (i + 1) };
                            DrawTextEx(papyrus, promotion_msgs[i], promotion_msg_loc, 50.0f, spacing, WHITE);
                        }
                    }
                }
            EndDrawing();
        }
    }
    UnloadMusicStream(menu_music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}