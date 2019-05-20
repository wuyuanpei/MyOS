/* Entrance function after asmhead.nas */
#include "bootpack.h"
#include <stdio.h>

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

/* Entrance of the system addr[2*8:0x0000001b] after SPL */
void HariMain(void)
{
	char buf[40], cursor[256], keybuf[KEY_BUF_LEN], mousebuf[MOUSE_BUF_LEN];
	unsigned char data;
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	struct MOUSE_DEC mdec;

	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, *buf_mouse; // free memory address

	int mx, my, i;

	init_gdtidt();
	init_pic();
	io_sti(); // Setup finished, open INT
	init_keyboard();
	enable_mouse(&mdec);
	
	fifo8_init(&keyfifo,KEY_BUF_LEN,keybuf);
	fifo8_init(&mousefifo,MOUSE_BUF_LEN,mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* open PIC1 and keyboard INT (11111001) */
	io_out8(PIC1_IMR, 0xef); /* open mouse INT (11101111) */
	
	/* Initialize free memory */
	mm_init((unsigned int *)FREE_MEMORY_BEGINNING,(unsigned int *)memtest(FREE_MEMORY_BEGINNING, MAX_FREE_MEMORY_ENDING));
	
	init_palette();
	
	shtctl_init(binfo->vram, binfo->xsize, binfo->ysize);
	sht_back = sheet_alloc();
	sht_mouse = sheet_alloc();

	buf_back = mm_malloc(binfo->xsize * binfo-> ysize);
	buf_mouse = mm_malloc(256); // mouse cursor is 16 * 16
	sheet_setbuf(sht_back, buf_back, binfo->xsize, binfo->ysize, 0xff); // 0xff for col_inv
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 0xff); // 0xff for col_inv
	init_screen(buf_back, binfo->xsize, binfo->ysize);
	init_mouse_cursor(buf_mouse,0xff); // background is transparent
	
	mx = (binfo->xsize - 16) / 2; /* Put the mouse at the center of the screen */
	my = (binfo->ysize - 16) / 2;
	sheet_updown(sht_back, 0);
	sheet_updown(sht_mouse, 1);
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_back, 0, 0);
	
	draw_string(buf_back, binfo->xsize, COL8_YELLOW, 0, 16, "MyOS 1.0");
	draw_string(buf_back, binfo->xsize, COL8_GREEN, 0, 32, "----Richard");
	sprintf(buf,"Welcome to use MyOS!");
	draw_rect(buf_back, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize, 16);
	draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 0, buf);
	draw_rect(buf_back, binfo->xsize, COL8_RED, 0, 48, binfo->xsize, 64);
	sprintf(buf,"(%d,%d)",mx,my);
	draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 48, buf);
	sheet_refresh(sht_back, 0, 0, binfo->xsize, binfo->ysize);
	
	while(1) {
		io_cli();
		if((fifo8_status(&keyfifo) | fifo8_status(&mousefifo)) == 0){
			io_stihlt();
		}else{
			if(fifo8_status(&keyfifo)){
				data = fifo8_get(&keyfifo);
				io_sti();
				sprintf(buf,"INT 21 (IRQ-1) : PS/2 keyboard [%x]",data);
				draw_rect(buf_back, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize, 16);
				draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 0, buf);
				sheet_refresh(sht_back, 0, 0, binfo->xsize, 16);
			}
			else if(fifo8_status(&mousefifo)){
				data = fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec,data)){
					sprintf(buf,"INT 2c (IRQ-12) : PS/2 mouse [%x,%x,%x]", mdec.buf[0], mdec.buf[1], mdec.buf[2]);
					draw_rect(buf_back, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize, 16);
					draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 0, buf);
					sheet_refresh(sht_back, 0, 0, binfo->xsize, 16);
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
					draw_rect(buf_back, binfo->xsize, COL8_RED, 0, 48, binfo->xsize, 64);
					sprintf(buf,"(%d,%d)",mx,my);
					draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 48, buf);
					sheet_refresh(sht_back, 0, 48, binfo->xsize, 64);
				}
			}
		}
	}
}

/* System encounters fatal error, this method never returns */
void sys_error(char * error_info)
{
	char buf[128];
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	draw_rect(binfo->vram, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize, binfo->ysize);
	sprintf(buf,"SYSTEM ERROR: %s", error_info);
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 8, 8, buf);
	while(1){
		io_hlt();
	}
}
