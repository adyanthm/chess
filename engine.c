#include "engine.h"

int enpassant = -1;

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

static int is_empty(int index){
    return board[index] == EMPTY;
}

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
    int pawn_dir = (me == (B_PAWN & COLOR_BIT)) ? 1 : -1;
    switch (piece_type) {
        case W_KING:
        case W_KNIGHT: {

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
        case W_BISHOP:
        case W_ROOK:{

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
        case W_PAWN: {
            int rank = index / 8;
            int start_rank = (pawn_dir == 1) ? 1 : 6;

            // pawn_dir is 1 for black and -1 for white.
            // move forward
            int one = offset(index, 0, pawn_dir);
            if (one != -1 && is_empty(one)) {
                move_buf[size++] = one;

                int two = offset(index, 0, 2 * pawn_dir);
                if (rank == start_rank && two != -1 && is_empty(two))
                    move_buf[size++] = two;
            }

            // capture
            for (int df = -1; df <= 1; df += 2) {
                int cap = offset(index, df, pawn_dir);
                if (cap == -1) continue;
                bool normal = !is_empty(cap) && (board[cap] & COLOR_BIT) != me;
                bool ep = is_empty(cap) && cap == enpassant;

                if (normal || ep)
                    move_buf[size++] = cap;
            }
            break;
        }
    }
    return size;
}

int find_king(uint8_t color){
    for (int i = 0; i < 64; i++){
        int piece = board[i];
        if(piece != EMPTY && (piece & TYPE_MASK) == W_KING && (piece & COLOR_BIT) == color)
            return i;
    }
    return -1;
}

bool is_check(uint8_t color){
    int king = find_king(color);
    return king != -1 && is_under_attack(king, color) != -1;
}

int is_under_attack(int index, int piece){
    uint8_t me = piece & COLOR_BIT;

    // knight + king
    for (int j = 0; j < 2; j++){
        int piece_type = (j == 0) ? W_KNIGHT : W_KING;
        const int (*deltas)[2] = (j == 0) ? knight_deltas : king_deltas;
        for (int i = 0; i < 8; i++){
            int to = offset(index, deltas[i][0], deltas[i][1]);
            if (to == -1) continue;
            uint8_t target = board[to];
            if ((target & TYPE_MASK)== piece_type && // knight / king
                (target & COLOR_BIT) != me) // its an enemy.
                return to;
        }
    }


    // bishop + rook + queen.
    for (int j = 0; j < 2; j++){
        const int (*directions)[2] = (j == 0) ? bishop_directions : rook_directions;
        int piece_type = (j == 0) ? W_BISHOP : W_ROOK;
        for (int i = 0; i < 4; i++){
            int df = directions[i][0];
            int dr = directions[i][1];
            int pos = index;
            while ((pos = offset(pos, df, dr)) != -1){
                uint8_t target = board[pos];
                if (target == EMPTY){
                    continue;
                }
                if ((target & COLOR_BIT) != me && // its an enemy
                    ((target & TYPE_MASK) == piece_type || (target & TYPE_MASK) == W_QUEEN)) // its a (bishop / rook) / queen
                    return pos;
                break;
            }
        }
    }

    // pawns
    int my_dir = me ? 1 : -1; // pawns attack me in same direction as my pawns move.
    int offsets[2][2] = {{1, my_dir}, {-1, my_dir}};
    for (int i = 0; i < 2; i++) {
        int from = offset(index, offsets[i][0], offsets[i][1]);
        if (from == -1) continue;               // off the board

        uint8_t t = board[from];
        if (t != EMPTY &&
            (t & TYPE_MASK) == W_PAWN &&        // it's a pawn
            (t & COLOR_BIT) != me)              // and it's an enemy
            return from;
    }
    return -1;
}

int legal_moves_from(int from, int *buf){
    uint8_t piece = board[from];
    uint8_t me = piece & COLOR_BIT;
    int dir = me ? 1 : -1;
    bool is_pawn = (piece & TYPE_MASK) == W_PAWN;

    int move_buf[64];
    int n = possible_moves(piece, from, move_buf);
    int size = 0;

    for (int i = 0; i < n; i++){
        int to = move_buf[i];
        uint8_t captured = board[to];

        /* ---------- Begin En Passant Edge Case ----------*/

        // en passant: pawn lands on the en passant square (empty)
        bool is_ep = is_pawn && to == enpassant && captured == EMPTY;
        int victim = to - dir * 8;
        uint8_t victim_piece = EMPTY;

        if (is_ep){
            victim_piece = board[victim];
            board[victim] = EMPTY;  // remove the victim for the test
        }

        /*---------- End En Passant Edge Case*/

        board[to] = piece;          // simulate the move
        board[from] = EMPTY;

        if (!is_check(me))
            buf[size++] = to;

        board[from] = piece;        // undo
        board[to] = captured;

        if (is_ep)
            board[victim] = victim_piece;
    }
    return size;
}

static bool has_legal_move(uint8_t color){
    int buf[64];
    for (int i = 0; i < 64; i++){
        uint8_t piece = board[i];
        if (piece != EMPTY && (piece & COLOR_BIT) == color)
            if (legal_moves_from(i, buf) > 0)
                return true;
    }
    return false;
}

GameStatus game_status(uint8_t color){
    bool check = is_check(color);
    bool legal_moves = has_legal_move(color);

    if (check && !legal_moves)
        return CHECKMATE;
    else if (!check && !legal_moves)
        return STALEMATE;
    else
        return PLAYING;
}
