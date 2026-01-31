#include <rendering.h>
#include <math.h>
#include <stddef.h>

static float direction_to_angle(Direction dir)
{
	// Sprite faces South by default, so South = 0°
	switch (dir)
	{
		case South: return 0.0f;
		case West:  return 90.0f;
		case North: return 180.0f;
		case East:  return 270.0f;
	}
	return 0.0f;
}

Tileset load_tileset(const char *path)
{
	Tileset ts;
	ts.texture = LoadTexture(path);
	ts.cols = ts.texture.width / TILE_SIZE;
	ts.rows = ts.texture.height / TILE_SIZE;
	return ts;
}

void unload_tileset(Tileset *ts)
{
	UnloadTexture(ts->texture);
}

void render_world(Tileset *ts, World *w, int screen_x, int screen_y)
{
	for (int y = 0; y < w->height; y++)
	{
		for (int x = 0; x < w->width; x++)
		{
			int tile = *get_tile(w, x, y);
			int src_x = (tile % ts->cols) * TILE_SIZE;
			int src_y = (tile / ts->cols) * TILE_SIZE;

			Rectangle src = { src_x, src_y, TILE_SIZE, TILE_SIZE };
			Vector2 pos = { screen_x + x * TILE_SIZE, screen_y + y * TILE_SIZE };

			DrawTextureRec(ts->texture, src, pos, WHITE);
		}
	}
}

RenderTexture2D init_render_target(void)
{
	RenderTexture2D target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
	SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
	return target;
}

void begin_virtual_drawing(RenderTexture2D target)
{
	BeginTextureMode(target);
	ClearBackground(BLACK);
}

void end_virtual_drawing(RenderTexture2D target)
{
	EndTextureMode();

	BeginDrawing();
	ClearBackground(BLACK);

	Rectangle src = { 0, 0, VIRTUAL_WIDTH, -VIRTUAL_HEIGHT };
	Rectangle dst = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	DrawTexturePro(target.texture, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);

	EndDrawing();
}

Animation load_animation(const char *path, int fps, bool looping)
{
	Animation anim;
	int frames = 0;
	anim.image = LoadImageAnim(path, &frames);

	anim.texture = LoadTextureFromImage(anim.image);
	SetTextureFilter(anim.texture, TEXTURE_FILTER_POINT);
	anim.frame_count = frames;
	anim.frame_width = anim.image.width;
	anim.frame_height = anim.image.height;
	anim.current_frame = 0;
	anim.frames_per_update = 60 / fps;
	anim.frame_counter = 0;
	anim.looping = looping;
	anim.finished = false;

	return anim;
}

void unload_animation(Animation *anim)
{
	UnloadTexture(anim->texture);
	UnloadImage(anim->image);
}

void update_animation(Animation *anim)
{
	if (anim->finished) return;

	anim->frame_counter++;
	if (anim->frame_counter >= anim->frames_per_update)
	{
		anim->frame_counter = 0;
		anim->current_frame++;
		if (anim->current_frame >= anim->frame_count)
		{
			if (anim->looping)
			{
				anim->current_frame = 0;
			}
			else
			{
				anim->current_frame = anim->frame_count - 1;
				anim->finished = true;
				return;
			}
		}
		unsigned int offset = anim->frame_width * anim->frame_height * 4 * anim->current_frame;
		UpdateTexture(anim->texture, ((unsigned char *)anim->image.data) + offset);
	}
}

void reset_animation(Animation *anim)
{
	anim->current_frame = 0;
	anim->frame_counter = 0;
	anim->finished = false;
	UpdateTexture(anim->texture, anim->image.data);
}

bool animation_finished(Animation *anim)
{
	return anim->finished;
}

void draw_animation(Animation *anim, int x, int y)
{
	Vector2 pos = { x, y };
	DrawTextureV(anim->texture, pos, WHITE);
}

void draw_animation_rotated(Animation *anim, int x, int y, Direction dir)
{
	float rotation = 0.0f;
	switch (dir)
	{
		case North: rotation = 0.0f; break;
		case East:  rotation = 90.0f; break;
		case South: rotation = 180.0f; break;
		case West:  rotation = 270.0f; break;
	}

	draw_animation_rotated_angle(anim, x, y, rotation);
}

