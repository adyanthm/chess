#include <raylib.h>

int main(void){
    const int width = 900;
    const int height = 900;

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(width, height, "Chess");
    SetExitKey(KEY_NULL);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){ 49, 46, 43, 255 });  // warm charcoal color.
        const int ORIGIN = 50;
        const int SQ = 100;
        for (int rank = 0; rank < 8; rank++) {
            for (int square = 0; square < 8; square++) {
                Color c = ((rank + square) % 2 == 0) ? RAYWHITE : BROWN;
                DrawRectangle(ORIGIN + square * SQ, ORIGIN + rank * SQ, SQ, SQ, c);
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
