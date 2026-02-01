#include <rendering.h>
#include <common.h>
#include <lang.h>
#include <ui.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Forward declarations
static void render_fog(Renderer *r, World *w, int screen_x, int screen_y);

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
	v->ramming = false;
	v->ram_returning = false;
	v->ram_origin_x = v->x;
	v->ram_origin_y = v->y;
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

	bool position_done = true;
	bool rotation_done = true;

	// Move toward target position
	float dx = v->target_x - v->x;
	float dy = v->target_y - v->y;

	if (fabsf(dx) > 0.5f)
	{
		float step = (dx > 0) ? speed : -speed;
		if (fabsf(step) > fabsf(dx)) step = dx;
		v->x += step;
		position_done = false;
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
		position_done = false;
	}
	else
	{
		v->y = v->target_y;
	}

	// Handle ram animation phases
	if (v->ramming && position_done)
	{
		if (!v->ram_returning)
		{
			// Reached ram target, now return to origin
			v->ram_returning = true;
			v->target_x = v->ram_origin_x;
			v->target_y = v->ram_origin_y;
			position_done = false;
		}
		else
		{
			// Returned to origin, ram complete
			v->ramming = false;
			v->ram_returning = false;
		}
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
		rotation_done = false;
	}
	else
	{
		v->rotation = v->target_rotation;
	}

	bool done = position_done && rotation_done;
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

