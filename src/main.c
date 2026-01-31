#include <stdlib.h>
#include <raylib.h>
#include <common.h>
#include <lang.h>

int main(void)
{
	// InitWindow(800, 600, "robots");
	// SetTargetFPS(60);

	State state = {0};
	LangContext ctx = new_context();

	char *program = read_program();
	if (program)
	{
		interpret(&state, &ctx, program);
		free(program);
	}

	del_context(&ctx);

	// while (!WindowShouldClose())
	// {
	// 	BeginDrawing();
	// 	ClearBackground(WHITE);

	// 	DrawFPS(0, 0);

	// 	EndDrawing();
	// }

	// CloseWindow();
}
