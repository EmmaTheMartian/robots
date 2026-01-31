#include <raylib.h>
#include <common.h>

int main(void)
{
	InitWindow(800, 600, "robots");
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(WHITE);

		DrawFPS(0, 0);

		EndDrawing();
	}

	CloseWindow();
}
