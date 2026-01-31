# Requirements

## Goal
Controllable robot game with simple scripting engine to drive robots.  Disassemble all other robots

Winning Condition:
  - Enemy robots all disassembled
  - Fuel: resource for both movement AND attack
    - Start with (configurable fuel amount per level) fuel points (oil)
    - Text file to store level definitions
    - Procedurally generated levels
    - Randomly scattered enemies
    - Winning screen
    - Gameover screen [no extra lives... Hell mode all the time...]

Losing Condition:
  - Running out of fuel

## Sprites 16x16
- oil
- enemy robots
- main player robot (45 degree rotations)
- explosion
- sparks
- walls
- floor
- gameover screen
- title screen

## Major Components

### Interpreter
- Simple basic like structure
- 16 registers for scans?

### Rendering Engine
- Decoupled animation
- Sound effects (bonus)
- Maps (16x16 tiles)
- Sprites (16x16 tiles)