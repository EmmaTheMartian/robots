#include <stdlib.h>
#include <time.h>
#include <common.h>
#include <lang.h>

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

	// Scatter some random walls
	int interior_tiles = (width - 2) * (height - 2);
	int wall_count = interior_tiles / WALL_DIVISOR;
	for (int i = 0; i < wall_count; i++)
	{
		int x = 1 + rand() % (width - 2);
		int y = 1 + rand() % (height - 2);
		*get_tile(state->world, x, y) = TILE_WALL;
	}

	// Scatter some energy pickups
	int energy_count = interior_tiles / ENERGY_DIVISOR;
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

    for (int i = 0 ; i < MAX_ROBOTS ; i++)
        state->robots[i] = (Robot){0}; /* initialize robots to zero */

	for (int i = 0; i < robot_count; i++)
	{
		int x, y;
		int attempts = 0;
		bool position_valid;
		do
		{
			x = 1 + rand() % (width - 2);
			y = 1 + rand() % (height - 2);
			attempts++;

			// Check if tile is free and no robot is already there
			position_valid = is_tile_free(state->world, x, y);
			if (position_valid)
			{
				for (int j = 0; j < state->robot_count; j++)
				{
					if (state->robots[j].x == x && state->robots[j].y == y)
					{
						position_valid = false;
						break;
					}
				}
			}
		} while (!position_valid && attempts < MAX_PLACEMENT_ATTEMPTS);

		if (attempts < MAX_PLACEMENT_ATTEMPTS)
		{
			Direction dir = rand() % 4;
			Robot r = new_robot(i == 0, x, y, dir);
			state->robots[state->robot_count++] = r;
		}
	}

    /* Create language context */
    state->stepper = make_stepper(0, NULL);
    state->program_running = false;

	return state;
}

void free_state(State *state)
{
	if (state)
	{
		del_stepper(state->stepper);
		free(state->world);
		free(state);
	}
}

/* Local function to check if the position is within the size of the World
 * Input/Pre-Condition: Needs the x & y coordinates
 * Output/Post-Condition: Returns a bool for whether or not it's in the world's size
*/
bool in_bounds(World *w, int x, int y) { return x >= 0 && x < w->width && y >= 0 && y < w->height; }


/* Returns the tile in a given position
 * Input/Pre-Condition: Takes the World and the given x & y positions
 * Output/Post-Condition: returns the tile within the given World
*/
int *get_tile(World *w, int x, int y)
{
    // only return a position if it's within the WORLD_SIZE
    if (in_bounds(w, x, y))
    {
        return &w->tiles[y * w->width + x];
    }
    else
    {
        return NULL;
    }
}


/* Gets the tile that's in front of the Robot based on original position
 * Input/Pre-Condition: Needs the World, the x & y coordinates, and direction of the object
 * Output/Post-Condition: Returns the tile that's in front of the object
*/
int *get_tile_with_offset(World *w, int x, int y, Direction d)
{
    switch (d)
    {
    case North:
        y--;
        break;
    case South:
        y++;
        break;
    case East:
        x++;
        break;
    case West:
        x--;
        break;
    }
    return get_tile(w, x, y);
}

bool is_tile_free(World *w, int x, int y)
{
	return *get_tile(w, x, y) == TILE_EMPTY;
}


/* Make a new robot
 * Input/Pre-Condition: Need to know if the robot is a player or not
 * Output/Post-Condition: Will return a Robot with the default values
*/
Robot new_robot(bool is_player, int x, int y, Direction dir)
{
    Robot r = {0};
    r.x = x;
    r.y = y;
    r.fuel = MAX_FUEL;
    r.dir = dir;
    r.is_player = is_player;
    return r;
}

/* Make the Robot go forward
 * Input/Pre-Condition: Needs the Robot that's moving
 * Output/Post-Condition: The Robot's x & y position will get updated based on the Direction its facing
*/
bool robot_forward(World *w, Robot *r)
{
    // get the Robot's current facing direction
    Direction current_dir = (*r).dir;

    int x = r->x;
    int y = r->y;

    // move based on the robot's current facing direction
    switch(current_dir)
    {
        case North:
            // move up 1
            y--;
            break;
        case East:
            // move right 1
            x++;
            break;
        case South:
            // move down 1
            y++;
            break;
        case West:
            // move left 1
            x--;
            break;
        default:
            // do nothing
    }

    if (*get_tile(w, x, y) != TILE_WALL)
    {
        r->x = x;
        r->y = y;
        return true;
    }

    return false;
}

