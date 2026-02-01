#include "editor.h"
#include <lang.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Colors matching ui.c palette
static const Color EDITOR_BG = { 20, 35, 60, 255 };
static const Color EDITOR_BORDER = { 140, 190, 215, 255 };
static const Color EDITOR_TEXT = { 200, 220, 240, 255 };

void editor_init(Editor *e, int x, int y, int width, int height)
{
	e->active = false;
	e->program[0] = '\0';
	e->input[0] = '\0';
	e->input_len = 0;
	e->x = x;
	e->y = y;
	e->width = width;
	e->height = height;

	// Buttons at bottom
	int btn_width = 40;
	int btn_height = 12;
	int btn_y = y + height - btn_height - 4;
	int btn_gap = 4;
	int btn_start_x = x + (width - btn_width * 2 - btn_gap) / 2;

	button_init(&e->buttons[EDITOR_BTN_SAVE], btn_start_x, btn_y, btn_width, btn_height, "Save");
	button_init(&e->buttons[EDITOR_BTN_CANCEL], btn_start_x + btn_width + btn_gap, btn_y, btn_width, btn_height, "Cancel");
}

void editor_open(Editor *e)
{
	char *program = read_program();
	if (program)
	{
		strncpy(e->program, program, EDITOR_MAX_TEXT - 1);
		e->program[EDITOR_MAX_TEXT - 1] = '\0';
		free(program);
	}
	else
	{
		e->program[0] = '\0';
	}
	e->input[0] = '\0';
	e->input_len = 0;
	e->active = true;
}

void editor_close(Editor *e)
{
	e->active = false;
}

// Count lines in program
static int count_lines(const char *program)
{
	int count = 0;
	if (*program) count = 1;
	while (*program)
	{
		if (*program == '\n') count++;
		program++;
	}
	return count;
}

// Replace or add a line in the program text
static void replace_line(char *program, int line_num, const char *new_text)
{
	int total_lines = count_lines(program);

	// If line doesn't exist, append new lines until we reach it
	if (line_num > total_lines)
	{
		int len = strlen(program);
		while (total_lines < line_num)
		{
			if (len > 0 && program[len - 1] != '\n')
			{
				if (len < EDITOR_MAX_TEXT - 1)
					program[len++] = '\n';
			}
			else if (len < EDITOR_MAX_TEXT - 1)
			{
				program[len++] = '\n';
			}
			total_lines++;
		}
		// Add the new text
		int new_len = strlen(new_text);
		if (len + new_len < EDITOR_MAX_TEXT)
		{
			memcpy(program + len, new_text, new_len + 1);
		}
		return;
	}

	// Find the start of the target line
	int current_line = 1;
	char *line_start = program;

	while (current_line < line_num && *line_start)
	{
		if (*line_start == '\n')
			current_line++;
		line_start++;
	}

	// Find end of this line
	char *line_end = line_start;
	while (*line_end && *line_end != '\n')
		line_end++;

	// Build new program: before + new_text + after
	int before_len = line_start - program;
	int after_len = strlen(line_end);
	int new_len = strlen(new_text);

	if (before_len + new_len + after_len >= EDITOR_MAX_TEXT)
		return; // Too long

	char temp[EDITOR_MAX_TEXT];
	memcpy(temp, program, before_len);
	memcpy(temp + before_len, new_text, new_len);
	memcpy(temp + before_len + new_len, line_end, after_len + 1);
	strcpy(program, temp);
}

// Delete a line from the program
static void delete_line(char *program, int line_num)
{
	int total_lines = count_lines(program);
	if (line_num > total_lines || line_num < 1)
		return;

	// Find the start of the target line
	int current_line = 1;
	char *line_start = program;

	while (current_line < line_num && *line_start)
	{
		if (*line_start == '\n')
			current_line++;
		line_start++;
	}

	// Find end of this line (including newline)
	char *line_end = line_start;
	while (*line_end && *line_end != '\n')
		line_end++;
	if (*line_end == '\n')
		line_end++;

	// Shift everything after the line back
	memmove(line_start, line_end, strlen(line_end) + 1);
}

