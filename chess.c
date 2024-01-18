#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

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
    Sound move_sound = LoadSound("assets/move.mp3");
    Sound capture_sound = LoadSound("assets/capture.mp3");
    Music menu_music =  LoadMusicStream("assets/menu.mp3");

    bool playing = false;
    Row target_row, selected_row;
    Column target_col, selected_col;
    bool selected_piece = false;
    bool calculated_moves = false;
    MoveBuffer possible_moves = {0};
    unsigned int move_index;
    float now;
    const Row back_row[2] = {1, 8};
    Move move;

    // TODO: implement move history
    // TODO: function to revert move, this needs a move history
    GameContext ctx;
    #define MOVE_HISTORY_CAP 200
    #define CTX_HISTORY_CAP 201
    GameContext ctx_history[CTX_HISTORY_CAP];
    unsigned int current_move = 0;


    
    while (!WindowShouldClose())
    {
        if (!playing) {
            // Menu state
            if (!IsMusicStreamPlaying(menu_music)) PlayMusicStream(menu_music);
            
            UpdateMusicStream(menu_music);
            now = GetTime();
            const float freq = 1.0f/4; // In seconds
            float alpha = (sinf(2 * PI * freq * now) + 1.0f)/2.0f;
            if (IsKeyPressed(KEY_ENTER)) {
                playing = true;
                initialize_game(&ctx);
                current_move = 0;
                ctx_history[current_move] = ctx;
                flush_move_buffer(&possible_moves); // Just to assure that we don't have junk data from a previous game
                StopMusicStream(menu_music);
            }

            BeginDrawing();
                ClearBackground(BROWN);
                Color tint = ColorAlpha(WHITE, alpha);
                char* title = "Papyrus Chess";
                int size = 160;
                int offset_x = MeasureText(title, size);
                Vector2 title_pos = { .x = SCREEN_WIDTH / 2 - offset_x / 4, .y = SCREEN_HEIGHT / 2 - 120};
                DrawTextEx(papyrus, title, title_pos, size, 0, WHITE);
                char* press_enter_text = "Press ENTER to start playing!";
                offset_x = MeasureText(press_enter_text, size/2);
                Vector2 press_enter_text_pos = { .x = SCREEN_WIDTH / 2 - offset_x / 4, .y = SCREEN_HEIGHT / 2 + 120};
                DrawTextEx(papyrus, press_enter_text, press_enter_text_pos, size/2, 0, tint);
            EndDrawing();
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
                        move = possible_moves.moves[move_index];
                        apply_move(move, &ctx);
                        if (move.type == CAPTURE || move.type == EN_PASSANT) {
                            PlaySound(capture_sound);
                        } else {
                            PlaySound(move_sound);
                        }
                        ctx.last_move = move;
                        // Update castling privileges
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
                    char* turn_msg = (ctx.turn == WH) ? "White to play" : "Black to play";
                    int pad = 50;
                    Vector2 turn_msg_pos = { .x = BOARD_SIZE + pad, .y = SCREEN_HEIGHT / 2 - 30};
                    float size = 60.0f;
                    float spacing = 5.0f;
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