void robot_visual_ram(RobotVisual *v, Direction dir)
{
	// Store origin to return to
	v->ram_origin_x = v->x;
	v->ram_origin_y = v->y;

	// Calculate ram target (half a tile in the direction)
	float offset = TILE_SIZE / 2.0f;
	switch (dir)
	{
		case North: v->target_y = v->y - offset; break;
		case South: v->target_y = v->y + offset; break;
		case East:  v->target_x = v->x + offset; break;
		case West:  v->target_x = v->x - offset; break;
	}

	v->ramming = true;
	v->ram_returning = false;
	v->animating = true;
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

void draw_hud(State *state, int fuel, int enemy_count, int level)
{
	char buffer[64];
	int font_size = HUD_FONT_SIZE;
	int y = 2;  // At top of screen

	// Build status line and center it
	snprintf(buffer, sizeof(buffer), "Fuel: %d  Enemies: %d  Level: %d", fuel, enemy_count, level);
	int text_width = MeasureText(buffer, font_size);
	int x = (VIRTUAL_WIDTH - text_width) / 2;
	DrawText(buffer, x, y, font_size, WHITE);

	if (state->program_running)
	{
		snprintf(buffer, sizeof(buffer), "Step: %d", state->stepper->n);
		text_width = MeasureText(buffer, font_size);
		// Draw step counter centered below the status line
		x = (VIRTUAL_WIDTH - text_width) / 2;
		DrawText(buffer, x, y + font_size + 1, font_size, WHITE);
	}
}

Renderer *init_renderer(void)
{
	Renderer *r = malloc(sizeof(Renderer));

	r->target = init_render_target();
	r->tileset = load_tileset("assets/tiles.png");
	r->player_anim = load_animation("assets/robot.gif", ANIM_FPS_ROBOT, true);
	r->enemy_anim = load_animation("assets/enemy.gif", ANIM_FPS_ROBOT, true);

	for (int i = 0; i < MAX_ROBOTS; i++)
	{
		r->disassembly_anims[i] = load_animation("assets/rapid_disassembly.gif", ANIM_FPS_DISASSEMBLY, false);
	}

	r->level = 1;

	// Fog of war
	r->fog_texture = LoadTexture("assets/fog_of_war.png");
	r->fog_scroll = 0.0f;
	for (int i = 0; i < MAX_WORLD_WIDTH * MAX_WORLD_HEIGHT; i++)
	{
		r->fog_map[i] = false;
	}

	// Initialize buttons - positioned at bottom of HUD area
	int start_x = (VIRTUAL_WIDTH - (BTN_WIDTH * BTN_COUNT + BTN_GAP * (BTN_COUNT - 1))) / 2;

	button_init(&r->buttons[BTN_EXECUTE], start_x, BTN_Y, BTN_WIDTH, BTN_HEIGHT, "Execute");
	button_init(&r->buttons[BTN_RESET], start_x + (BTN_WIDTH + BTN_GAP), BTN_Y, BTN_WIDTH, BTN_HEIGHT, "Reset");
	button_init(&r->buttons[BTN_FOG], start_x + (BTN_WIDTH + BTN_GAP) * 2, BTN_Y, BTN_WIDTH, BTN_HEIGHT, "Add Fog");
	button_init(&r->buttons[BTN_QUIT], start_x + (BTN_WIDTH + BTN_GAP) * 3, BTN_Y, BTN_WIDTH, BTN_HEIGHT, "Quit");

	return r;
}

void free_renderer(Renderer *r)
{
	unload_animation(&r->player_anim);
	unload_animation(&r->enemy_anim);
	for (int i = 0; i < MAX_ROBOTS; i++)
	{
		unload_animation(&r->disassembly_anims[i]);
	}
	unload_tileset(&r->tileset);
	UnloadTexture(r->fog_texture);
	UnloadRenderTexture(r->target);
	free(r);
}

void renderer_sync_visuals(Renderer *r, State *state)
{
	for (int i = 0; i < state->robot_count; i++)
	{
		Robot *robot = &state->robots[i];
		robot_visual_init(&r->visuals[i], robot->x, robot->y, robot->dir);
	}
}

void renderer_update(Renderer *r, State *state, float speed)
{
	// Update sprite animations
	update_animation(&r->player_anim);
	update_animation(&r->enemy_anim);
	for (int i = 0; i < state->robot_count; i++)
	{
		update_animation(&r->disassembly_anims[i]);
	}

	// Update robot visual animations (smooth movement/rotation)
	for (int i = 0; i < state->robot_count; i++)
	{
		robot_visual_update(&r->visuals[i], speed);
	}

	// Update fog scroll (slow scroll to the left)
	r->fog_scroll += FOG_SCROLL_SPEED;
	if (r->fog_scroll >= TILE_SIZE)
	{
		r->fog_scroll -= TILE_SIZE;
	}
}

void renderer_render(Renderer *r, State *state)
{
	begin_virtual_drawing(r->target);

	int offset_x = (VIRTUAL_WIDTH - state->world->width * TILE_SIZE) / 2;
	// Center world between top text and buttons
	int world_height = state->world->height * TILE_SIZE;
	int offset_y = HUD_TOP_MARGIN + (BTN_Y - HUD_TOP_MARGIN - world_height) / 2;
	render_world(&r->tileset, state->world, offset_x, offset_y);
	render_robots(state, r->visuals, &r->player_anim, &r->enemy_anim, offset_x, offset_y);
	render_fog(r, state->world, offset_x, offset_y);

	// Calculate enemy count for HUD
	int enemy_count = 0;
	Robot *player = NULL;
	for (int i = 0; i < state->robot_count; i++)
	{
		if (state->robots[i].is_player)
		{
			player = &state->robots[i];
		}
		else if (!robot_visual_is_disassembled(&r->visuals[i]))
		{
			enemy_count++;
		}
	}

	int fuel = player ? player->fuel : 0;
	draw_hud(state, fuel, enemy_count, r->level);

	// Draw buttons
	for (int i = 0; i < BTN_COUNT; i++)
	{
		button_draw(&r->buttons[i]);
	}

	/* Debug information */
	#if DEBUG_GAME
	int size = 8;
	int y = 0;
	DrawText(TextFormat("FPS: %d", GetFPS()), 0, y, size, RED); y += size;
	DrawText(TextFormat("Pos: %d,%d", player->x, player->y), 0, y, size, RED); y += size;
	#endif

	end_virtual_drawing(r->target);
}

RobotVisual *renderer_get_visual(Renderer *r, int index)
{
	return &r->visuals[index];
}

void renderer_set_fog(Renderer *r, int x, int y, bool fogged)
{
	if (x >= 0 && x < MAX_WORLD_WIDTH && y >= 0 && y < MAX_WORLD_HEIGHT)
	{
		r->fog_map[y * MAX_WORLD_WIDTH + x] = fogged;
	}
}

bool renderer_get_fog(Renderer *r, int x, int y)
{
	if (x >= 0 && x < MAX_WORLD_WIDTH && y >= 0 && y < MAX_WORLD_HEIGHT)
	{
		return r->fog_map[y * MAX_WORLD_WIDTH + x];
	}
	return true; // Out of bounds is fogged
}

void renderer_clear_fog(Renderer *r)
{
	for (int i = 0; i < MAX_WORLD_WIDTH * MAX_WORLD_HEIGHT; i++)
	{
		r->fog_map[i] = false;
	}
}

void renderer_fill_fog(Renderer *r, World *w)
{
	for (int y = 0; y < w->height && y < MAX_WORLD_HEIGHT; y++)
	{
		for (int x = 0; x < w->width && x < MAX_WORLD_WIDTH; x++)
		{
			r->fog_map[y * MAX_WORLD_WIDTH + x] = true;
		}
	}
}

static void render_fog(Renderer *r, World *w, int screen_x, int screen_y)
{
	int scroll_offset = (int)r->fog_scroll;

	for (int y = 0; y < w->height; y++)
	{
		for (int x = 0; x < w->width; x++)
		{
			if (!renderer_get_fog(r, x, y)) continue;

			int px = screen_x + x * TILE_SIZE;
			int py = screen_y + y * TILE_SIZE;

			// Draw greyed out tile underneath
			int tile = *get_tile(w, x, y);
			int src_x = (tile % r->tileset.cols) * TILE_SIZE;
			int src_y = (tile / r->tileset.cols) * TILE_SIZE;
			Rectangle src = { src_x, src_y, TILE_SIZE, TILE_SIZE };
			Vector2 pos = { px, py };
			DrawTextureRec(r->tileset.texture, src, pos, DARKGRAY);

			// Draw scrolling fog texture with alpha blending
			// Use two draws to create seamless scrolling
			Rectangle fog_src1 = { scroll_offset, 0, TILE_SIZE - scroll_offset, TILE_SIZE };
			Vector2 fog_pos1 = { px, py };

			Rectangle fog_src2 = { 0, 0, scroll_offset, TILE_SIZE };
			Vector2 fog_pos2 = { px + TILE_SIZE - scroll_offset, py };

			Color fog_color = { 255, 255, 255, FOG_ALPHA };
			DrawTextureRec(r->fog_texture, fog_src1, fog_pos1, fog_color);
			if (scroll_offset > 0)
			{
				DrawTextureRec(r->fog_texture, fog_src2, fog_pos2, fog_color);
			}
		}
	}
}

void renderer_update_buttons(Renderer *r)
{
	float scale = (float)SCREEN_WIDTH / (float)VIRTUAL_WIDTH;
	for (int i = 0; i < BTN_COUNT; i++)
	{
		button_update(&r->buttons[i], scale);
	}
}

bool renderer_button_clicked(Renderer *r, int button_id)
{
	if (button_id < 0 || button_id >= BTN_COUNT) return false;
	return button_clicked(&r->buttons[button_id]);
}
