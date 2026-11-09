#include <raylib.h>
#include "engine.h"

static const int ORIGIN = 50;
static const int SQ = 100;
static const Color BG = { 49, 46, 43, 255 };

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

void draw_piece(uint8_t piece, int x, int y, int size){
    int col = type_to_col[piece & TYPE_MASK];
    int row = piece & COLOR_BIT ? 1 : 0;

    Rectangle src = { col * tile_W, row * tile_H, tile_W, tile_H };
    Rectangle dst = { x, y, size, size };
    DrawTexturePro(sheet, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

static Vector2 idxtovec(int idx){
    return (Vector2){ORIGIN + (idx % 8) * SQ, ORIGIN + (idx / 8) * SQ};
}

static void draw_squares(void){
    for (int sq = 0; sq < 64; sq++) {
        Vector2 pos = idxtovec(sq);
        Color c = (((sq / 8) + (sq % 8)) % 2 == 0) ? RAYWHITE : BROWN;
        DrawRectangle(pos.x, pos.y, SQ, SQ, c);
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

int main(void){
    const int width = 900;
    const int height = 900;

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(width, height, "Chess");
    load_pieces();
    SetExitKey(KEY_NULL);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BG);  // warm charcoal color.
        draw_squares();
        draw_pieces();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
