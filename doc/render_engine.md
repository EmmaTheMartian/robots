# Render Engine Documentation

This document describes how game logic developers can use the rendering system without needing to understand the underlying implementation details.

## Overview

The rendering engine handles all visual aspects of the game:
- Window and render target management
- Sprite animations (player, enemy, disassembly effects)
- Smooth robot movement and rotation
- World tile rendering
- HUD display

Game logic code interacts with the renderer through a small set of functions, keeping visual concerns separate from game rules.

## Quick Start

```c
#include <rendering.h>

// Initialize
Renderer *renderer = init_renderer();
State *state = generate_world(10, 8, 4);
renderer_sync_visuals(renderer, state);

// Game loop
while (!WindowShouldClose())
{
    renderer_update(renderer, state, 2.0f);

    // ... game logic here ...

    renderer_render(renderer, state);
}

// Cleanup
free_renderer(renderer);
```

## Core Functions

### Initialization

#### `Renderer *init_renderer(void)`
Creates and initializes the rendering context. Loads all assets (tileset, animations). Call once at startup.

```c
Renderer *renderer = init_renderer();
```

#### `void renderer_sync_visuals(Renderer *r, State *state)`
Synchronizes visual state with game state. Call after creating or resetting the game world to position all robot visuals at their logical positions.

```c
State *state = generate_world(10, 8, 4);
renderer_sync_visuals(renderer, state);
```

### Per-Frame Functions

#### `void renderer_update(Renderer *r, State *state, float speed)`
Updates all animations and visual movements. Call once per frame before rendering.

- `speed` - Movement interpolation speed (recommended: `2.0f`)

```c
renderer_update(renderer, state, 2.0f);
```

#### `void renderer_render(Renderer *r, State *state)`
Renders the complete frame: world, robots, and HUD. Call once per frame after update and game logic.

```c
renderer_render(renderer, state);
```

### Cleanup

#### `void free_renderer(Renderer *r)`
Frees all rendering resources. Call once at shutdown.

```c
free_renderer(renderer);
```

## Robot Visual Actions

To trigger visual actions on robots, first get the robot's visual state, then call the appropriate action function.

### Getting Robot Visuals

#### `RobotVisual *renderer_get_visual(Renderer *r, int index)`
Returns a pointer to the visual state for the robot at the given index in `state->robots[]`.

```c
RobotVisual *player_visual = renderer_get_visual(renderer, 0);
RobotVisual *enemy_visual = renderer_get_visual(renderer, 1);
```

### Movement

#### `void robot_visual_move_to(RobotVisual *v, int grid_x, int grid_y)`
Triggers smooth movement animation to the specified grid position. Call after updating the robot's logical position.

```c
robot_forward(player);  // Update logical position
robot_visual_move_to(player_visual, player->x, player->y);  // Animate
```

### Rotation

#### `void robot_visual_rotate_to(RobotVisual *v, Direction dir)`
Triggers smooth rotation animation to face the specified direction. Call after updating the robot's logical direction.

```c
robot_turn_left(player);  // Update logical direction
robot_visual_rotate_to(player_visual, player->dir);  // Animate
```

### Ram Attack

#### `void robot_visual_ram(RobotVisual *v, Direction dir)`
Triggers a ram animation: the robot lunges forward half a tile in the specified direction, then returns to its original position.

```c
robot_visual_ram(player_visual, player->dir);
```

### Disassembly

#### `void robot_visual_disassemble(RobotVisual *v, Animation *disassembly_anim)`
Triggers the disassembly (destruction) animation. The robot will play the disassembly animation and then disappear.

```c
robot_visual_disassemble(visual, &renderer->disassembly_anims[robot_index]);
```

### State Queries

#### `bool robot_visual_is_animating(RobotVisual *v)`
Returns `true` if the robot is currently animating (moving, rotating, or ramming). Use this to prevent new actions while an animation is in progress.

```c
if (!robot_visual_is_animating(player_visual))
{
    // Safe to start a new action
}
```

#### `bool robot_visual_is_disassembled(RobotVisual *v)`
Returns `true` if the robot has been disassembled.

```c
if (!robot_visual_is_disassembled(visual))
{
    // Robot is still active
}
```

## Renderer Properties

The `Renderer` struct exposes some properties that game logic may need:

| Property | Type | Description |
|----------|------|-------------|
| `level` | `int` | Current level number (displayed in HUD) |
| `disassembly_anims` | `Animation[]` | Per-robot disassembly animations |

```c
renderer->level = 5;  // Update HUD level display
```

## Typical Game Loop Pattern

```c
while (!WindowShouldClose())
{
    // 1. Update audio
    update_music();

    // 2. Update renderer (animations, visual movements)
    renderer_update(renderer, state, 2.0f);

    // 3. Handle input and game logic
    Robot *player = &state->robots[0];
    RobotVisual *player_visual = renderer_get_visual(renderer, 0);

    if (!robot_visual_is_animating(player_visual))
    {
        if (IsKeyPressed(KEY_UP))
        {
            robot_forward(player);
            robot_visual_move_to(player_visual, player->x, player->y);
        }
        // ... other input handling
    }

    // 4. Render frame
    renderer_render(renderer, state);
}
```

## Constants

Defined in `rendering.h`:

| Constant | Value | Description |
|----------|-------|-------------|
| `TILE_SIZE` | 16 | Size of one tile in pixels |
| `VIRTUAL_WIDTH` | 160 | Virtual canvas width |
| `VIRTUAL_HEIGHT` | 160 | Virtual canvas height |
| `SCREEN_WIDTH` | 800 | Window width |
| `SCREEN_HEIGHT` | 800 | Window height |

## Notes

- The virtual canvas (160x160) is scaled up to the screen size (800x800) using nearest-neighbor filtering for a crisp pixel art look.
- The world area is 10x8 tiles (160x128 pixels), leaving 32 pixels at the bottom for the HUD.
- All visual actions are non-blocking. The renderer smoothly interpolates positions/rotations over multiple frames.
- Always check `robot_visual_is_animating()` before starting new movement/rotation actions to prevent animation conflicts.
