#ifndef ENGINE_H
#define ENGINE_H
#include <stdbool.h>
#include <stdint.h>

#define COLOR_BIT   0b1000
#define TYPE_MASK   0b0111
#define BLACK_COLOR 0b1000
#define WHITE_COLOR 0b0000

#define IS_BLACK(p) (((p) & COLOR_BIT) != 0)
#define TYPE_OF(p)  ((p) & TYPE_MASK)

#define SQ(f, r)    ((r) * 8 + (f))
#define FILE_OF(s)  ((s) % 8)
#define RANK_OF(s)  ((s) / 8)

/*
 * How are we representing the Pieces?
 *
 * Each piece is stored in one byte (uint8_t):
 *
 *   bit:  7 6 5 4 3 2 1 0
 *         0 0 0 0 C T T T
 *
 *   bits 0-2 (T): piece type (1 = pawn ... 6 = king, 0 = empty)
 *   bit  3   (C): color (0 = white, 1 = black)
 *   bits 4-7    : unused, always 0
 *
 * Example:
 *
 * White Pawn: 0b00000001
 * Black Pawn: 0b00001001
 *
 * Color: PIECE & COLOR_BIT
 * Piece Type: PIECE & TYPE_MASK
 *
 * WARN: Check for EMPTY first because EMPTY has no color bit
 * so it looks like white.
*/

typedef enum {
    EMPTY    = 0b0000,

    W_PAWN   = 0b0001,
    W_KNIGHT = 0b0010,
    W_BISHOP = 0b0011,
    W_ROOK   = 0b0100,
    W_QUEEN  = 0b0101,
    W_KING   = 0b0110,

    B_PAWN   = 0b1001,
    B_KNIGHT = 0b1010,
    B_BISHOP = 0b1011,
    B_ROOK   = 0b1100,
    B_QUEEN  = 0b1101,
    B_KING   = 0b1110,
} PieceType;

typedef enum {
    PLAYING,
    CHECKMATE,
    STALEMATE,
} GameStatus;

typedef enum {
    SIDE_BLACK = 0,
    SIDE_WHITE = 1,
} Side;

enum { WK_CASTLE = 1, WQ_CASTLE = 2, BK_CASTLE = 4, BQ_CASTLE = 8 };

typedef struct {
    uint8_t    board[64];
    int        king[2];
    int        enpassant;
    uint8_t    castling;
    Side       turn;
    bool       in_check;
    GameStatus status;
} GameState;

extern GameState game;

static inline Side piece_side(uint8_t p) {
    return (p & COLOR_BIT) ? SIDE_BLACK : SIDE_WHITE;
}

static inline uint8_t side_to_color(Side s) {
    return (s == SIDE_BLACK) ? BLACK_COLOR : WHITE_COLOR;
}

static inline bool in_bounds(int f, int r) {
    return f >= 0 && f < 8 && r >= 0 && r < 8;
}

static inline bool is_empty(int f, int r) {
    return game.board[SQ(f, r)] == EMPTY;
}

int possible_moves(PieceType piece, int index, int *move_buf);
int is_under_attack(int index, uint8_t color);
bool is_check(uint8_t color);
int legal_moves_from(int from, int *buf);
void make_move(int from, int to);
GameStatus game_status(uint8_t color);

#endif