int editor_update(Editor *e, float scale)
{
	if (!e->active)
		return 0;

	// Update buttons
	for (int i = 0; i < EDITOR_BTN_COUNT; i++)
		button_update(&e->buttons[i], scale);

	// Apply edit on Enter key
	if (IsKeyPressed(KEY_ENTER) && e->input_len > 0)
	{
		int line_num = atoi(e->input);
		char *space = strchr(e->input, ' ');
		if (line_num > 0)
		{
			if (space)
				replace_line(e->program, line_num, space + 1);
			else
				delete_line(e->program, line_num);
		}
		// Clear input for next edit
		e->input[0] = '\0';
		e->input_len = 0;
	}

	// Check Save - write to file and close
	if (button_clicked(&e->buttons[EDITOR_BTN_SAVE]))
	{
		FILE *fp = fopen(LANG_PROGRAMPATH, "w");
		if (fp)
		{
			fprintf(fp, "%s", e->program);
			fclose(fp);
		}
		editor_close(e);
		return 1;
	}

	if (button_clicked(&e->buttons[EDITOR_BTN_CANCEL]))
	{
		editor_close(e);
		return -1;
	}

	// Handle text input
	int key = GetCharPressed();
	while (key > 0)
	{
		if (key >= 32 && key <= 126 && e->input_len < EDITOR_MAX_INPUT - 1)
		{
			e->input[e->input_len++] = (char)key;
			e->input[e->input_len] = '\0';
		}
		key = GetCharPressed();
	}

	if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE))
	{
		if (e->input_len > 0)
			e->input[--e->input_len] = '\0';
	}

	return 0;
}

void editor_draw(Editor *e)
{
	if (!e->active)
		return;

	// Background
	DrawRectangle(e->x, e->y, e->width, e->height, EDITOR_BG);

	// Border
	DrawRectangleLines(e->x, e->y, e->width, e->height, EDITOR_BORDER);

	// Title
	const char *title = "program.rbt";
	int title_w = MeasureText(title, EDITOR_FONT_SIZE);
	DrawText(title, e->x + (e->width - title_w) / 2, e->y + 2, EDITOR_FONT_SIZE, EDITOR_BORDER);

	// Draw program with line numbers
	int y = e->y + 14;
	int line_num = 1;
	char *p = e->program;
	char line_buf[256];

	while (*p && y < e->y + e->height - 30)
	{
		// Extract line
		int i = 0;
		while (*p && *p != '\n' && i < 255)
			line_buf[i++] = *p++;
		line_buf[i] = '\0';
		if (*p == '\n')
			p++;

		// Draw "N: text"
		char display[280];
		snprintf(display, sizeof(display), "%d: %s", line_num, line_buf);
		DrawText(display, e->x + 4, y, EDITOR_FONT_SIZE, EDITOR_TEXT);

		y += EDITOR_LINE_HEIGHT;
		line_num++;
	}

	// Input field label and box
	int input_y = e->y + e->height - 28;
	DrawText("Line# Txt: ", e->x + 4, input_y, EDITOR_FONT_SIZE, EDITOR_BORDER);

	int input_x = e->x + 58;
	int input_w = e->width - 62;
	DrawRectangle(input_x, input_y - 1, input_w, 10, (Color){30, 45, 80, 255});
	DrawText(e->input, input_x + 2, input_y, EDITOR_FONT_SIZE, EDITOR_TEXT);

	// Blinking cursor
	if ((int)(GetTime() * 2) % 2 == 0)
	{
		int cursor_x = input_x + 2 + MeasureText(e->input, EDITOR_FONT_SIZE);
		DrawRectangle(cursor_x, input_y, 1, 8, EDITOR_TEXT);
	}

	// Buttons
	for (int i = 0; i < EDITOR_BTN_COUNT; i++)
		button_draw(&e->buttons[i]);
}

bool editor_is_active(Editor *e)
{
	return e->active;
}
