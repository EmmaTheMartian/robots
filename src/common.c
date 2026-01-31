#include <common.h>
#include <stdlib.h>
#include <time.h>

static int dummy_tile = 0;

Robot new_robot(bool is_player, int x, int y, Direction dir)
{
	Robot r;
	r.x = x;
	r.y = y;
	r.fuel = 100;
	r.dir = dir;
	r.is_player = is_player;
	return r;
}

void robot_forward(Robot *r)
{
	switch (r->dir)
	{
		case North: r->y--; break;
		case South: r->y++; break;
		case East:  r->x++; break;
		case West:  r->x--; break;
	}
}

void robot_backward(Robot *r)
{
	switch (r->dir)
	{
		case North: r->y++; break;
		case South: r->y--; break;
		case East:  r->x--; break;
		case West:  r->x++; break;
	}
}

void robot_turn_left(Robot *r)
{
	switch (r->dir)
	{
		case North: r->dir = West; break;
		case West:  r->dir = South; break;
		case South: r->dir = East; break;
		case East:  r->dir = North; break;
	}
}

void robot_turn_right(Robot *r)
{
	switch (r->dir)
	{
		case North: r->dir = East; break;
		case East:  r->dir = South; break;
		case South: r->dir = West; break;
		case West:  r->dir = North; break;
	}
}

void robot_refuel(Robot *r, int fuel_amount)
{
	r->fuel += fuel_amount;
}

World *new_world(int width, int height)
{
	World *w = malloc(sizeof(World) + (width * height * sizeof(int)));
	w->width = width;
	w->height = height;
	for (int i = 0; i < width * height; i++)
	{
		w->tiles[i] = 0;
	}
	return w;
}

int *get_tile(World *w, int x, int y)
{
	if (x < 0 || x >= w->width || y < 0 || y >= w->height)
	{
		return &dummy_tile;
	}
	return &w->tiles[y * w->width + x];
}

static bool is_tile_free(World *w, int x, int y)
{
	return *get_tile(w, x, y) == TILE_EMPTY;
}

State *generate_world(int width, int height, int robot_count)
{
	srand((unsigned int)time(NULL));

	State *state = malloc(sizeof(State));
	state->world = new_world(width, height);
	state->robot_count = 0;

	// Add walls around the border
	for (int x = 0; x < width; x++)
	{
		*get_tile(state->world, x, 0) = TILE_WALL;
		*get_tile(state->world, x, height - 1) = TILE_WALL;
	}
	for (int y = 0; y < height; y++)
	{
		*get_tile(state->world, 0, y) = TILE_WALL;
		*get_tile(state->world, width - 1, y) = TILE_WALL;
	}

	// Scatter some random walls (about 10% of interior)
	int interior_tiles = (width - 2) * (height - 2);
	int wall_count = interior_tiles / 10;
	for (int i = 0; i < wall_count; i++)
	{
		int x = 1 + rand() % (width - 2);
		int y = 1 + rand() % (height - 2);
		*get_tile(state->world, x, y) = TILE_WALL;
	}

	// Scatter some energy pickups (about 5% of interior)
	int energy_count = interior_tiles / 20;
	for (int i = 0; i < energy_count; i++)
	{
		int x = 1 + rand() % (width - 2);
		int y = 1 + rand() % (height - 2);
		if (is_tile_free(state->world, x, y))
		{
			*get_tile(state->world, x, y) = TILE_ENERGY;
		}
	}

	// Place robots randomly
	if (robot_count > MAX_ROBOTS)
	{
		robot_count = MAX_ROBOTS;
	}

	for (int i = 0; i < robot_count; i++)
	{
		int x, y;
		int attempts = 0;
		do
		{
			x = 1 + rand() % (width - 2);
			y = 1 + rand() % (height - 2);
			attempts++;
		} while (!is_tile_free(state->world, x, y) && attempts < 100);

		if (attempts < 100)
		{
			Direction dir = rand() % 4;
			Robot r = new_robot(i == 0, x, y, dir);
			state->robots[state->robot_count++] = r;
		}
	}

	return state;
}

void free_state(State *state)
{
	if (state)
	{
		free(state->world);
		free(state);
	}
}
