#include <common.h>
#include <stdlib.h>
#include <time.h>

static int dummy_tile = 0;

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

/* Local function to check if the position is within the size of the World
 * Input/Pre-Condition: Needs the x & y coordinates
 * Output/Post-Condition: Returns a bool for whether or not it's in the world's size
*/
bool in_bounds(int x, int y) { return x >= 0 && x < WORLD_SIZE && y >= 0 && y < WORLD_SIZE; }


/* Returns the tile in a given position
 * Input/Pre-Condition: Takes the World and the given x & y positions
 * Output/Post-Condition: returns the tile within the given World
*/
int *get_tile(World *w, int x, int y)
{
    // only return a position if it's within the WORLD_SIZE
    if (in_bounds(x, y))
    {
        return &w->tiles[y * WORLD_SIZE + x];
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


/* Make a new robot
 * Input/Pre-Condition: Need to know if the robot is a player or not
 * Output/Post-Condition: Will return a Robot with the default values
*/
Robot new_robot(bool is_player, int x, int y, Direction dir)
{
    Robot r;
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
void robot_foward(Robot *r)
{
    // get the Robot's current facing direction
    Direction current_dir = (*r).dir;

    // move based on the robot's current facing direction
    switch(current_dir)
    {
        case North:
            // move up 1
            (*r).y += 1;
            break;
        case East:
            // move right 1
            (*r).x += 1;
            break;
        case South:
            // move down 1
            (*r).y -= 1;
            break;
        case West:
            // move left 1
            (*r).x -= 1;
            break;
        default:
            // do nothing
    }
}

/* Make the Robot go backwards
 * Input/Pre-Condition: Needs the Robot that's moving
 * Output/Post-Condition: The Robot's x & y position will get updated based on the Direction its facing
*/
void robot_backward(Robot *r)
{
    // get the Robot's current facing direction
    Direction current_dir = (*r).dir;

    // move based on the robot's current facing direction
    switch(current_dir)
    {
        case North:
            // move down 1
            (*r).y -= 1;
            break;
        case East:
            // move left 1
            (*r).x -= 1;
            break;
        case South:
            // move up 1
            (*r).y += 1;
            break;
        case West:
            // move right 1
            (*r).x += 1;
            break;
        default:
            // do nothing
    }
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
 * Output/Post-Condition: The Robot's fuel will be set to 0
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
    // get the Robot's current facing direction
    Direction current_dir = (*r).dir;

    // get the current directions
    int curr_x, curr_y;
    curr_x = (*r).x;
    curr_y = (*r).y;

    return get_tile_with_offset(w, curr_x, curr_y, current_dir);

    // commented out for now
    // int next_x, next_y;

    // // get the position of the tile in front of the robot
    // switch(current_dir)
    // {
    //     case North:
    //         // look up 1
    //         next_y = curr_y + 1;
    //         break;
    //     case East:
    //         // look right 1
    //         next_x = curr_x + 1;
    //         break;
    //     case South:
    //         // look down 1
    //         next_y = curr_y - 1;
    //         break;
    //     case West:
    //         // look left 1
    //         next_x = curr_x - 1;
    //         break;
    //     default:
    //         // do nothing
    // }
}
