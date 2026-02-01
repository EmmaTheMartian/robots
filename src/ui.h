#ifndef __robots_ui__
#define __robots_ui__

#include <raylib.h>
#include <stdbool.h>

#define MAX_BUTTON_TEXT 32

extern const Color BTN_BG, BTN_HOVER, BTN_BORDER, BTN_FLASH;

typedef struct
{
	int x, y;
	int width, height;
	char text[MAX_BUTTON_TEXT];
	float flash_timer;
	bool was_clicked;
} Button;

void button_init(Button *btn, int x, int y, int width, int height, const char *text);
void button_update(Button *btn, float scale);
void button_draw(Button *btn);
bool button_clicked(Button *btn);

#endif
