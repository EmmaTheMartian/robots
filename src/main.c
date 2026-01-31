#include <raylib.h>
#include <stdlib.h>
#include <common.h>
#include <rendering.h>
#include <audio.h>

#ifdef RENDER_TEST
#include <render_test.h>
#endif

static void init_window(void)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "robots");
	SetTargetFPS(60);
}

static void init_sound(void)
{
	init_audio();
	play_music("assets/music.mp3");

	load_sfx(SFX_ADVANCING, "assets/advancing.mp3");
	load_sfx(SFX_REVERSE, "assets/reverse.mp3");
	load_sfx(SFX_ROTATING, "assets/rotating.mp3");
	load_sfx(SFX_DISASSEMBLED, "assets/disassembled.mp3");
	load_sfx(SFX_BEGIN, "assets/begin.mp3");
	load_sfx(SFX_GAMEOVER, "assets/gameover.mp3");
	load_sfx(SFX_REFUELING, "assets/refueling.mp3");
	load_sfx(SFX_AWAITING_INSTRUCTIONS, "assets/awaiting_instructions.mp3");
	load_sfx(SFX_BEGIN_EXECUTION, "assets/begin_execution.mp3");
}

static void shutdown_sound(void)
{
	stop_music();
	free_audio();
}

int main(void)
{
	init_window();
	init_sound();

	Renderer *renderer = init_renderer();
	State *state = generate_world(10, 8, 4);
	renderer_sync_visuals(renderer, state);

	while (!WindowShouldClose())
	{
		update_music();
		renderer_update(renderer, state, 2.0f);

#ifdef RENDER_TEST
		render_test_logic(renderer, state);
#endif

		renderer_render(renderer, state);
	}

	shutdown_sound();
	free_state(state);
	free_renderer(renderer);
	CloseWindow();
}
