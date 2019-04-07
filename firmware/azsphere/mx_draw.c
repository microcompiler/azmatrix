#include <string.h>
#include "mx_draw.h"

#define MX_MATH_ABS(x) ((x) > 0 ? (x) : (-(x)))
#define MX_SWAPCOORD(a, b) { int t = a; a = b; b = t; }

void MX_DrawPixel(int x, int y, mx_color_t color)
{
	x += offset_x;
	y += offset_y;
	
	if ((x < 0) || (x >= MX_PANEL_COLS) || (y < 0) || (y >= MX_PANEL_ROWS))
		return;

	switch (rotation)
	{
	case 1:
		MX_SWAPCOORD(x, y);
		x = MX_PANEL_ROWS - 1 - x;
		break;
	case 2:
		x = MX_PANEL_ROWS - 1 - x;
		y = MX_PANEL_COLS - 1 - y;
		break;
	case 3:
		MX_SWAPCOORD(x, y);
		y = MX_PANEL_COLS - 1 - y;
		break;
	}

	frame[x][y] = color;
}

void MX_DrawLine(int x1, int y1, int x2, int y2, mx_color_t color)
{
	int dx, dy;
	int steep, error, ystep;

	if (x1 == x2)
	{
		MX_DrawVline(x1, y1, y2, color);
	}
	else if (y1 == y2)
	{
		MX_DrawHline(x1, x2, y1, color);
	}

	steep = MX_MATH_ABS(y2 - y1) > MX_MATH_ABS(x2 - x1);

	if (steep)
	{
		MX_SWAPCOORD(x1, y1);
		MX_SWAPCOORD(x2, y2);
	}

	if (x1 > x2)
	{
		MX_SWAPCOORD(x1, x2);
		MX_SWAPCOORD(y1, y2);
	}

	dx = x2 - x1;
	dy = MX_MATH_ABS(y2 - y1);

	error = dx / 2;

	if (y1 < y2)
	{
		ystep = 1;
	}
	else
	{
		ystep = -1;
	}

	for (; x1 <= x2; x1++)
	{
		if (steep)
		{
			MX_DrawPixel(y1, x1, color);
		}
		else
		{
			MX_DrawPixel(x1, y1, color);
		}
		error -= dy;
		if (error < 0)
		{
			y1 += ystep;
			error += dx;
		}
	}
}

void MX_DrawVline(int x1, int y1, int y2, mx_color_t color)
{
	int y;

	if (y1 < y2)
	{
		for (y = y1; y <= y2; y++)
			MX_DrawPixel(x1, y, color);
	}
	else
	{
		for (y = y2; y <= y1; y++)
			MX_DrawPixel(x1, y, color);
	}
}

void MX_DrawHline(int x1, int x2, int y1, mx_color_t color)
{
	int x;

	if (x1 < x2)
	{
		for (x = x1; x <= x2; x++)
			MX_DrawPixel(x, y1, color);
	}
	else
	{
		for (x = x2; x <= x1; x++)
			MX_DrawPixel(x, y1, color);
	}
}

void MX_DrawFill(int x1, int y1, int x2, int y2, mx_color_t color)
{
	int y;

	if (y1 < y2)
	{
		for (y = y1; y <= y2; y++)
			MX_DrawHline(x1, x2, y, color);
	}
	else
	{
		for (y = y2; y <= y1; y++)
			MX_DrawHline(x1, x2, y, color);
	}
}

void MX_DrawCircle(int x, int y, int rad, mx_color_t color)
{
	rad = MX_MATH_ABS(rad);
	int vx = 0;
	int vy = rad;
	int v1 = 3 - 2 * rad;

	while (vx <= vy)
	{
		MX_DrawPixel(x + vx, y + vy, color);
		MX_DrawPixel(x - vx, y + vy, color);
		MX_DrawPixel(x + vx, y - vy, color);
		MX_DrawPixel(x - vx, y - vy, color);
		MX_DrawPixel(x + vy, y + vx, color);
		MX_DrawPixel(x - vy, y + vx, color);
		MX_DrawPixel(x + vy, y - vx, color);
		MX_DrawPixel(x - vy, y - vx, color);
		if (v1 < 0)
		{
			v1 = v1 + 4 * vx + 6;
		}
		else
		{
			v1 = v1 + 4 * (vx - vy) + 10;
			vy--;
		}
		vx++;
	}
}

void MX_DrawCircleFill(int x, int y0, int rad, mx_color_t color)
{
	int vx = MX_MATH_ABS(rad);
	int vy = 0;
	int dx = 1 - (rad << 1);
	int dy = 0;
	int error = 0;

	while (vx >= vy)
	{
		for (int i = x - vx; i <= x + vx; i++)
		{
			MX_DrawPixel(i, y0 + vy, color);
			MX_DrawPixel(i, y0 - vy, color);
		}
		for (int i = x - vy; i <= x + vy; i++)
		{
			MX_DrawPixel(i, y0 + vx, color);
			MX_DrawPixel(i, y0 - vx, color);
		}

		vy++;
		error += dy;
		dy += 2;
		if (((error << 1) + dx) > 0)
		{
			vx--;
			error += dx;
			dx += 2;
		}
	}
}

void MX_DrawString(int x, int y, const mx_font_t *font_p, char *text_p, mx_color_t color)
{
	int col, col_sub, row;
	int i = 0;

	while (text_p[i] != 0)
	{
		char width = mx_font_get_width(font_p, text_p[i]);
		const char *bitmap_p = mx_font_get_bitmap(font_p, text_p[i]);

		for (row = 0; row < font_p->height_row; row++)
		{
			for (col = 0, col_sub = 7; col < width; col++, col_sub--)
			{
				if (*bitmap_p & (1 << col_sub))
				{
					MX_DrawPixel(x + row, y + col, color);
				}

				if (col_sub == 0)
				{
					bitmap_p++;
					col_sub = 8;
				}
			}
			if (col_sub != 7)
				bitmap_p++;
		}
		y += width;
		i++;
	}
}

void MX_DisplayClear(void)
{
	memcpy(frame, 0, sizeof(frame));
}

void MX_DisplayTestPattern(void)
{
	for (int x = 0; x < MX_PANEL_ROWS; x++)
	{
		for (int y = 0; y < MX_PANEL_COLS; y++)
		{
			int a = x / 4;
			int b = y / 4;

			if (((a + b) % 2) == 0)
			{
				MX_DrawPixel(x, y, MX_COLOR_GREEN);
			}
			else
			{
				MX_DrawPixel(x, y, MX_COLOR_BLUE);
			}
		}
	}
}

void MX_DisplayTest(void)
{
	uint8_t word[] = { "MX" };

	MX_DrawPixel(1, 1, MX_COLOR_WHITE);
	MX_DrawLine(23, 4, 23, 28, MX_COLOR_BLUE);
	MX_DrawCircle(5, 7, 3, MX_COLOR_CYAN);
	MX_DrawCircleFill(5, 25, 3, MX_COLOR_YELLOW);
	MX_DrawFill(28, 2, 30, 29, MX_COLOR_PINK);
	MX_DrawString(6, 2, &mx_font_dejavu_20, word, MX_COLOR_RED);
}