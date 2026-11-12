#include <raylib.h>
#include "engine.h"
#include "themes.h"

static const Theme *theme = &THEME_GREEN;

static const int ORIGIN = 50;
static const int TILE_SIZE = 100;

static int selected_square = -1;
static int legal_moves[64];
static int legal_move_count = 0;

static Texture2D sheet;
static float tile_W, tile_H;

static const int type_to_col[] = {-1, 5, 3, 2, 4, 1, 0};

extern const unsigned char _binary_chess_sheet_png_start[];
extern const unsigned char _binary_chess_sheet_png_end[];

static void load_pieces(void) {
    Image img = LoadImageFromMemory(
        ".png",
        _binary_chess_sheet_png_start,
        (int)(_binary_chess_sheet_png_end - _binary_chess_sheet_png_start)
    );

    sheet = LoadTextureFromImage(img);
    UnloadImage(img);

    GenTextureMipmaps(&sheet);
    SetTextureFilter(sheet, TEXTURE_FILTER_TRILINEAR);

    tile_W = sheet.width / 6.0f;
    tile_H = sheet.height / 2.0f;
}

static int mouse_to_square(Vector2 mouse) {
    if (mouse.x < ORIGIN || mouse.y < ORIGIN) return -1;
    int f = (mouse.x - ORIGIN) / TILE_SIZE;
    int r = (mouse.y - ORIGIN) / TILE_SIZE;
    return in_bounds(f, r) ? SQ(f, r) : -1;
}

static void clear_selection(void) {
    selected_square = -1;
    legal_move_count = 0;
}

static bool is_legal_target(int sq) {
    for (int i = 0; i < legal_move_count; i++)
        if (legal_moves[i] == sq) return true;
    return false;
}

static bool is_turn_piece(uint8_t piece) {
    return piece != EMPTY && piece_side(piece) == game.turn;
}

static void draw_piece(uint8_t piece, int x, int y, int size) {
    int col = type_to_col[TYPE_OF(piece)];
    int row = IS_BLACK(piece) ? 1 : 0;

    Rectangle src = { col * tile_W, row * tile_H, tile_W, tile_H };
    Rectangle dst = { x, y, size, size };
    DrawTexturePro(sheet, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

static Vector2 idxtovec(int idx) {
    return (Vector2){
        ORIGIN + FILE_OF(idx) * TILE_SIZE,
        ORIGIN + RANK_OF(idx) * TILE_SIZE
    };
}

static bool is_light_square(int sq) {
    return (FILE_OF(sq) + RANK_OF(sq)) % 2 == 0;
}

static void draw_squares(void) {
    for (int sq = 0; sq < 64; sq++) {
        Vector2 pos = idxtovec(sq);
        DrawRectangle(pos.x, pos.y, TILE_SIZE, TILE_SIZE,
                      is_light_square(sq) ? theme->light : theme->dark);
    }
}

static void draw_pieces(void) {
    for (int sq = 0; sq < 64; sq++) {
        if (game.board[sq] != EMPTY) {
            Vector2 pos = idxtovec(sq);
            draw_piece(game.board[sq], pos.x, pos.y, TILE_SIZE);
        }
    }
}

static void draw_selection(void) {
    if (selected_square < 0) return;

    Vector2 pos = idxtovec(selected_square);
    DrawRectangle(pos.x, pos.y, TILE_SIZE, TILE_SIZE,
                  is_light_square(selected_square) ? theme->selection_light : theme->selection_dark);
}

static void handle_mouse(void) {
    if (game.status != PLAYING) return;
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

    int clicked = mouse_to_square(GetMousePosition());

    if (clicked >= 0 && is_legal_target(clicked)) {
        make_move(selected_square, clicked);
        clear_selection();
    } else if (clicked >= 0 && is_turn_piece(game.board[clicked])) {
        selected_square = clicked;
        legal_move_count = legal_moves_from(clicked, legal_moves);
    } else {
        clear_selection();
    }
}

static void draw_move_indicators(void) {
    for (int i = 0; i < legal_move_count; i++) {
        int sq = legal_moves[i];
        Vector2 p = idxtovec(sq);
        const Vector2 center = { p.x + TILE_SIZE / 2, p.y + TILE_SIZE / 2 };
        const Color indicator = is_light_square(sq) ? theme->indicator_light : theme->indicator_dark;
        if (game.board[sq] == EMPTY)
            DrawCircleV(center, 15, indicator);
        else
            DrawRing(center, TILE_SIZE / 2 - 10, TILE_SIZE / 2, 360, 0, 64, indicator);
    }
}

static void draw_check_highlight(void) {
    if (!game.in_check) return;
    Vector2 p = idxtovec(game.king[game.turn]);
    const Color HIGHLIGHTED = {255, 0, 0, 123};
    DrawRectangle(p.x, p.y, TILE_SIZE, TILE_SIZE, HIGHLIGHTED);
}

static void handle_keys(void) {
    for (int i = 0; i < THEME_COUNT; i++)
        if (IsKeyPressed(KEY_ONE + i)) theme = THEMES[i];
}

static void draw_status(void) {
    const char *msg;
    bool white_turn = (game.turn == SIDE_WHITE);
    if (game.status == CHECKMATE)
        msg = white_turn ? "Checkmate - Black wins" : "Checkmate - White wins";
    else if (game.status == STALEMATE)
        msg = "Stalemate - draw";
    else if (game.in_check)
        msg = white_turn ? "White is in check" : "Black is in check";
    else
        msg = white_turn ? "White to move" : "Black to move";
    DrawText(msg, 50, 10, 30, RAYWHITE);
}

int main(void){
    const int width = 900;
    const int height = 900;

    SetTraceLogLevel(LOG_NONE);
    InitWindow(width, height, "Chess");
    load_pieces();
    SetExitKey(KEY_NULL);

    SetTargetFPS(20);

    while (!WindowShouldClose()) {
        handle_keys();
        handle_mouse();
        BeginDrawing();
        ClearBackground(theme->bg);
        draw_squares();
        draw_check_highlight();
        draw_selection();
        draw_move_indicators();
        draw_pieces();
        draw_status();
        EndDrawing();
    }

    UnloadTexture(sheet);
    CloseWindow();
    return 0;
}
