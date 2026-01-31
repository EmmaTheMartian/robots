# Audio Engine Documentation

This document describes how game logic developers can use the audio system for music and sound effects.

## Overview

The audio engine provides:
- Background music playback (streaming, looped)
- Sound effects triggered by game events

## Quick Start

```c
#include <audio.h>

// Initialize
init_audio();
play_music("assets/music.mp3");

// Load sound effects
load_sfx(SFX_ADVANCING, "assets/advancing.mp3");
load_sfx(SFX_DISASSEMBLED, "assets/disassembled.mp3");

// Game loop
while (!WindowShouldClose())
{
    update_music();

    // Play sound effects based on game events
    if (player_moved)
    {
        play_sfx(SFX_ADVANCING);
    }
}

// Cleanup
stop_music();
free_audio();
```

## Core Functions

### Initialization

#### `void init_audio(void)`
Initializes the audio device. Call once at startup before any other audio functions.

```c
init_audio();
```

### Cleanup

#### `void free_audio(void)`
Unloads all sound effects, stops music, and closes the audio device. Call once at shutdown.

```c
free_audio();
```

## Music Functions

### `void play_music(const char *path)`
Loads and plays background music from the specified file. Music loops automatically. If music is already playing, it will be stopped and replaced.

Supported formats: MP3, OGG, WAV, FLAC

```c
play_music("assets/music.mp3");
```

### `void stop_music(void)`
Stops the currently playing music.

```c
stop_music();
```

### `void update_music(void)`
Updates the music stream buffer. **Must be called once per frame** to keep music playing smoothly.

```c
while (!WindowShouldClose())
{
    update_music();  // Call every frame
    // ... rest of game loop
}
```

## Sound Effects

### Loading Sound Effects

#### `void load_sfx(int id, const char *path)`
Loads a sound effect from file and associates it with the given ID. Call during initialization for each sound effect.

```c
load_sfx(SFX_ADVANCING, "assets/advancing.mp3");
load_sfx(SFX_REVERSE, "assets/reverse.mp3");
```

### Playing Sound Effects

#### `void play_sfx(int id)`
Plays the sound effect associated with the given ID. Sound effects can overlap (multiple sounds can play simultaneously).

```c
play_sfx(SFX_ADVANCING);
```

## Predefined Sound Effect IDs

The following IDs are predefined in `audio.h`:

| ID | Constant | Description |
|----|----------|-------------|
| 0 | `SFX_ADVANCING` | Robot moving forward |
| 1 | `SFX_REVERSE` | Robot moving backward |
| 2 | `SFX_ROTATING` | Robot turning |
| 3 | `SFX_DISASSEMBLED` | Robot destroyed |
| 4 | `SFX_BEGIN` | Level/game start |
| 5 | `SFX_GAMEOVER` | Game over |
| 6 | `SFX_REFUELING` | Robot refueling/collecting energy |
| 7 | `SFX_AWAITING_INSTRUCTIONS` | Waiting for player input |
| 8 | `SFX_BEGIN_EXECUTION` | Starting to execute commands |

The maximum number of sound effects is defined by `SFX_COUNT` (currently 9).

## Typical Initialization Pattern

```c
static void init_sound(void)
{
    init_audio();
    play_music("assets/music.mp3");

    load_sfx(SFX_ADVANCING, "assets/advancing.mp3");
    load_sfx(SFX_REVERSE, "assets/reverse.mp3");
    load_sfx(SFX_ROTATING, "assets/rotating.mp3");
    load_sfx(SFX_DISASSEMBLED, "assets/disassembled.mp3");
    load_sfx(SFX_BEGIN, "assets/begin.mp3");
    load_sfx(SFX_GAMEOVER, "assets/gameover.mp3");
    load_sfx(SFX_REFUELING, "assets/refueling.mp3");
    load_sfx(SFX_AWAITING_INSTRUCTIONS, "assets/awaiting_instructions.mp3");
    load_sfx(SFX_BEGIN_EXECUTION, "assets/begin_execution.mp3");
}

static void shutdown_sound(void)
{
    stop_music();
    free_audio();
}
```

## Typical Usage in Game Logic

```c
// Play sound when robot moves forward
if (IsKeyPressed(KEY_UP))
{
    robot_forward(player);
    robot_visual_move_to(player_visual, player->x, player->y);
    play_sfx(SFX_ADVANCING);
}

// Play sound when robot is destroyed
if (robot_destroyed)
{
    robot_visual_disassemble(visual, anim);
    play_sfx(SFX_DISASSEMBLED);
}

// Play sound when robot collects energy
if (collected_energy)
{
    robot_refuel(player, fuel_amount);
    play_sfx(SFX_REFUELING);
}

// Play sound at game start
play_sfx(SFX_BEGIN);

// Play sound at game over
play_sfx(SFX_GAMEOVER);

// Play sound when waiting for player input (programming phase)
play_sfx(SFX_AWAITING_INSTRUCTIONS);

// Play sound when starting to execute programmed commands
play_sfx(SFX_BEGIN_EXECUTION);
```

## Notes

- `update_music()` must be called every frame or music will stutter/stop.
- Sound effects are fire-and-forget; they play to completion automatically.
- Multiple sound effects can play simultaneously.
- Music automatically loops until `stop_music()` is called.
- All audio files should be placed in the `assets/` directory.
- Supported audio formats: MP3, OGG, WAV, FLAC (via raylib).
