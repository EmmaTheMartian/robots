#include "ui.h"
#include <string.h>

#define FLASH_DURATION 0.15f

// Colors from robot.gif palette
static const Color BTN_BG = { 40, 65, 110, 255 };        // Dark blue
static const Color BTN_BORDER = { 140, 190, 215, 255 };  // Light cyan
static const Color BTN_FLASH = { 100, 155, 190, 255 };   // Medium cyan

void button_init(Button *btn, int x, int y, int width, int height, const char *text)
{
	btn->x = x;
	btn->y = y;
	btn->width = width;
	btn->height = height;
	strncpy(btn->text, text, MAX_BUTTON_TEXT - 1);
	btn->text[MAX_BUTTON_TEXT - 1] = '\0';
	btn->flash_timer = 0.0f;
	btn->was_clicked = false;
}

void button_update(Button *btn, float scale)
{
	btn->was_clicked = false;

	// Update flash timer
	if (btn->flash_timer > 0.0f)
	{
		btn->flash_timer -= GetFrameTime();
		if (btn->flash_timer < 0.0f)
			btn->flash_timer = 0.0f;
	}

	// Get mouse position in virtual coordinates
	Vector2 mouse = GetMousePosition();
	int mx = (int)(mouse.x / scale);
	int my = (int)(mouse.y / scale);

	// Check if mouse is over button
	bool hovering = mx >= btn->x && mx < btn->x + btn->width &&
	                my >= btn->y && my < btn->y + btn->height;

	// Check for click
	if (hovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
	{
		btn->flash_timer = FLASH_DURATION;
		btn->was_clicked = true;
	}
}

void button_draw(Button *btn)
{
	// Background color - flash lighter when clicked
	Color bg = btn->flash_timer > 0.0f ? BTN_FLASH : BTN_BG;

	DrawRectangle(btn->x, btn->y, btn->width, btn->height, bg);

	// Draw border - DrawLine excludes endpoint, so vertical lines need +1 to reach bottom corners
	int x1 = btn->x;
	int y1 = btn->y;
	int x2 = btn->x + btn->width - 1;
	int y2 = btn->y + btn->height - 1;
	DrawLine(x1, y1, x2, y1, BTN_BORDER);      // Top
	DrawLine(x1, y2, x2, y2, BTN_BORDER);      // Bottom
	DrawLine(x1, y1, x1, y2 + 1, BTN_BORDER);  // Left (+1 to include bottom-left)
	DrawLine(x2, y1, x2, y2 + 1, BTN_BORDER);  // Right (+1 to include bottom-right)

	// Center text in button
	int font_size = 8;
	int text_width = MeasureText(btn->text, font_size);
	int text_x = btn->x + (btn->width - text_width) / 2;
	int text_y = btn->y + (btn->height - font_size) / 2;

	DrawText(btn->text, text_x, text_y, font_size, BTN_BORDER);
}

bool button_clicked(Button *btn)
{
	return btn->was_clicked;
}
