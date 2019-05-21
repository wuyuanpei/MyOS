/* Entrance function after asmhead.nas */
#include "bootpack.h"
#include <stdio.h>

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

/* Entrance of the system addr[2*8:0x0000001b] after SPL */
void HariMain(void)
{
	// string buffer, FIFO for keyboard and mouse
	char buf[40], keybuf[KEY_BUF_LEN], mousebuf[MOUSE_BUF_LEN];
	// temporary variable (for FIFO data)
	unsigned char data;
	// temporary variable (for memory test)
	unsigned int mem_end;
	// Address for BOOTINFO
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	// Mouse decoder
	struct MOUSE_DEC mdec;
	// Sheets
	struct SHEET *sht_back, *sht_mouse, *sht_window1;
	unsigned char *buf_back, *buf_mouse, *buf_window1; // free memory address for sheet buffer
	// Mouse location (for display)
	int mx, my;
	// Counter
	int count = 0;

	/* Memory test */
	mem_end = memtest(FREE_MEMORY_BEGINNING, MAX_FREE_MEMORY_ENDING);
	/* Stop initializing the system */
	if(mem_end < MIN_MEMORY_REQUIRED){
		sprintf(buf, "%d MB memory required for the OS", MIN_MEMORY_REQUIRED >> 20);
		sys_error(buf);
	}
	/* Initialize free memory */
	mm_init((unsigned int *)FREE_MEMORY_BEGINNING,(unsigned int *)mem_end);

	init_gdtidt();
	init_pic();
	io_sti(); // Setup finished, open INT
	init_keyboard();
	enable_mouse(&mdec);
	
	fifo8_init(&keyfifo,KEY_BUF_LEN,keybuf);
	fifo8_init(&mousefifo,MOUSE_BUF_LEN,mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* open PIC1 and keyboard INT (11111001) */
	io_out8(PIC1_IMR, 0xef); /* open mouse INT (11101111) */
	
	init_palette();
	
	shtctl_init(binfo->vram, binfo->xsize, binfo->ysize);
	sht_back = sheet_alloc();
	sht_mouse = sheet_alloc();
	sht_window1 = sheet_alloc();

	buf_back = mm_malloc(binfo->xsize * binfo-> ysize);
	buf_mouse = mm_malloc(256); // mouse cursor is 16 * 16
	buf_window1 = mm_malloc(150*75); // window size

	sheet_setbuf(sht_back, buf_back, binfo->xsize, binfo->ysize, 0xff); // 0xff for col_inv
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 0xff); // 0xff for col_inv
	sheet_setbuf(sht_window1, buf_window1, 150, 75, 0xff); // 0xff for col_inv
	
	make_window(buf_window1, 150, 75, "First Window");
	init_screen(buf_back, binfo->xsize, binfo->ysize);
	init_mouse_cursor(buf_mouse,0xff); // background is transparent
	
	mx = (binfo->xsize - 16) / 2; /* Put the mouse at the center of the screen */
	my = (binfo->ysize - 16) / 2;
	
	// Set priority
	sheet_updown(sht_back, 0);
	sheet_updown(sht_window1, 1);
	sheet_updown(sht_mouse, 2);

	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_window1, 85, 64);
	
	draw_string(buf_back, binfo->xsize, COL8_YELLOW, 0, 16, "MyOS 1.0 ----Richard");
	sprintf(buf,"Memory sbrk: %x",mm_check());
	draw_string(buf_back, binfo->xsize, COL8_GREEN, 0, 32,buf);
	sprintf(buf,"Welcome to use MyOS!");
	draw_rect(buf_back, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize, 16);
	draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 0, buf);
	draw_rect(buf_back, binfo->xsize, COL8_RED, 0, 48, binfo->xsize, 64);
	sprintf(buf,"Decode(%d,%d,N)",mx,my);
	draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 48, buf);
	sheet_refresh(sht_back, 0, 0, binfo->xsize, binfo->ysize, 0);

	while(1) {
		draw_rect(buf_window1, 150, COL8_GRAY, 25, 25, 125, 40);
		sprintf(buf,"%012d",++count);
		draw_string(buf_window1, 150, COL8_BLACK, 25, 25, buf);
		sheet_refresh(sht_window1, 25, 25, 125, 41, 0);
		
		if((fifo8_status(&keyfifo) | fifo8_status(&mousefifo)) == 0){
			io_hlt();
		}else{
			io_cli();
			if(fifo8_status(&keyfifo)){
				data = fifo8_get(&keyfifo);
				io_sti();
				sprintf(buf,"INT 21 (IRQ-1) : PS/2 keyboard [%x]",data);
				draw_rect(buf_back, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize, 15);
				draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 0, buf);
				sheet_refresh(sht_back, 0, 0, binfo->xsize, 16, 0);
			}
			else if(fifo8_status(&mousefifo)){
				data = fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec,data)){
					sprintf(buf,"INT 2c (IRQ-12) : PS/2 mouse [%x,%x,%x]", mdec.buf[0], mdec.buf[1], mdec.buf[2]);
					draw_rect(buf_back, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize, 15);
					draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 0, buf);
					sheet_refresh(sht_back, 0, 0, binfo->xsize, 16, 0);
					/* Draw the new cursor */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->xsize - 1) {
						mx = binfo->xsize - 1;
					}
					if (my > binfo->ysize - 1) {
						my = binfo->ysize - 1;
					}
					sheet_slide(sht_mouse, mx, my);
					draw_rect(buf_back, binfo->xsize, COL8_RED, 0, 48, binfo->xsize, 63);
					if(mdec.btn == 1)
						sprintf(buf,"Decode(%d,%d,L)",mx,my);
					else if(mdec.btn == 2)
						sprintf(buf,"Decode(%d,%d,R)",mx,my);
					else if(mdec.btn == 4)
						sprintf(buf,"Decode(%d,%d,M)",mx,my);
					else
						sprintf(buf,"Decode(%d,%d,N)",mx,my);
					draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 48, buf);
					sheet_refresh(sht_back, 0, 48, binfo->xsize, 64, 0);
				}
			}
			io_sti();
		}
	}
}

/* System encounters fatal error, this method never returns */
void sys_error(char * error_info)
{
	char buf[128];
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	draw_rect(binfo->vram, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize - 1, binfo->ysize - 1);
	sprintf(buf,"SYSTEM ERROR:");
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 8, 8, buf);
	sprintf(buf,"> %s",error_info);
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 8, 24, buf);
	while(1){
		io_hlt();
	}
}

void make_window(unsigned char *buf, int xsize, int ysize, char *title)
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
	draw_rect(buf, xsize, COL8_D_BLUE, 3,         3,         xsize - 4, 20       );
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