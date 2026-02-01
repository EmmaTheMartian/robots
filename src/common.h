#ifndef __robots_common__
#define __robots_common__


#include <stdbool.h>


#define TILE_EMPTY 0
#define TILE_WALL 1
#define TILE_ENERGY 2

#define MAX_FUEL 100
#define MAX_ROBOTS 16


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
} Robot;

Robot new_robot(bool is_player, int x, int y, Direction dir);
void robot_forward(Robot *r);
void robot_backward(Robot *r);
void robot_turn_left(Robot *r);
void robot_turn_right(Robot *r);
void robot_refuel(Robot *r, int fuel_amount);
void robot_ram(Robot *r);
int robot_scan(Robot *r, World *w);


typedef struct
{
	World *world;
	Robot robots[MAX_ROBOTS];
	int robot_count;
} State;

State *generate_world(int width, int height, int robot_count);
void free_state(State *state);

#endif
