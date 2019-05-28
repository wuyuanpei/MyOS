/* Entrance function after asmhead.nas */
#include "bootpack.h"
#include <stdio.h>

extern char mtask_on;

// The kernal task
struct TASK *task_kernal;

// The focused task
struct TASK *task_focused;

void task_b_main(void){
	char buf[20]; // string buffer
	struct SYS_TMR *tmr = (struct SYS_TMR *)SYS_TMR_ADR; //Address for System Timer
	struct SHEET *sht_window = sheet_alloc();
	unsigned char *buf_window = mm_malloc(150*50); // window size
	sheet_setbuf(sht_window, buf_window, 150, 50, 0xff); // 0xff for col_inv
	make_window(buf_window, 150, 50, "System Timer", 0);
	sheet_updown(sht_window, 1);
	sheet_slide(sht_window, 160, 70);
	start_timing(1,1);
	while(1){
		draw_rect(buf_window, 150, COL8_GRAY, 25, 25, 125, 40);
		sprintf(buf,"%02d%09d",tmr->time_high,tmr->time_low);
		draw_string(buf_window, 150, COL8_BLACK, 25, 25, buf);
		sheet_refresh(sht_window, 25, 25, 125, 41, 0);
		if(fifo_status(& task_now()->fifo) == 0)
			task_sleep(task_now());
		else{
			fifo_get(& task_now()->fifo);
			start_timing(1,1);
		}
	}
}

void task_console_main(void){
	int x = -1, y = 0, show_cursor = 1, xc = 0, yc = 0;
	struct SHEET *sht_window = sheet_alloc();
	unsigned char *buf_window = mm_malloc(150*105); // window size
	sheet_setbuf(sht_window, buf_window, 150, 105, 0xff); // 0xff for col_inv
	make_window(buf_window, 150, 105, "Command", 1);
	draw_rect(buf_window, 150, COL8_BLACK, 3, 22, 146, 101);
	sheet_updown(sht_window, 1);
	sheet_slide(sht_window, 5, 70);
	start_timing(1,50);
	while(1){
		if(fifo_status(& task_now()->fifo) == 0)
			task_sleep(task_now());
		unsigned int data = fifo_get(& task_now()->fifo);
		if(data == TIMER_OFFSET + 1){
			start_timing(1,50);
			if(show_cursor){
				draw_rect(buf_window, 150, COL8_GREEN, 3 + 8 * xc, 22 + 16 * yc, 10 + 8 * xc, 37 + 16 * yc);
				sheet_refresh(sht_window, 3 + 8 * xc, 22 + 16 * yc, 11 + 8 * xc, 38 + 16 * yc, 0);
			}else{
				draw_rect(buf_window, 150, COL8_BLACK, 3 + 8 * xc, 22 + 16 * yc, 10 + 8 * xc, 37 + 16 * yc);
				sheet_refresh(sht_window, 3 + 8 * xc, 22 + 16 * yc, 11 + 8 * xc, 38 + 16 * yc, 0);
			}
			show_cursor ^= 1;
			continue;
		}
		data = key_to_char(data);
		if(data == 0) 
			continue;
		x++;
		xc++;
		if(x == 18){
			x = 0;
			y++;
			if(y==5){
				y = 0;
				draw_rect(buf_window, 150, COL8_BLACK, 3, 22, 146, 101);
				sheet_refresh(sht_window, 3, 22, 147, 102, 0);
			}
		}
		if(xc == 18){
			xc = 0;
			yc++;
			if(yc==5){
				yc = 0;
			}
		}
		draw_rect(buf_window, 150, COL8_BLACK, 3 + 8 * x, 22 + 16 * y, 10 + 8 * x, 37 + 16 * y);
		draw_char(buf_window, 150, COL8_GREEN, 3 + 8 * x, 22 + 16 * y, hankaku + data * 16);
		sheet_refresh(sht_window, 3 + 8 * x, 22 + 16 * y, 11 + 8 * x, 38 + 16 * y, 0);
	}
}

