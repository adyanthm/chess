#include <stdint.h>

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

#define COLOR_BIT 0b1000
#define TYPE_MASK 0b0111

#define NUM_PIECES 32

#define IS_BLACK(p) (((p) & COLOR_BIT) != 0)
#define TYPE_OF(p)  ((p) & TYPE_MASK)

typedef enum {
    EMPTY       = 0b0000,

    W_PAWN      = 0b0001,
    W_KNIGHT    = 0b0010,
    W_BISHOP    = 0b0011,
    W_ROOK      = 0b0100,
    W_QUEEN     = 0b0101,
    W_KING      = 0b0110,

    B_PAWN      = 0b1001,
    B_KNIGHT    = 0b1010,
    B_BISHOP    = 0b1011,
    B_ROOK      = 0b1100,
    B_QUEEN     = 0b1101,
    B_KING      = 0b1110,
} PieceType;

uint8_t board[64] = {
    B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_BISHOP, B_KNIGHT, B_ROOK,
    B_PAWN, B_PAWN,   B_PAWN,   B_PAWN,  B_PAWN, B_PAWN,   B_PAWN,   B_PAWN,
    EMPTY,  EMPTY,    EMPTY,    EMPTY,   EMPTY,  EMPTY,    EMPTY,    EMPTY,
    EMPTY,  EMPTY,    EMPTY,    EMPTY,   EMPTY,  EMPTY,    EMPTY,    EMPTY,
    EMPTY,  EMPTY,    EMPTY,    EMPTY,   EMPTY,  EMPTY,    EMPTY,    EMPTY,
    EMPTY,  EMPTY,    EMPTY,    EMPTY,   EMPTY,  EMPTY,    EMPTY,    EMPTY,
    W_PAWN, W_PAWN,   W_PAWN,   W_PAWN,  W_PAWN, W_PAWN,   W_PAWN,   W_PAWN,
    W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_BISHOP, W_KNIGHT, W_ROOK,
};
