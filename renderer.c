#include <raylib.h>
#include "engine.h"
#include "themes.h"

static const Theme *theme = &THEME_GREEN;

static const int ORIGIN = 50;
static const int SQ = 100;

static int selected_square = -1;
static int legal_moves[64];
static int legal_move_count = 0;

Texture2D sheet;
float tile_W, tile_H;

/*
 * Our enum starts with pawn as 1. but the sprite sheet has king as 1.
 * This converts the piece id into the col index in sprite sheet
 */

int type_to_col[] = {-1, 5, 3, 2, 4, 1, 0};

void load_pieces(void){
    sheet = LoadTexture("chess_sheet.png");
    GenTextureMipmaps(&sheet);
    SetTextureFilter(sheet, TEXTURE_FILTER_TRILINEAR);
    tile_W = sheet.width / 6.0f;
    tile_H = sheet.height / 2.0f;
}

static int mouse_to_square(Vector2 mouse){
    int file = (mouse.x - ORIGIN) / SQ;
    int rank = (mouse.y - ORIGIN) / SQ;

    if (mouse.x < ORIGIN || mouse.y < ORIGIN || file < 0 || file > 7 || rank < 0 || rank > 7)
        return -1;
    return rank * 8 + file;
}

static void clear_selection(void){
    selected_square = -1;
    legal_move_count = 0;
}

static bool is_legal_target(int sq) {
    for (int i = 0; i < legal_move_count; i++)
        if (legal_moves[i] == sq) return true;
    return false;
}

void draw_piece(uint8_t piece, int x, int y, int size){
    int col = type_to_col[piece & TYPE_MASK];
    int row = piece & COLOR_BIT ? 1 : 0;

    Rectangle src = { col * tile_W, row * tile_H, tile_W, tile_H };
    Rectangle dst = { x, y, size, size };
    DrawTexturePro(sheet, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

Vector2 idxtovec(int idx){
    return (Vector2){ORIGIN + (idx % 8) * SQ, ORIGIN + (idx / 8) * SQ};
}

static bool is_light_square(int sq) {
    return ((sq / 8) + (sq % 8)) % 2 == 0;
}

static void draw_squares(void){
    for (int sq = 0; sq < 64; sq++) {
        Vector2 pos = idxtovec(sq);
        DrawRectangle(pos.x, pos.y, SQ, SQ, is_light_square(sq) ? theme->light : theme->dark);
    }
}

static void draw_pieces(void) {
    for (int sq = 0; sq < 64; sq++) {
        if (board[sq]){
            Vector2 pos = idxtovec(sq);
            draw_piece(board[sq], pos.x, pos.y, SQ);
        }
    }
}



static void draw_selection(void) {
    if (selected_square < 0) return;

    Vector2 pos = idxtovec(selected_square);
    DrawRectangle(pos.x, pos.y, SQ, SQ, is_light_square(selected_square) ? theme->selection_light : theme->selection_dark);
}

static void handle_mouse(void){
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

    int clicked = mouse_to_square(GetMousePosition());

    if (clicked >= 0 && is_legal_target(clicked)){
        board[clicked] = board[selected_square];
        board[selected_square] = EMPTY;
        clear_selection();
    } else if (clicked >= 0 && board[clicked] != EMPTY){
        selected_square = clicked;
        legal_move_count = possible_moves(board[clicked], clicked, legal_moves);
    } else {
        clear_selection();
    }
}

static void draw_move_indicators(void) {
    for (int i = 0; i < legal_move_count; i++) {
        int sq = legal_moves[i];
        Vector2 p = idxtovec(sq);
        const Vector2 center = (Vector2){ p.x + SQ / 2, p.y + SQ / 2 };
        const Color indicator = is_light_square(sq) ? theme->indicator_light: theme->indicator_dark;
        if (board[sq] == EMPTY)
            DrawCircleV(center, 15, indicator);
        else
            DrawRing(center , SQ/2 - 10, SQ/2, 360, 0, 64, indicator);
    }
}

static void handle_keys(void){
    for (int i = 0; i < THEME_COUNT; i++)
        if (IsKeyPressed(KEY_ONE + i)) theme = THEMES[i];
}

int main(void){
    const int width = 900;
    const int height = 900;

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(width, height, "Chess");
    load_pieces();
    SetExitKey(KEY_NULL);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        handle_keys();
        handle_mouse();
        BeginDrawing();
        ClearBackground(theme->bg);  // warm charcoal color.
        draw_squares();
        draw_selection();
        draw_move_indicators();
        draw_pieces();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
