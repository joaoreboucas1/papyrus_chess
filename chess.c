#include <stdio.h>
#include <raylib.h>

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Chess");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    float square_len = screenWidth / 8;
                    float x = i * square_len;
                    float y = j * square_len;
                    Color square_color;
                    if ((i + j) % 2 == 0) square_color = BLACK; else square_color = WHITE;
                    DrawRectangle(x, y, square_len, square_len, square_color);
                }
            }
        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context
    return 0;
}