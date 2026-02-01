#ifndef __robots_rendering__
#define __robots_rendering__

#include <raylib.h>
#include <common.h>
#include <ui.h>
#include <editor.h>

#define TILE_SIZE 16

// UI layout constants
#define HUD_FONT_SIZE 8
#define HUD_TOP_MARGIN 10
#define BTN_WIDTH 46
#define BTN_HEIGHT 14
#define BTN_Y 182
#define BTN_GAP 2

// Animation constants
#define ANIM_FPS_ROBOT 10
#define ANIM_FPS_DISASSEMBLY 15
#define ANIM_SPEED 2.0f
#define FOG_SCROLL_SPEED 0.2f
#define FOG_ALPHA 180

typedef struct
{
	Texture2D texture;
	int cols;
	int rows;
} Tileset;

Tileset load_tileset(const char *path);
void unload_tileset(Tileset *ts);
void render_world(Tileset *ts, World *w, int screen_x, int screen_y);

RenderTexture2D init_render_target(void);
void begin_virtual_drawing(RenderTexture2D target);
void end_virtual_drawing(RenderTexture2D target);

typedef struct
{
	Image image;
	Texture2D texture;
	int frame_count;
	int frame_width;
	int frame_height;
	int current_frame;
	int frames_per_update;
	int frame_counter;
	bool looping;
	bool finished;
} Animation;

Animation load_animation(const char *path, int fps, bool looping);
void unload_animation(Animation *anim);
void update_animation(Animation *anim);
void reset_animation(Animation *anim);
bool animation_finished(Animation *anim);
void draw_animation(Animation *anim, int x, int y);
void draw_animation_rotated(Animation *anim, int x, int y, Direction dir);
void draw_animation_rotated_angle(Animation *anim, int x, int y, float rotation);

// Visual state for robots (decoupled from game logic)
typedef struct
{
	float x, y;
	float rotation;
	float target_x, target_y;
	float target_rotation;
	bool animating;
	bool disassembled;
	Animation *disassembly_anim;
	// Ram animation state
	bool ramming;
	bool ram_returning;
	float ram_origin_x, ram_origin_y;
} RobotVisual;

void robot_visual_init(RobotVisual *v, int grid_x, int grid_y, Direction dir);
void robot_visual_move_to(RobotVisual *v, int grid_x, int grid_y);
void robot_visual_rotate_to(RobotVisual *v, Direction dir);
bool robot_visual_update(RobotVisual *v, float speed);
bool robot_visual_is_animating(RobotVisual *v);
void robot_visual_disassemble(RobotVisual *v, Animation *disassembly_anim);
bool robot_visual_is_disassembled(RobotVisual *v);
void robot_visual_ram(RobotVisual *v, Direction dir);

void render_robots(State *state, RobotVisual *visuals, Animation *player_anim, Animation *enemy_anim, int screen_x, int screen_y);

void draw_hud(State *state, int fuel, int enemy_count, int level);

// Maximum world size for fog map
#define MAX_WORLD_WIDTH 16
#define MAX_WORLD_HEIGHT 16

// Button indices
#define BTN_EXECUTE 0
#define BTN_RESET 1
#define BTN_EDIT 2
#define BTN_QUIT 3
#define BTN_COUNT 4

// Renderer context - encapsulates all rendering state
typedef struct
{
	RenderTexture2D target;
	Tileset tileset;
	Animation player_anim;
	Animation enemy_anim;
	Animation disassembly_anims[MAX_ROBOTS];
	RobotVisual visuals[MAX_ROBOTS];
	int level;
	// Fog of war
	Texture2D fog_texture;
	float fog_scroll;
	bool fog_map[MAX_WORLD_WIDTH * MAX_WORLD_HEIGHT];
	// UI buttons
	Button buttons[BTN_COUNT];
	// Editor
	Editor editor;
	// Notification
	char *notification_msg; /* when NULL, no notification is shown. */
} Renderer;

Renderer *init_renderer(void);
void free_renderer(Renderer *r);
void renderer_sync_visuals(Renderer *r, State *state);
void renderer_update(Renderer *r, State *state, float speed);
void renderer_render(Renderer *r, State *state);
RobotVisual *renderer_get_visual(Renderer *r, int index);

// Fog of war functions
void renderer_set_fog(Renderer *r, int x, int y, bool fogged);
bool renderer_get_fog(Renderer *r, int x, int y);
void renderer_clear_fog(Renderer *r);
void renderer_fill_fog(Renderer *r, World *w);

// UI functions
void renderer_update_buttons(Renderer *r);
bool renderer_button_clicked(Renderer *r, int button_id);

// Editor functions
void renderer_open_editor(Renderer *r);
void renderer_close_editor(Renderer *r);
bool renderer_editor_active(Renderer *r);
int renderer_update_editor(Renderer *r);

// Notification functions
void renderer_set_notif(Renderer *r, char *msg); /* duplicates the given string */
void renderer_clear_notif(Renderer *r);

#endif