/* Entrance of the system addr[2*8:0x0000001b] after SPL */
void HariMain(void)
{
	// string buffer
	char buf[40];
	// temporary variable (for FIFO data)
	unsigned int data;
	// temporary variable (for memory test)
	unsigned int mem_end;
	// Address for BOOTINFO
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	// Address for System Timer
	struct SYS_TMR *tmr = (struct SYS_TMR *)SYS_TMR_ADR;
	// A user timer
	//struct USR_TMR usr_tmr1, usr_tmr2, usr_tmr3;
	// Mouse decoder
	struct MOUSE_DEC mdec;
	// Sheets
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, *buf_mouse; // free memory address for sheet buffer
	// Mouse location (for display)
	int mx, my;
	// Input Signal
	char input_signal = 0;
	// System Test
	//int count = 0;
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

	init_pit();
	io_out8(PIC0_IMR, 0xf8); /* open PIC0 (timer), PIC1, and keyboard INT (11111000) */
	io_out8(PIC1_IMR, 0xef); /* open mouse INT (11101111) */
	
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
	
	// Set priority
	sheet_updown(sht_back, 0);
	sheet_updown(sht_mouse, 1);

	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_back, 0, 0);
	
	draw_string(buf_back, binfo->xsize, COL8_YELLOW, 0, 16, "MyOS 1.0 ----Richard");
	sprintf(buf,"vram: %x",binfo->vram);
	draw_rect(buf_back, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize, 16);
	draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 0, buf);
	sheet_refresh(sht_back, 0, 0, binfo->xsize, binfo->ysize, 0);
	
	// Task
	task_kernal = task_init();

	struct TASK *task_b;
	task_b = task_alloc();
	task_b->tss.esp = (unsigned int)mm_malloc(64 * 1024) + 64 * 1024; // 64 KB stack
	task_b->tss.eip = (int) &task_b_main;
	task_b->tss.es = 1 * 8;
	task_b->tss.cs = 2 * 8; // Same segment as the kernal
	task_b->tss.ss = 1 * 8;
	task_b->tss.ds = 1 * 8;
	task_b->tss.fs = 1 * 8;
	task_b->tss.gs = 1 * 8;
	task_run(task_b, 1, 0); // Level 1, Priority 1 (keep default)

	struct TASK *task_console;
	task_console = task_alloc();
	task_focused = task_console;
	task_console->tss.esp = (unsigned int)mm_malloc(64 * 1024) + 64 * 1024; // 64 KB stack
	task_console->tss.eip = (int) &task_console_main;
	task_console->tss.es = 1 * 8;
	task_console->tss.cs = 2 * 8; // Same segment as the kernal
	task_console->tss.ss = 1 * 8;
	task_console->tss.ds = 1 * 8;
	task_console->tss.fs = 1 * 8;
	task_console->tss.gs = 1 * 8;
	task_run(task_console, 1, 0); // Level 1, Priority 1 (keep default)

	while(1) {
		if(fifo_status(&task_kernal->fifo) == 0){
			task_sleep(task_kernal);
		}

		data = fifo_get(&task_kernal->fifo);
		
		// Keyboard
		if(data < 256){ // Keyboard input will forward call the focused task
			fifo_put(&task_focused->fifo, data);
			task_run(task_focused, -1, 0);
			char ch[2] = {0};
			ch[0] = key_to_char(data);
			sprintf(buf,"INT 21 (IRQ-1) : PS/2 keyboard [%x, %s]",data,ch);
			draw_rect(buf_back, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize, 15);
			draw_string(buf_back, binfo->xsize, COL8_WHITE, 0, 0, buf);
			sheet_refresh(sht_back, 0, 0, binfo->xsize, 16, 0);
		}
		
		// Mouse
		else{
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
			}
		}
	}
}

/* System encounters fatal error, this method never returns */
void sys_error(char * error_info)
{
	char buf[128];
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	draw_rect(binfo->vram, binfo->xsize, COL8_BLACK, 0, 0, binfo->xsize - 1, binfo->ysize - 1);
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 8, 8, "SYSTEM ERROR:");
	sprintf(buf,"> %s",error_info);
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 8, 24, buf);
	while(1){
		io_hlt();
	}
}

/* Print out debug information */
void sys_debug(char * debug_info)
{
	static int i = 1;
	char buf[50];
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	sprintf(buf,"M%d: %s",i++,debug_info);
	draw_rect(binfo->vram, binfo->xsize, COL8_WHITE, 0, 16, binfo->xsize - 1, 31);
	draw_string(binfo->vram, binfo->xsize, COL8_BLACK, 0, 16,buf);
}

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
