#include "engine.h"
#include <stdbool.h>
#include <stdlib.h>

GameState game = {
    .board = {
        B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_BISHOP, B_KNIGHT, B_ROOK,
        B_PAWN, B_PAWN,   B_PAWN,   B_PAWN,  B_PAWN, B_PAWN,   B_PAWN,   B_PAWN,
        EMPTY,  EMPTY,    EMPTY,    EMPTY,   EMPTY,  EMPTY,    EMPTY,    EMPTY,
        EMPTY,  EMPTY,    EMPTY,    EMPTY,   EMPTY,  EMPTY,    EMPTY,    EMPTY,
        EMPTY,  EMPTY,    EMPTY,    EMPTY,   EMPTY,  EMPTY,    EMPTY,    EMPTY,
        EMPTY,  EMPTY,    EMPTY,    EMPTY,   EMPTY,  EMPTY,    EMPTY,    EMPTY,
        W_PAWN, W_PAWN,   W_PAWN,   W_PAWN,  W_PAWN, W_PAWN,   W_PAWN,   W_PAWN,
        W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_BISHOP, W_KNIGHT, W_ROOK,
    },
    .king      = { [SIDE_BLACK] = SQ(4, 0), [SIDE_WHITE] = SQ(4, 7) },
    .enpassant = -1,
    .castling  = WK_CASTLE | WQ_CASTLE | BK_CASTLE | BQ_CASTLE,
    .turn      = SIDE_WHITE,
    .in_check  = false,
    .status    = PLAYING,
};

static const int knight_deltas[8][2] = {
    {-1, -2}, {-1,  2},
    { 1, -2}, { 1,  2},
    {-2, -1}, {-2,  1},
    { 2, -1}, { 2,  1}
};

static const int king_deltas[8][2] = {
    {-1,  1}, { 0,  1},
    { 1,  1}, {-1,  0},
    { 1,  0}, {-1, -1},
    { 0, -1}, { 1, -1}
};

static const int bishop_directions[4][2] = {
    { 1,  1}, {-1, -1},
    {-1,  1}, { 1, -1}
};

static const int rook_directions[4][2] = {
    {-1,  0}, { 1,  0},
    { 0,  1}, { 0, -1}
};

static const int queen_directions[8][2] = {
    { 1,  1}, {-1, -1},
    {-1,  1}, { 1, -1},
    {-1,  0}, { 1,  0},
    { 0,  1}, { 0, -1}
};

int is_under_attack(int index, uint8_t color) {
    uint8_t me = color & COLOR_BIT;
    int f0 = FILE_OF(index);
    int r0 = RANK_OF(index);

    // knight + king
    for (int j = 0; j < 2; j++) {
        int piece_type = (j == 0) ? W_KNIGHT : W_KING;
        const int (*deltas)[2] = (j == 0) ? knight_deltas : king_deltas;
        for (int i = 0; i < 8; i++) {
            int f = f0 + deltas[i][0];
            int r = r0 + deltas[i][1];
            if (!in_bounds(f, r)) continue;

            uint8_t target = game.board[SQ(f, r)];
            if (TYPE_OF(target) == piece_type && (target & COLOR_BIT) != me)
                return SQ(f, r);
        }
    }

    // bishop + rook + queen
    for (int j = 0; j < 2; j++) {
        const int (*directions)[2] = (j == 0) ? bishop_directions : rook_directions;
        int piece_type = (j == 0) ? W_BISHOP : W_ROOK;
        for (int i = 0; i < 4; i++) {
            int df = directions[i][0];
            int dr = directions[i][1];
            int f = f0 + df;
            int r = r0 + dr;
            while (in_bounds(f, r)) {
                uint8_t target = game.board[SQ(f, r)];
                if (target == EMPTY) {
                    f += df;
                    r += dr;
                    continue;
                }
                if ((target & COLOR_BIT) != me &&
                    (TYPE_OF(target) == piece_type || TYPE_OF(target) == W_QUEEN))
                    return SQ(f, r);
                break;
            }
        }
    }

    // pawns
    int my_dir = me ? 1 : -1;
    for (int df = -1; df <= 1; df += 2) {
        int f = f0 + df;
        int r = r0 + my_dir;
        if (!in_bounds(f, r)) continue;

        uint8_t t = game.board[SQ(f, r)];
        if (t != EMPTY && TYPE_OF(t) == W_PAWN && (t & COLOR_BIT) != me)
            return SQ(f, r);
    }

    return -1;
}

