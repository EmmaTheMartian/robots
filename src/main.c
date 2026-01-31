#include <raylib.h>
#include <stdlib.h>
#include <common.h>
#include <rendering.h>
#include <audio.h>

int main(void)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "robots");
	SetTargetFPS(60);
	init_audio();
	play_music("assets/music.mp3");

	// Load sound effects
	load_sfx(SFX_ADVANCING, "assets/advancing.mp3");
	load_sfx(SFX_REVERSE, "assets/reverse.mp3");
	load_sfx(SFX_ROTATING, "assets/rotating.mp3");
	load_sfx(SFX_DISASSEMBLED, "assets/disassembled.mp3");
	load_sfx(SFX_BEGIN, "assets/begin.mp3");
	load_sfx(SFX_GAMEOVER, "assets/gameover.mp3");

	RenderTexture2D target = init_render_target();
	Tileset tileset = load_tileset("assets/tiles.png");
	Animation player_anim = load_animation("assets/robot.gif", 10, true);
	Animation enemy_anim = load_animation("assets/enemy.gif", 10, true);
	Animation disassembly_anims[MAX_ROBOTS];
	for (int i = 0; i < MAX_ROBOTS; i++)
	{
		disassembly_anims[i] = load_animation("assets/rapid_disassembly.gif", 15, false);
	}
	State *state = generate_world(10, 8, 4);

	// Initialize visual state for each robot
	RobotVisual visuals[MAX_ROBOTS];
	for (int i = 0; i < state->robot_count; i++)
	{
		Robot *r = &state->robots[i];
		robot_visual_init(&visuals[i], r->x, r->y, r->dir);
	}

	// Movement type tracking for sound effects
	#define MOVE_NONE     0
	#define MOVE_FORWARD  1
	#define MOVE_BACKWARD 2
	#define MOVE_ROTATE   3
	int last_move = MOVE_NONE;

	while (!WindowShouldClose())
	{
		// Update music stream
		update_music();

		// Update sprite animations
		update_animation(&player_anim);
		update_animation(&enemy_anim);
		for (int i = 0; i < state->robot_count; i++)
		{
			update_animation(&disassembly_anims[i]);
		}

		// Update robot visual animations
		for (int i = 0; i < state->robot_count; i++)
		{
			robot_visual_update(&visuals[i], 2.0f);
		}

		// Test input for player robot
		Robot *player = &state->robots[0];
		RobotVisual *player_visual = &visuals[0];
		if (!robot_visual_is_animating(player_visual))
		{
			if (IsKeyPressed(KEY_UP))
			{
				robot_forward(player);
				robot_visual_move_to(player_visual, player->x, player->y);
				if (last_move != MOVE_FORWARD)
				{
					play_sfx(SFX_ADVANCING);
					last_move = MOVE_FORWARD;
				}
			}
			else if (IsKeyPressed(KEY_DOWN))
			{
				robot_backward(player);
				robot_visual_move_to(player_visual, player->x, player->y);
				if (last_move != MOVE_BACKWARD)
				{
					play_sfx(SFX_REVERSE);
					last_move = MOVE_BACKWARD;
				}
			}
			else if (IsKeyPressed(KEY_LEFT))
			{
				robot_turn_left(player);
				robot_visual_rotate_to(player_visual, player->dir);
				if (last_move != MOVE_ROTATE)
				{
					play_sfx(SFX_ROTATING);
					last_move = MOVE_ROTATE;
				}
			}
			else if (IsKeyPressed(KEY_RIGHT))
			{
				robot_turn_right(player);
				robot_visual_rotate_to(player_visual, player->dir);
				if (last_move != MOVE_ROTATE)
				{
					play_sfx(SFX_ROTATING);
					last_move = MOVE_ROTATE;
				}
			}
		}

		// Test: press B to disassemble a random robot
		if (IsKeyPressed(KEY_B))
		{
			int idx = rand() % state->robot_count;
			if (!robot_visual_is_disassembled(&visuals[idx]))
			{
				robot_visual_disassemble(&visuals[idx], &disassembly_anims[idx]);
				play_sfx(SFX_DISASSEMBLED);
			}
		}

		// Test: press R to ram in the player's current direction
		if (IsKeyPressed(KEY_R) && !robot_visual_is_animating(player_visual))
		{
			robot_visual_ram(player_visual, player->dir);
		}

		begin_virtual_drawing(target);

		render_world(&tileset, state->world, 0, 0);
		render_robots(state, visuals, &player_anim, &enemy_anim, 0, 0);

		// Calculate enemy count (non-player, non-disassembled robots)
		int enemy_count = 0;
		for (int i = 0; i < state->robot_count; i++)
		{
			if (!state->robots[i].is_player && !robot_visual_is_disassembled(&visuals[i]))
			{
				enemy_count++;
			}
		}
		draw_hud(player->fuel, enemy_count, 1);

		end_virtual_drawing(target);
	}

	stop_music();
	free_audio();
	free_state(state);
	unload_animation(&player_anim);
	unload_animation(&enemy_anim);
	for (int i = 0; i < MAX_ROBOTS; i++)
	{
		unload_animation(&disassembly_anims[i]);
	}
	unload_tileset(&tileset);
	UnloadRenderTexture(target);
	CloseWindow();
}
