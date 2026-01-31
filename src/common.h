#ifndef __robots_common__
#define __robots_common__


#include <stdbool.h>


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


Robot new_robot(bool is_player);
void robot_forward(Robot *r);
void robot_backward(Robot *r);
void robot_turn_left(Robot *r);
void robot_turn_right(Robot *r);
void robot_refuel(Robot *r, int fuel_amount);
void robot_ram(Robot *r);
int robot_scan(Robot *r);


typedef struct
{
	int width, height;
	int tiles[];
} World;


World new_world(int width, int height);
int *get_tile(World *w, int x, int y);


typedef struct
{
	World world;
	Robot robots[16];
} State;


#endif
