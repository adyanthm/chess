#include <raylib.h>
#include "engine.h"
#include "themes.h"

#define ANIM_TIME 0.20 // in seconds

static const Theme *theme = &THEME_GREEN;

static const int ORIGIN = 50;
static const int TILE_SIZE = 100;

static int selected_square = -1;
static int legal_moves[64];
static int legal_move_count = 0;

static bool promoting = false;
static Move pending_move;

static Texture2D sheet;
static float tile_W, tile_H;

static const int type_to_col[] = {-1, 5, 3, 2, 4, 1, 0};

extern const unsigned char _binary_chess_sheet_png_start[];
extern const unsigned char _binary_chess_sheet_png_end[];

// animations
typedef struct { int from, to; uint8_t piece; double start; bool active; } Anim;
static Anim anim;

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

// ----- Begin animation helpers.

static float ease_out_cubic(float t) { float u = 1 - t; return 1 - u * u * u; }

static void play_move(Move m) {
    anim = (Anim){ m.from, m.to, game.board[m.from], GetTime(), true };
    make_move(m);
}

static void draw_pieces(void) {
    float t = 1.0f;
    if (anim.active) {
        t = (float)((GetTime() - anim.start) / ANIM_TIME);
        if (t >= 1.0f) anim.active = false;
    }

    for (int sq = 0; sq < 64; sq++) {
        if (game.board[sq] == EMPTY) continue;
        if (anim.active && sq == anim.to) continue;   // drawn separately below
        Vector2 pos = idxtovec(sq);
        draw_piece(game.board[sq], pos.x, pos.y, TILE_SIZE);
    }

    if (anim.active) {
        Vector2 a = idxtovec(anim.from), b = idxtovec(anim.to);
        float e = ease_out_cubic(t);
        draw_piece(anim.piece, a.x + (b.x - a.x) * e, a.y + (b.y - a.y) * e, TILE_SIZE);
    }
}

// ---- End animation helpers. -----

static void draw_selection(void) {
    if (selected_square < 0) return;

    Vector2 pos = idxtovec(selected_square);
    DrawRectangle(pos.x, pos.y, TILE_SIZE, TILE_SIZE,
                  is_light_square(selected_square) ? theme->selection_light : theme->selection_dark);
}

// Promotion picker -> center strip of 4 tiles
static Rectangle promo_rect(int i) {
    float w = TILE_SIZE * 4;
    const int BOARD_CENTER = ORIGIN + 4 * TILE_SIZE;
    return (Rectangle){
        BOARD_CENTER - w / 2 + i * TILE_SIZE,
        BOARD_CENTER - TILE_SIZE / 2,
        TILE_SIZE, TILE_SIZE
    };
}

static Rectangle promo_panel(void) {
    const float PAD = 20;
    const float TITLE_H = 45;
    Rectangle first = promo_rect(0);
    return (Rectangle){
        first.x - PAD,
        first.y - PAD - TITLE_H,
        TILE_SIZE * 4 + PAD * 2,
        TILE_SIZE + PAD * 2 + TITLE_H
    };
}

static void draw_promotion(void) {
    if (!promoting) return;

    // dim the board
    DrawRectangle(ORIGIN, ORIGIN, TILE_SIZE * 8, TILE_SIZE * 8, (Color){0, 0, 0, 150});

    Rectangle panel = promo_panel();

    // shadow
    Rectangle shadow = { panel.x + 6, panel.y + 8, panel.width, panel.height };
    DrawRectangleRounded(shadow, 0.08f, 12, (Color){0, 0, 0, 120});

    // border -> then panel on top (border = slightly larger rounded rect)
    Rectangle border = { panel.x - 3, panel.y - 3, panel.width + 6, panel.height + 6 };
    DrawRectangleRounded(border, 0.08f, 12, theme->dark);
    DrawRectangleRounded(panel, 0.08f, 12, (Color){38, 36, 33, 255});

    // title -> centered in the title area
    const char *title = "Promote to";
    int fs = 35;
    int tw = MeasureText(title, fs);
    DrawText(title, panel.x + (panel.width - tw) / 2, panel.y + 14, fs, RAYWHITE);

    // 4 choices
    Vector2 mouse = GetMousePosition();
    uint8_t color_bit = pending_move.piece & COLOR_BIT;

    for (int i = 0; i < 4; i++) {
        Rectangle r = promo_rect(i);
        bool hover = CheckCollisionPointRec(mouse, r);
        Color tile_bg = IS_BLACK(color_bit) ? theme->dark : theme-> light;
        Color tile_lines = IS_BLACK(color_bit) ? theme->light : theme-> dark;
        DrawRectangleRec(r, hover ? theme->selection_light : tile_bg);
        DrawRectangleLinesEx(r, 2, tile_lines);
        draw_piece(promo_types[i] | color_bit, r.x, r.y, TILE_SIZE);
    }
}


static void handle_mouse(void) {
    if (game.status != PLAYING) return;

    // picker open: it takes all input, the board is blocked
    if (promoting) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            promoting = false;
            clear_selection();
            return;
        }
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

        Vector2 mouse = GetMousePosition();

        for (int i = 0; i < 4; i++) {
            if (CheckCollisionPointRec(mouse, promo_rect(i))) {
                pending_move.piece = promo_types[i] | (pending_move.piece & COLOR_BIT);
                play_move(pending_move);
                promoting = false;
                clear_selection();
                return;
            }
        }

        // clicked outside the picker: cancel
        promoting = false;
        clear_selection();
        return;
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

    int clicked = mouse_to_square(GetMousePosition());

    if (clicked >= 0 && is_legal_target(clicked)) {
        Move move = {
            .from = selected_square,
            .to = clicked,
            .piece = game.board[selected_square]
        };
        if (is_promotion_move(move.from, move.to)) {
            pending_move = move;
            promoting = true;
        } else {
            play_move(move);
            clear_selection();
        }
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

    SetTargetFPS(60); // 60 fps since animations.

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
        draw_promotion();
        draw_status();
        EndDrawing();
    }

    UnloadTexture(sheet);
    CloseWindow();
    return 0;
}