bool is_check(uint8_t color) {
    Side s = (color == BLACK_COLOR) ? SIDE_BLACK : SIDE_WHITE;
    return is_under_attack(game.king[s], color) != -1;
}

static bool can_castle(uint8_t color, bool kingside) {
    int r = color ? 0 : 7;
    int step = kingside ? 1 : -1;
    int castle_type = color ? (kingside ? BK_CASTLE : BQ_CASTLE)
                            : (kingside ? WK_CASTLE : WQ_CASTLE);

    if (!(game.castling & castle_type)) return false;

    int rook_f = kingside ? 7 : 0;
    for (int f = 4 + step; f != rook_f; f += step)
        if (!is_empty(f, r)) return false;

    for (int i = 0; i <= 2; i++)
        if (is_under_attack(SQ(4 + step * i, r), color) != -1)
            return false;

    return true;
}

int possible_moves(PieceType piece, int index, int *move_buf) {
    int piece_type = TYPE_OF(piece);
    int size = 0;
    uint8_t me = piece & COLOR_BIT;
    int pawn_dir = me ? 1 : -1;
    int f0 = FILE_OF(index);
    int r0 = RANK_OF(index);

    switch (piece_type) {
        case W_KING:
        case W_KNIGHT: {
            const int (*deltas)[2] = (piece_type == W_KNIGHT) ? knight_deltas : king_deltas;
            for (int i = 0; i < 8; i++) {
                int f = f0 + deltas[i][0];
                int r = r0 + deltas[i][1];
                if (!in_bounds(f, r)) continue;

                uint8_t target = game.board[SQ(f, r)];
                if (target == EMPTY || (target & COLOR_BIT) != me)
                    move_buf[size++] = SQ(f, r);
            }
            break;
        }
        case W_QUEEN:
        case W_BISHOP:
        case W_ROOK: {
            const int (*deltas)[2] =
                (piece_type == W_BISHOP) ? bishop_directions :
                (piece_type == W_ROOK)   ? rook_directions   : queen_directions;
            int count = (piece_type == W_QUEEN) ? 8 : 4;

            for (int i = 0; i < count; i++) {
                int df = deltas[i][0];
                int dr = deltas[i][1];
                int f = f0 + df;
                int r = r0 + dr;

                while (in_bounds(f, r)) {
                    uint8_t target = game.board[SQ(f, r)];
                    if (target == EMPTY) {
                        move_buf[size++] = SQ(f, r);
                        f += df;
                        r += dr;
                        continue;
                    }
                    if ((target & COLOR_BIT) != me)
                        move_buf[size++] = SQ(f, r);
                    break;
                }
            }
            break;
        }
        case W_PAWN: {
            int start_rank = (pawn_dir == 1) ? 1 : 6;

            // move forward
            if (in_bounds(f0, r0 + pawn_dir) && is_empty(f0, r0 + pawn_dir)) {
                move_buf[size++] = SQ(f0, r0 + pawn_dir);

                if (r0 == start_rank && is_empty(f0, r0 + 2 * pawn_dir))
                    move_buf[size++] = SQ(f0, r0 + 2 * pawn_dir);
            }

            // capture
            for (int df = -1; df <= 1; df += 2) {
                int cf = f0 + df;
                int cr = r0 + pawn_dir;
                if (!in_bounds(cf, cr)) continue;

                int cap = SQ(cf, cr);
                bool normal = !is_empty(cf, cr) && (game.board[cap] & COLOR_BIT) != me;
                bool ep     = is_empty(cf, cr) && cap == game.enpassant;

                if (normal || ep)
                    move_buf[size++] = cap;
            }
            break;
        }
    }
    return size;
}

