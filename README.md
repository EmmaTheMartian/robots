# Rapid Disassembly

A robot programming game built with raylib for HackMT 2026

## Setup

First time setup (downloads and builds raylib):

```sh
./init.sh
```

## Building and Running

```sh
./run.sh [options]
```

This compiles the source files and immediately runs the game.

### Options

| Option | Description |
|--------|-------------|
| (none) | Build and run in normal mode |
| `--test` or `-t` | Build with render test mode enabled |

### Examples

```sh
# Normal build
./run.sh

# Build with test controls enabled (arrow keys, B, R)
./run.sh --test
```

### Test Mode Controls

When built with `--test`, the following controls are available:

| Key | Action |
|-----|--------|
| Arrow Up | Move robot forward |
| Arrow Down | Move robot backward |
| Arrow Left | Turn robot left |
| Arrow Right | Turn robot right |
| R | Ram attack |
| B | Disassemble a random enemy robot |
| F | Set fuel to zero (trigger game over) |

## Documentation

- [Render Engine](doc/render_engine.md) - How to use the rendering system
- [Audio Engine](doc/audio_engine.md) - How to use the audio system
