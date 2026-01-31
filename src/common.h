#ifndef __robots_common__
#define __robots_common__
#include <stdbool.h>

#define WORLD_SIZE 64
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
	int x, y;
	int fuel;
	Direction dir;
	bool is_player;
} Robot;

// World helpers
World new_world();
int *get_tile(World *w, int x, int y);
int *get_tile_with_offset(World *w, int x, int y, Direction d);

// Robot helpers
Robot new_robot(bool is_player);
void robot_foward(Robot *r);
void robot_backward(Robot *r);
void robot_turn_left(Robot *r);
void robot_turn_right(Robot *r);
void robot_refuel(Robot *r, int fuel_amount);
void robot_ram(Robot *r);
int robot_scan(Robot *r, World *w);


typedef struct
{
	int tiles[WORLD_SIZE*WORLD_SIZE];
} World;


typedef struct
{
	World world;
	Robot robots[MAX_ROBOTS];
} State;

#endif