/* Make the Robot go backwards
 * Input/Pre-Condition: Needs the Robot that's moving
 * Output/Post-Condition: The Robot's x & y position will get updated based on the Direction its facing
*/
bool robot_backward(World *w, Robot *r)
{
    // get the Robot's current facing direction
    Direction current_dir = (*r).dir;

    int x = r->x;
    int y = r->y;

    // move based on the robot's current facing direction
    switch(current_dir)
    {
        case North:
            // move down 1
            y++;
            break;
        case East:
            // move left 1
            x--;
            break;
        case South:
            // move up 1
            y--;
            break;
        case West:
            // move right 1
            x++;
            break;
        default:
            // do nothing
    }

    if (*get_tile(w, x, y) != TILE_WALL)
    {
        r->x = x;
        r->y = y;
        return true;
    }

    return false;
}

/* Make the Robot pivot left
 * Input/Pre-Condition: Takes the given robot
 * Output/Post-Condition: Changes the direction based on it's current direction
*/
void robot_turn_left(Robot *r)
{
    // get the Robot's current facing direction
    Direction current_dir = (*r).dir;

    // move based on the robot's current facing direction
    switch(current_dir)
    {
        case North:
            (*r).dir = West;
            break;
        case East:
            (*r).dir = North;
            break;
        case South:
            (*r).dir = East;
            break;
        case West:
            (*r).dir = South;
            break;
        default:
            // do nothing
    }
}


/* Make the Robot pivot right
 * Input/Pre-Condition: Takes the given Robot
 * Output/Post-Condition: Changes the position based on the current position
*/
void robot_turn_right(Robot *r)
{
    // get the Robot's current facing direction
    Direction current_dir = (*r).dir;

    // move based on the robot's current facing direction
    switch(current_dir)
    {
        case North:
            (*r).dir = East;
            break;
        case East:
            (*r).dir = South;
            break;
        case South:
            (*r).dir = West;
            break;
        case West:
            (*r).dir = North;
            break;
        default:
            // do nothing
    }
}


/* Refuel the robot the specified amount
 * Input/Pre-Condition: Takes a pointer to a Robot and the amount to refuel
 * Output/Post-Condition: Fill the robot to the given fuel_amount
*/
void robot_refuel(Robot *r, int fuel_amount)
{
    (*r).fuel += fuel_amount;

    // can't go over the MAX_FUEL amount
    if ((*r).fuel > MAX_FUEL)
    {
        (*r).fuel = MAX_FUEL;
    }
}


/* Player Robot attacks the other Robot
 * Input/Pre-Condition: Need the Robot that's being attacked
 * Output/Post-Condition: The Robot's fuel will be set to 0, subtract from the total robots
*/
void robot_ram(Robot *r)
{
    if (!(*r).is_player)
    {
        (*r).fuel = 0;
    }
}


/* Get the Robot's position and scan the tile in front of it and get that tile ID
 * Input/Pre-Condition: Get the Robot's current position
 * Output/Post-Condition: Returns the tile in front of the Robot
*/
int robot_scan(Robot *r, World *w)
{
    return *get_tile_with_offset(w, r->x, r->y, r->dir);
}


/* Get the Robot's position and scan the tile in front of it and get that tile ID
 * Input/Pre-Condition: Get the Robot's current position
 * Output/Post-Condition: Returns the tile in front of the Robot
*/
void robot_disassemble(Robot *r)
{
    (*r).is_disassembled = true;
}


/* Use the given amount of fuel
 * Input/Pre-Condition: Needs the Robut and how much fuel to take
 * Output/Post-Condition: Returns true if the fuel is above 0 and false if not
*/
bool robot_use_fuel(Robot *r, int amount)
{
    // handle invalid amounts
    if (amount <= 0)
    {
        return true;
    }

    // bot is out of fuel
    if ((*r).fuel <= 0)
    {
        return false;
    }

    // use amount
    (*r).fuel -= amount;
    if ((*r).fuel < 0)
    {
        // can't have negative fuel
        (*r).fuel = 0;
    }

    // return whether it's out or not
    return ((*r).fuel > 0);
}