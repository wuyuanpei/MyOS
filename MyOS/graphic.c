/* Graphic Setup */
#include "bootpack.h"

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:Black */
		0xff, 0x00, 0x00,	/*  1:Red */
		0x00, 0xff, 0x00,	/*  2:Green */
		0xff, 0xff, 0x00,	/*  3:Yellow */
		0x00, 0x00, 0xff,	/*  4:Blue */
		0xff, 0x00, 0xff,	/*  5:Purple */
		0x00, 0xff, 0xff,	/*  6:Cyan */
		0xff, 0xff, 0xff,	/*  7:White */
		0xc6, 0xc6, 0xc6,	/*  8:Gray */
		0x84, 0x00, 0x00,	/*  9:Dark red */
		0x00, 0x84, 0x00,	/* 10:Dark green */
		0x84, 0x84, 0x00,	/* 11:Dark yellow */
		0x00, 0x00, 0x84,	/* 12:Dark cyan */
		0x84, 0x00, 0x84,	/* 13:Dark purple */
		0x00, 0x84, 0x84,	/* 14:Dark blue */
		0x84, 0x84, 0x84	/* 15:Dark gray */
	};
	int i, eflags;
	unsigned char * rgb;
	eflags = io_load_eflags();	/* Record current INT state */
	io_cli(); 					/* Block interrupt */
	io_out8(0x03c8, 0);			/* Device Number */
	rgb = table_rgb;
	for (i = 0; i < 16; i++) {
		io_out8(0x03c9, rgb[0]/4); /* Range: 0~0x3f */
		io_out8(0x03c9, rgb[1]/4);
		io_out8(0x03c9, rgb[2]/4);
		rgb += 3;
	}
	io_store_eflags(eflags);	/* Restore INT */
}

void draw_rect(char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
}

void init_screen(char *vram, int xsize, int ysize){
	draw_rect(vram, xsize, COL8_D_CYAN, 0, 0, xsize - 1, ysize - 29);
	draw_rect(vram, xsize, COL8_D_GRAY, 0, ysize - 10, xsize - 1, ysize - 28);
	draw_rect(vram, xsize, COL8_WHITE, 0, ysize - 27, xsize - 1, ysize - 27);
	draw_rect(vram, xsize, COL8_D_GRAY, 0, ysize - 26, xsize - 1, ysize - 1);

	draw_rect(vram, xsize, COL8_WHITE, 3, ysize - 24, 59, ysize - 24);
	draw_rect(vram, xsize, COL8_WHITE, 2, ysize - 24, 2, ysize - 4);
	draw_rect(vram, xsize, COL8_GRAY, 3, ysize - 4, 59, ysize - 4);
	draw_rect(vram, xsize, COL8_GRAY, 59, ysize - 23, 59, ysize - 5);
	draw_rect(vram, xsize, COL8_BLACK, 2, ysize - 3, 59, ysize - 3);
	draw_rect(vram, xsize, COL8_BLACK, 60, ysize - 24, 60, ysize - 3);

	draw_rect(vram, xsize, COL8_GRAY, xsize - 47, ysize - 24, xsize - 4, ysize - 24);
	draw_rect(vram, xsize, COL8_GRAY, xsize - 47, ysize - 23, xsize - 47, ysize - 4);
	draw_rect(vram, xsize, COL8_WHITE, xsize - 47, ysize - 3, xsize - 4, ysize - 3);
	draw_rect(vram, xsize, COL8_WHITE, xsize - 3, ysize - 24, xsize - 3, ysize - 3);
}

void draw_char(char *vram, int xsize, unsigned char c, int x, int y, char *font)
{
	int i,j;
	char *p,d;
	for(i = 0; i < 16; i++){
		p = vram + (y + i) * xsize + x;
		d = font[i];
		for(j = 0; j < 8; j++){
			if(d&(1<<(7-j))) p[j] = c;
		}
	}
}

void draw_string(char *vram, int xsize, unsigned char c, int x, int y, char *str)
{
	while(*str){
		draw_char(vram, xsize, c, x , y, hankaku + ((*str++) << 4));
		x += 8;
	}
}

void init_mouse_cursor(char *mouse, char bg_c)
{
	static char cursor[16][16] = {
		"**..............",
		"*O*.............",
		"*OO*............",
		"*OOO*...........",
		"*OOOO*..........",
		"*OOOOO*.........",
		"*OOOOOO*........",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOO*******.....",
		"*OO*............",
		"*O*.............",
		"**..............",
		"*...............",
		"................",
		"................"
	};
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_BLACK;
			}
			else if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_WHITE;
			}
			else {
				mouse[y * 16 + x] = bg_c;
			}
		}
	}
}

void draw_block(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
}
