#pragma once
#include "mx_config.h"
#include "mx_font.h"

typedef struct {
	char red;
	char green;
	char blue;
} mx_color_t;

static const mx_color_t MX_COLOR_BLACK = { 0, 0, 0 };
static const mx_color_t MX_COLOR_RED = { 255, 0, 0 };
static const mx_color_t MX_COLOR_GREEN = { 0, 255, 0 };
static const mx_color_t MX_COLOR_BLUE = { 0, 0, 255 };
static const mx_color_t MX_COLOR_YELLOW = { 255, 255, 0 };
static const mx_color_t MX_COLOR_CYAN = { 0, 255, 255 };
static const mx_color_t MX_COLOR_PINK = { 255, 0, 255 };
static const mx_color_t MX_COLOR_WHITE = { 255, 255, 255 };

mx_color_t frame[MX_PANEL_ROWS][MX_PANEL_COLS];

const mx_font_t *ft_p;
int rotation;
int offset_x;
int offset_y;

void MX_DrawPixel(int x, int y, mx_color_t color);
void MX_DrawLine(int x1, int y1, int x2, int y2, mx_color_t color);
void MX_DrawVline(int x1, int y1, int y2, mx_color_t color);
void MX_DrawHline(int x1, int x2, int y1, mx_color_t color);
void MX_DrawFill(int x1, int y1, int x2, int y2, mx_color_t color);
void MX_DrawCircle(int x, int y, int rad, mx_color_t color);
void MX_DrawCircleFill(int x, int y0, int rad, mx_color_t color);
void MX_DrawString(int x, int y, const mx_font_t *font_p, char *text_p, mx_color_t color);
void MX_DisplayClear(void);
void MX_DisplayTestPattern(void);
void MX_DisplayTest(void);
