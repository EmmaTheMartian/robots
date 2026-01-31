#ifdef RENDER_TEST

#include <raylib.h>
#include <stdlib.h>
#include <render_test.h>
#include <audio.h>

#define MOVE_NONE     0
#define MOVE_FORWARD  1
#define MOVE_BACKWARD 2
#define MOVE_ROTATE   3

void render_test_logic(Renderer *renderer, State *state)
{
	static int last_move = MOVE_NONE;

	Robot *player = &state->robots[0];
	RobotVisual *player_visual = renderer_get_visual(renderer, 0);

	// Test movement input
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

	// Test: press B to disassemble a random robot that hasn't been disassembled
	if (IsKeyPressed(KEY_B))
	{
		int available[MAX_ROBOTS];
		int available_count = 0;

		for (int i = 0; i < state->robot_count; i++)
		{
			if (!robot_visual_is_disassembled(renderer_get_visual(renderer, i)))
			{
				available[available_count++] = i;
			}
		}

		if (available_count > 0)
		{
			int idx = available[rand() % available_count];
			RobotVisual *v = renderer_get_visual(renderer, idx);
			robot_visual_disassemble(v, &renderer->disassembly_anims[idx]);
			play_sfx(SFX_DISASSEMBLED);
		}
	}

	// Test: press R to ram in the player's current direction
	if (IsKeyPressed(KEY_R) && !robot_visual_is_animating(player_visual))
	{
		robot_visual_ram(player_visual, player->dir);
	}
}

#endif /* RENDER_TEST */
