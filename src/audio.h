#ifndef __robots_audio__
#define __robots_audio__

#include <raylib.h>

// Sound effect IDs
#define SFX_ADVANCING     0
#define SFX_REVERSE       1
#define SFX_ROTATING      2
#define SFX_DISASSEMBLED  3
#define SFX_BEGIN         4
#define SFX_GAMEOVER      5
#define SFX_COUNT         6

void init_audio(void);
void free_audio(void);

void play_music(const char *path);
void stop_music(void);
void update_music(void);

void load_sfx(int id, const char *path);
void play_sfx(int id);

#endif
