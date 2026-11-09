#include "engine.h"
#include <raylib.h>
#include <stdint.h>

static const int knight_deltas[8][2] = {
    {-1, -2}, {-1,  2},
    { 1, -2}, { 1,  2},
    {-2, -1}, {-2,  1},
    { 2, -1}, { 2,  1}
};

static const int king_deltas[8][2] = {
    {-1, 1}, {0, 1},
    {1, 1}, {-1, 0},
    {1, 0}, {-1, -1},
    {0, -1}, {1, -1}
};

static const int bishop_directions[4][2] = {
    { 1,  1},
    {-1, -1},
    {-1,  1},
    { 1, -1}
};

static const int rook_directions[4][2] = {
    {-1, 0},
    {1, 0},
    {0, 1},
    {0, -1}
};

static const int queen_directions[8][2] = {
    { 1,  1}, {-1, -1},
    {-1,  1}, { 1, -1},
    {-1,  0}, { 1,  0},
    { 0,  1}, { 0, -1}
};

static int offset(int index, int df, int dr) {
    int file = index % 8 + df;
    int rank = index / 8 + dr;

    if (file < 0 || file >= 8 || rank < 0 || rank >= 8)
        return -1;

    return rank * 8 + file;
}

int possible_moves(PieceType piece, int index, int *move_buf){
    int piece_type = piece & TYPE_MASK;
    int size = 0;
    uint8_t me = piece & COLOR_BIT;

    switch (piece_type) {
        case W_KING:
        case B_KING:
        case W_KNIGHT:
        case B_KNIGHT: {

            const int (*deltas)[2] = (piece_type == W_KNIGHT || piece_type == B_KNIGHT) ? knight_deltas : king_deltas;

            for (int i = 0; i < 8; i++){
                int to = offset(index, deltas[i][0], deltas[i][1]);
                if (to == -1) continue;
                uint8_t target = board[to];
                if (target == EMPTY || (target & COLOR_BIT) != me)
                    move_buf[size++] = to;
            }
            break;
        }

        case W_QUEEN:
        case B_QUEEN:
        case W_BISHOP:
        case B_BISHOP:
        case W_ROOK:
        case B_ROOK: {

            const int (*deltas)[2] =
                (piece_type == W_BISHOP || piece_type == B_BISHOP) ? bishop_directions :
                    (piece_type == W_ROOK || piece_type == B_ROOK) ? rook_directions : queen_directions;

            int count = (piece_type == W_QUEEN || piece_type == B_QUEEN) ? 8 : 4;

            for (int i = 0; i < count; i++){

                int df = deltas[i][0];
                int dr = deltas[i][1];
                int pos = index;

                while ((pos = offset(pos, df, dr)) != -1){
                    uint8_t target = board[pos];

                    if (target == EMPTY){
                        move_buf[size++] = pos;
                        continue;
                    }

                    if ((target & COLOR_BIT) != me)
                        move_buf[size++] = pos;

                    break;
                }
            }
            break;
        }
    }
    return size;
}
