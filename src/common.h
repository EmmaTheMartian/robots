#ifndef __robots_common__
#define __robots_common__


#include <stdbool.h>


#define TILE_EMPTY 0
#define TILE_WALL 1
#define TILE_ENERGY 2

#define MAX_FUEL 50
#define FUEL_CANISTER_AMOUNT 25
#define MAX_ROBOTS 16
#define EXEC_SPEED_SECONDS (0.25f)
#define EXEC_SPEED ((int)(EXEC_SPEED_SECONDS*60)) /* frames per statement executed */

// World generation constants
#define DEFAULT_WORLD_WIDTH 10
#define DEFAULT_WORLD_HEIGHT 8
#define DEFAULT_ROBOT_COUNT 4
#define WALL_DIVISOR 10      // Interior tiles / this = wall count
#define ENERGY_DIVISOR 20    // Interior tiles / this = energy count
#define MAX_PLACEMENT_ATTEMPTS 100

// Window constants
#define VIRTUAL_WIDTH 200
#define VIRTUAL_HEIGHT 200
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800


typedef enum
{
	North,
	South,
	East,
	West,
} Direction;


typedef struct
{
	int width, height;
	int tiles[];
} World;

World *new_world(int width, int height);
bool in_bounds(World *w, int x, int y);
int *get_tile(World *w, int x, int y);
int *get_tile_with_offset(World *w, int x, int y, Direction d);
bool is_tile_free(World *w, int x, int y);


typedef struct
{
	int x, y;
	int fuel;
	Direction dir;
	bool is_player;
	bool is_disassembled;
} Robot;

Robot new_robot(bool is_player, int x, int y, Direction dir);
bool robot_forward(World *w, Robot *r);
bool robot_backward(World *w, Robot *r);
void robot_turn_left(Robot *r);
void robot_turn_right(Robot *r);
void robot_refuel(Robot *r, int fuel_amount);
void robot_ram(Robot *r);
int robot_scan(Robot *r, World *w);
void robot_disassemble(Robot *r);
bool robot_use_fuel(Robot *r, int amount);


typedef struct rbt_stepper LangStepper;
typedef struct
{
	World *world;
	Robot robots[MAX_ROBOTS];
	int robot_count;
	LangStepper *stepper;
	bool program_running;
} State;

State *generate_world(long seed, int width, int height, int robot_count);
void free_state(State *state);

#endif
