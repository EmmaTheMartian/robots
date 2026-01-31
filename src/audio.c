#include <audio.h>
#include <stdbool.h>

static Sound sfx[SFX_COUNT];
static Music music;
static bool music_loaded = false;

void init_audio(void)
{
	InitAudioDevice();

	for (int i = 0; i < SFX_COUNT; i++)
	{
		sfx[i].frameCount = 0;
	}
}

void free_audio(void)
{
	if (music_loaded)
	{
		UnloadMusicStream(music);
		music_loaded = false;
	}

	for (int i = 0; i < SFX_COUNT; i++)
	{
		if (sfx[i].frameCount > 0)
		{
			UnloadSound(sfx[i]);
		}
	}

	CloseAudioDevice();
}

void play_music(const char *path)
{
	if (music_loaded)
	{
		StopMusicStream(music);
		UnloadMusicStream(music);
	}

	music = LoadMusicStream(path);
	music.looping = true;
	music_loaded = true;
	SetMusicVolume(music, 1.0f);
	PlayMusicStream(music);
}

void stop_music(void)
{
	if (music_loaded)
	{
		StopMusicStream(music);
	}
}

void update_music(void)
{
	if (music_loaded)
	{
		UpdateMusicStream(music);
	}
}

void load_sfx(int id, const char *path)
{
	if (id >= 0 && id < SFX_COUNT)
	{
		sfx[id] = LoadSound(path);
	}
}

void play_sfx(int id)
{
	if (id >= 0 && id < SFX_COUNT && sfx[id].frameCount > 0)
	{
		PlaySound(sfx[id]);
	}
}