static bool move_is_safe(int from, int to, uint8_t piece) {
    uint8_t me = piece & COLOR_BIT;
    Side s = piece_side(piece);
    bool is_pawn = TYPE_OF(piece) == W_PAWN;
    bool is_king = TYPE_OF(piece) == W_KING;

    uint8_t captured = game.board[to];
    int old_king = game.king[s];

    // en passant victim is on (FILE_OF(to), RANK_OF(from))
    bool is_ep = is_pawn && to == game.enpassant && captured == EMPTY;
    int victim = SQ(FILE_OF(to), RANK_OF(from));
    uint8_t victim_piece = EMPTY;

    if (is_ep) {
        victim_piece = game.board[victim];
        game.board[victim] = EMPTY;
    }

    game.board[to] = piece;
    game.board[from] = EMPTY;
    if (is_king) game.king[s] = to;

    bool safe = !is_check(me);

    game.board[from] = piece;
    game.board[to] = captured;
    if (is_king) game.king[s] = old_king;
    if (is_ep)   game.board[victim] = victim_piece;

    return safe;
}

int legal_moves_from(int from, int *buf) {
    uint8_t piece = game.board[from];
    uint8_t me = piece & COLOR_BIT;

    int move_buf[64];
    int n = possible_moves(piece, from, move_buf);
    int size = 0;

    for (int i = 0; i < n; i++)
        if (move_is_safe(from, move_buf[i], piece))
            buf[size++] = move_buf[i];

    if (TYPE_OF(piece) == W_KING && from == (me ? SQ(4, 0) : SQ(4, 7))) {
        if (can_castle(me, true))  buf[size++] = from + 2;
        if (can_castle(me, false)) buf[size++] = from - 2;
    }

    return size;
}

static bool has_legal_move(uint8_t color) {
    int buf[64];
    for (int i = 0; i < 64; i++) {
        uint8_t piece = game.board[i];
        if (piece != EMPTY && (piece & COLOR_BIT) == color)
            if (legal_moves_from(i, buf) > 0)
                return true;
    }
    return false;
}

GameStatus game_status(uint8_t color) {
    bool check = is_check(color);
    bool legal_moves = has_legal_move(color);

    if (check && !legal_moves)
        return CHECKMATE;
    else if (!check && !legal_moves)
        return STALEMATE;
    else
        return PLAYING;
}

void make_move(int from, int to) {
    uint8_t piece = game.board[from];
    bool is_pawn = TYPE_OF(piece) == W_PAWN;
    bool is_king = TYPE_OF(piece) == W_KING;
    int from_f = FILE_OF(from), from_r = RANK_OF(from);
    int to_f   = FILE_OF(to),   to_r   = RANK_OF(to);

    // castling: move rook
    if (is_king && abs(to_f - from_f) == 2) {
        if (to_f == 6) {
            game.board[SQ(5, to_r)] = game.board[SQ(7, to_r)];
            game.board[SQ(7, to_r)] = EMPTY;
        } else {
            game.board[SQ(3, to_r)] = game.board[SQ(0, to_r)];
            game.board[SQ(0, to_r)] = EMPTY;
        }
    }

    if (is_king) {
        game.king[piece_side(piece)] = to;
        game.castling &= (piece & COLOR_BIT) ? ~(BK_CASTLE | BQ_CASTLE)
                                             : ~(WK_CASTLE | WQ_CASTLE);
    }

    int touched[2] = { from, to };
    for (int i = 0; i < 2; i++) {
        switch (touched[i]) {
            case SQ(0, 0): game.castling &= ~BQ_CASTLE; break;
            case SQ(7, 0): game.castling &= ~BK_CASTLE; break;
            case SQ(0, 7): game.castling &= ~WQ_CASTLE; break;
            case SQ(7, 7): game.castling &= ~WK_CASTLE; break;
        }
    }

    // en passant capture
    if (is_pawn && to == game.enpassant)
        game.board[SQ(to_f, from_r)] = EMPTY;

    int new_ep = -1;
    if (is_pawn && abs(to_r - from_r) == 2)
        new_ep = SQ(from_f, (from_r + to_r) / 2);

    game.board[to] = piece;
    game.board[from] = EMPTY;
    game.enpassant = new_ep;

    game.turn = (game.turn == SIDE_WHITE) ? SIDE_BLACK : SIDE_WHITE;
    uint8_t next_color = side_to_color(game.turn);
    game.in_check = is_check(next_color);
    game.status   = game_status(next_color);
}
