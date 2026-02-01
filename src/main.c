#include <stdlib.h>
#include <raylib.h>
#include <common.h>
#include <rendering.h>
#include <audio.h>
#include <lang.h>

#ifdef RENDER_TEST
#include <render_test.h>
#endif

// Game states
typedef enum
{
	GAME_TITLE,
	GAME_PLAYING,
	GAME_OVER
} GameState;

static void init_window(void)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Rapid Disassembly");
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

static void draw_title_screen(Renderer *renderer, Texture2D *texture)
{
	begin_virtual_drawing(renderer->target);

	// Draw title image centered
	int title_x = (VIRTUAL_WIDTH - texture->width) / 2;
	int title_y = (VIRTUAL_HEIGHT - texture->height) / 2 - 20;
	DrawTexture(*texture, title_x, title_y, WHITE);

	// Draw title text at bottom
	const char *line1 = "Rapid Disassembly";
	const char *line2 = "HackMT 2026";
	int line1_width = MeasureText(line1, 8);
	int line2_width = MeasureText(line2, 8);
	DrawText(line1, (VIRTUAL_WIDTH - line1_width) / 2, VIRTUAL_HEIGHT - 28, 8, WHITE);
	DrawText(line2, (VIRTUAL_WIDTH - line2_width) / 2, VIRTUAL_HEIGHT - 18, 8, WHITE);

	end_virtual_drawing(renderer->target);
}

static void draw_gameover_screen(Renderer *renderer, Texture2D *texture)
{
	begin_virtual_drawing(renderer->target);

	// Draw game over image centered
	int go_x = (VIRTUAL_WIDTH - texture->width) / 2;
	int go_y = (VIRTUAL_HEIGHT - texture->height) / 2 - 20;
	DrawTexture(*texture, go_x, go_y, WHITE);

	// Draw "Game Over" text
	const char *go_text = "Game Over";
	int text_width = MeasureText(go_text, 10);
	DrawText(go_text, (VIRTUAL_WIDTH - text_width) / 2, VIRTUAL_HEIGHT - 25, 10, WHITE);

	end_virtual_drawing(renderer->target);
}

int main(void)
{
	init_window();
	init_sound();

	Renderer *renderer = init_renderer();
	State *state = NULL;
	GameState game_state = GAME_TITLE;
	unsigned long frame = 0, program_frame = 0;
	bool running = true;

	Texture2D title_texture = LoadTexture("assets/title.png");
	Texture2D gameover_texture = LoadTexture("assets/gameover.png");

	while (running && !WindowShouldClose())
	{
		frame++;

		update_music();

		switch (game_state)
		{
			case GAME_TITLE:
			{
				// Any key or click advances to game
				if (GetKeyPressed() != 0 || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
				{
					// Initialize game state
					if (state != NULL)
					{
						free_state(state);
					}
					state = generate_world(DEFAULT_WORLD_WIDTH, DEFAULT_WORLD_HEIGHT, DEFAULT_ROBOT_COUNT);
					renderer_sync_visuals(renderer, state);

					play_sfx(SFX_AWAITING_INSTRUCTIONS);
					game_state = GAME_PLAYING;

					// TODO:  Additional initialization?
				}

				draw_title_screen(renderer, &title_texture);
			}
			break;

			case GAME_PLAYING:
			{
				renderer_update(renderer, state, ANIM_SPEED);

				// Handle editor if active
				if (renderer_editor_active(renderer))
				{
					int editor_result = renderer_update_editor(renderer);
					if (editor_result == 1)
					{
						// Saved - reload the program
						stepper_reload(state->stepper);
					}
					// If editor is active, skip game logic and button updates
					renderer_render(renderer, state);
					break;
				}

				renderer_update_buttons(renderer);

#ifdef RENDER_TEST
				render_test_logic(renderer, state);
#endif

				// TODO: Additional game logic here for processing instructions, etc.
				if (
					state->program_running &&
					++program_frame % EXEC_SPEED == 0 &&
					!stepper_step(state, state->stepper, renderer)
				)
				{
					state->program_running = false;
					program_frame = 0;
				}

				// Handle button clicks
				if (renderer_button_clicked(renderer, BTN_EXECUTE))
				{
					state->program_running = !state->program_running;
					if (state->program_running)
						stepper_reload(state->stepper);
					program_frame = 0;
				}

				if (renderer_button_clicked(renderer, BTN_RESET))
				{
					// Reset level - regenerate with same parameters
					free_state(state);
					state = generate_world(DEFAULT_WORLD_WIDTH, DEFAULT_WORLD_HEIGHT, DEFAULT_ROBOT_COUNT);
					renderer_sync_visuals(renderer, state);
					renderer_clear_fog(renderer);
					play_sfx(SFX_AWAITING_INSTRUCTIONS);
				}

				if (renderer_button_clicked(renderer, BTN_EDIT))
				{
					// Open the program editor
					renderer_open_editor(renderer);
				}

				if (renderer_button_clicked(renderer, BTN_QUIT))
				{
					running = false;
				}

				// Check for game over condition
				Robot *player = &state->robots[0];
				if (player->fuel <= 0)
				{
					play_sfx(SFX_GAMEOVER);
					game_state = GAME_OVER;
				}

				// TODO: Check for level complete condition
				int alive_enemies = 0;

				// count alive enemies
				for (int i = 0; i < (*state).robot_count; i++)
				{
					if (!(*state).robots[i].is_player &&
						(*state).robots[i].fuel > 0 &&
						!(*state).robots[i].is_disassembled)
					{
						alive_enemies++;
					}
				}

				// player has won the level
				if (alive_enemies == 0)
				{
					// go to the next level
					int next_level = (*renderer).level + 1;

					// Initialize NEW game state
					free_state(state);

					state = generate_world(10, 8, 4);
					(*renderer).level = next_level;
					renderer_sync_visuals(renderer, state);
				}

				renderer_render(renderer, state);
			}
			break;

			case GAME_OVER:
			{
				// Any key or click returns to title
				if (GetKeyPressed() != 0 || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
				{
					game_state = GAME_TITLE;
				}

				draw_gameover_screen(renderer, &gameover_texture);
			}
			break;
		}
	}

	// Cleanup
	UnloadTexture(title_texture);
	UnloadTexture(gameover_texture);
	shutdown_sound();
	if (state != NULL)
	{
		free_state(state);
	}
	free_renderer(renderer);
	CloseWindow();
}