void draw_animation_rotated_angle(Animation *anim, int x, int y, float rotation)
{
	Rectangle src = { 0, 0, anim->frame_width, anim->frame_height };
	Rectangle dst = {
		x + anim->frame_width / 2,
		y + anim->frame_height / 2,
		anim->frame_width,
		anim->frame_height
	};
	Vector2 origin = { anim->frame_width / 2.0f, anim->frame_height / 2.0f };

	DrawTexturePro(anim->texture, src, dst, origin, rotation, WHITE);
}

void robot_visual_init(RobotVisual *v, int grid_x, int grid_y, Direction dir)
{
	v->x = grid_x * TILE_SIZE;
	v->y = grid_y * TILE_SIZE;
	v->rotation = direction_to_angle(dir);
	v->target_x = v->x;
	v->target_y = v->y;
	v->target_rotation = v->rotation;
	v->animating = false;
	v->disassembled = false;
	v->disassembly_anim = NULL;
}

void robot_visual_move_to(RobotVisual *v, int grid_x, int grid_y)
{
	v->target_x = grid_x * TILE_SIZE;
	v->target_y = grid_y * TILE_SIZE;
	v->animating = true;
}

void robot_visual_rotate_to(RobotVisual *v, Direction dir)
{
	v->target_rotation = direction_to_angle(dir);
	v->animating = true;
}

bool robot_visual_update(RobotVisual *v, float speed)
{
	if (!v->animating) return true;

	bool done = true;

	// Move toward target position
	float dx = v->target_x - v->x;
	float dy = v->target_y - v->y;

	if (fabsf(dx) > 0.5f)
	{
		float step = (dx > 0) ? speed : -speed;
		if (fabsf(step) > fabsf(dx)) step = dx;
		v->x += step;
		done = false;
	}
	else
	{
		v->x = v->target_x;
	}

	if (fabsf(dy) > 0.5f)
	{
		float step = (dy > 0) ? speed : -speed;
		if (fabsf(step) > fabsf(dy)) step = dy;
		v->y += step;
		done = false;
	}
	else
	{
		v->y = v->target_y;
	}

	// Handle rotation with wraparound
	float rot_diff = v->target_rotation - v->rotation;
	if (rot_diff > 180.0f) rot_diff -= 360.0f;
	if (rot_diff < -180.0f) rot_diff += 360.0f;

	if (fabsf(rot_diff) > 0.5f)
	{
		float rot_speed = speed * 6.0f;
		float step = (rot_diff > 0) ? rot_speed : -rot_speed;
		if (fabsf(step) > fabsf(rot_diff)) step = rot_diff;
		v->rotation += step;
		if (v->rotation >= 360.0f) v->rotation -= 360.0f;
		if (v->rotation < 0.0f) v->rotation += 360.0f;
		done = false;
	}
	else
	{
		v->rotation = v->target_rotation;
	}

	if (done)
	{
		v->animating = false;
	}

	return done;
}

bool robot_visual_is_animating(RobotVisual *v)
{
	return v->animating;
}

void robot_visual_disassemble(RobotVisual *v, Animation *disassembly_anim)
{
	v->disassembled = true;
	v->disassembly_anim = disassembly_anim;
	reset_animation(disassembly_anim);
}

bool robot_visual_is_disassembled(RobotVisual *v)
{
	return v->disassembled;
}

void render_robots(State *state, RobotVisual *visuals, Animation *player_anim, Animation *enemy_anim, int screen_x, int screen_y)
{
	for (int i = 0; i < state->robot_count; i++)
	{
		Robot *r = &state->robots[i];
		RobotVisual *v = &visuals[i];
		int px = screen_x + (int)v->x;
		int py = screen_y + (int)v->y;

		if (v->disassembled)
		{
			// Don't render if animation finished
			if (v->disassembly_anim && !animation_finished(v->disassembly_anim))
			{
				// Draw without rotation
				draw_animation(v->disassembly_anim, px, py);
			}
		}
		else
		{
			Animation *anim = r->is_player ? player_anim : enemy_anim;
			draw_animation_rotated_angle(anim, px, py, v->rotation);
		}
	}
}
