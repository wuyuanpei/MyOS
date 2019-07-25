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
	struct SHTCTL *shtctl = (struct SHTCTL *)ADR_SHTCTL;
	sheet_updown(sht_window, shtctl->top);
	sheet_slide(sht_window, 100, 100);
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
	//struct SYS_TMR *tmr = (struct SYS_TMR *)SYS_TMR_ADR;
	// A user timer
	//struct USR_TMR usr_tmr1, usr_tmr2, usr_tmr3;
	// Mouse decoder
	struct MOUSE_DEC mdec;
	// Sheets
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, *buf_mouse; // free memory address for sheet buffer
	// Mouse location (for display)
	int mx, my;
	// System Test
	//int count = 0;
	/* Memory test */
	mem_end = memtest(FREE_MEMORY_BEGINNING, MAX_FREE_MEMORY_ENDING);
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 0, "> Memory tested [0]");
	/* Stop initializing the system */
	if(mem_end < MIN_MEMORY_REQUIRED){
		sprintf(buf, "%d MB memory required for the OS", MIN_MEMORY_REQUIRED >> 20);
		sys_error(buf);
	}
	/* Initialize free memory */
	mm_init((unsigned int *)FREE_MEMORY_BEGINNING,(unsigned int *)mem_end);
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 16, "> Memory initialized [1]");
	init_gdtidt();
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 32, "> GDT/IDT initialized [2]");
	init_pic();
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 48, "> PIC initialized [3]");
	io_sti(); // Setup finished, open INT
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 64, "> CPU interrupt flag initialized [4]");
	init_pit();
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 80, "> PIT initialized [5]");
	init_keyboard();
	enable_mouse(&mdec);
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 96, "> Keyboard/Mouse initialized [6]");
	
	io_out8(PIC0_IMR, 0xf8); /* open PIC0 (timer), PIC1, and keyboard INT (11111000) */
	io_out8(PIC1_IMR, 0xef); /* open mouse INT (11101111) */
	
	// Task
	task_kernal = task_init();
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 112, "> Multitask controller initialized [7]");
	init_palette();
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 128, "> Screen palette initialized [8]");
	shtctl_init(binfo->vram, binfo->xsize, binfo->ysize);
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 144, "> Sheet controller initialized [9]");
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
	draw_string(binfo->vram, binfo->xsize, COL8_WHITE, 0, 160, "> Screen/Sheets initialized [10]");
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
		// Set CAPSLK
		if(data == (0xBA + KEYBOARD_OFFSET))
			set_kb_led();

		// Keyboard
		if(data < 256){ 
			// CTRL+C to kill the application (If the console task is in application)
			extern char ctrl;
			extern int program_addr;
			if((key_to_char(data) == 'c' || key_to_char(data) == 'C') && (ctrl == 1) && (program_addr != 0)){
				io_cli();
				task_console->tss.eax = (int) &(task_console->tss.esp0);
				task_console->tss.eip = (int) &end_app;
				io_sti();
				print_error("Program Terminated");
				continue; // Do not send 'c' to the application
			}
			// F1 is reserved for turning the lowest window to the highest
			struct SHTCTL *shtctl = (struct SHTCTL *)ADR_SHTCTL;
			if(data == 0x3b && shtctl->top > 2) {
				sheet_updown(shtctl->sheets[1], shtctl->top - 1);
			}
			// Keyboard input will forward call the focused task
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
extern char mtask_on;
void sys_error(char * error_info)
{
	mtask_on = 0; // System encounters fatal error, no context switching any more
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

