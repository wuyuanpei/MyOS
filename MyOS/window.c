#include "bootpack.h"

/* Draw a window */
void make_window(unsigned char *buf, int xsize, int ysize, char *title, char act)
{
	static char closebtn[14][14] = {
		"OOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQ@",
		"OQQQQQQQQQQQQ@",
		"OQQ@@QQQQ@@QQ@",
		"OQQQ@@QQ@@QQQ@",
		"OQQQQ@@@@QQQQ@",
		"OQQQQQ@@QQQQQ@",
		"OQQQQ@@@@QQQQ@",
		"OQQQ@@QQ@@QQQ@",
		"OQQ@@QQQQ@@QQ@",
		"OQQQQQQQQQQQQ@",
		"OQQQQQQQQQQQQ@",
		"OQQQQQQQQQQQQ@",
		"@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	draw_rect(buf, xsize, COL8_GRAY, 0,         0,         xsize - 1, 0        );
	draw_rect(buf, xsize, COL8_WHITE, 1,         1,         xsize - 2, 1        );
	draw_rect(buf, xsize, COL8_GRAY, 0,         0,         0,         ysize - 1);
	draw_rect(buf, xsize, COL8_WHITE, 1,         1,         1,         ysize - 2);
	draw_rect(buf, xsize, COL8_D_GRAY, xsize - 2, 1,         xsize - 2, ysize - 2);
	draw_rect(buf, xsize, COL8_BLACK, xsize - 1, 0,         xsize - 1, ysize - 1);
	draw_rect(buf, xsize, COL8_GRAY, 2,         2,         xsize - 3, ysize - 3);
	if (act)
		draw_rect(buf, xsize, COL8_D_BLUE, 3,         3,         xsize - 4, 20       );
	else
		draw_rect(buf, xsize, COL8_D_GRAY, 3,         3,         xsize - 4, 20       );
	draw_rect(buf, xsize, COL8_D_GRAY, 1,         ysize - 2, xsize - 2, ysize - 2);
	draw_rect(buf, xsize, COL8_BLACK, 0,         ysize - 1, xsize - 1, ysize - 1);
	draw_string(buf, xsize, COL8_WHITE, 8, 4, title);
	// Iterate the image
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 14; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_BLACK;
			} else if (c == 'Q') {
				c = COL8_GRAY;
			} else if (c == 'O'){
				c = COL8_WHITE;
			}
			buf[(5 + y) * xsize + (xsize - 19 + x)] = c;
		}
	}
	return;
}