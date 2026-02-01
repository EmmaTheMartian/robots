#ifndef __robots_editor__
#define __robots_editor__

#include <raylib.h>
#include <stdbool.h>
#include <ui.h>

#define EDITOR_MAX_TEXT 4096
#define EDITOR_MAX_INPUT 128
#define EDITOR_FONT_SIZE 8
#define EDITOR_LINE_HEIGHT 9

// Editor button indices
#define EDITOR_BTN_SAVE 0
#define EDITOR_BTN_CANCEL 1
#define EDITOR_BTN_COUNT 2

typedef struct
{
	bool active;
	char program[EDITOR_MAX_TEXT];   // The full program text
	char input[EDITOR_MAX_INPUT];    // Line number + replacement text input
	int input_len;
	Button buttons[EDITOR_BTN_COUNT];
	int x, y, width, height;
} Editor;

void editor_init(Editor *e, int x, int y, int width, int height);
void editor_open(Editor *e);
void editor_close(Editor *e);
int editor_update(Editor *e, float scale);  // 0=editing, 1=saved, -1=cancelled
void editor_draw(Editor *e);
bool editor_is_active(Editor *e);

#endif
