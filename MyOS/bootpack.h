/* All the function/constant declarations in .sys c and assmbly files*/

/** asmhead.nas */
/* Boot information struct*/
struct BOOTINFO{ /* 0x0ff0 ~ 0x0fff*/
	char cyls, leds, vmode, padding;
	short xsize, ysize;
	char *vram;
};
#define ADR_BOOTINFO 0x00000ff0

/** dsctbl.c */
/* GDT content (8 Bytes) */
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};
/* IDT content (8 Bytes) */
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
void init_gdtidt(void); // Initialize gdt and idt
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit,
 int base, int ar); // Set one gdt entry
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset,
 int selector, int ar); // Set one idt entry
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e
#define AR_TSS32		0x0089

/** hankaku.obj */
extern char hankaku[4096]; // Font Document

/** naskfunc.nas */
// External functions implemented by asm in naskfunc.nas
void io_cli(void); // CLI
void io_out8(int port, int data); // OUT
int io_in8(int port); // IN
int io_load_eflags(void); // Get current INT state
void io_store_eflags(int eflags); // Set INT state
void io_hlt(void); // HLT
void io_sti(void); // STI
void io_stihlt(void); // STI HLT
void load_gdtr(int limit, int addr); // Load global descriptor table register
void load_idtr(int limit, int addr); // Load interrupt descriptor table register
void load_tr(int tr); // Load task register
int load_cr0(void); // Load CR0
void store_cr0(int cr0); // Store CR0
void asm_inthandler20(void); // Inthandler preset
void asm_inthandler21(void); // Inthandler preset
//void asm_inthandler27(void);
void asm_inthandler2c(void); // Inthandler preset
// Memory test
unsigned int memtest_sub(unsigned int start, unsigned int end);
// Far jump, used in context switching
void farjmp(int eip, int cs);

/** graphic.c */
void init_palette(void); // Initialize palette
void init_screen(char *vram, int xsize, int ysize); // Initialize screen
void init_mouse_cursor(char *mouse, char bg_c); // Initialize cursor
void draw_rect(char *vram, int xsize, unsigned char c,
 int x0, int y0, int x1, int y1); // Draw a rectangle
void draw_char(char *vram, int xsize, unsigned char c,
 int x, int y, char *font); // Draw a character
void draw_string(char *vram, int xsize, unsigned char c,
 int x, int y, char *str); // Draw a string
 // Draw a block: vxsize-vram x size;
 // pxsize, pysize-block size; bxsize-buf x size
void draw_block(char *vram, int vxsize, int pxsize,
 int pysize, int px0, int py0, char *buf, int bxsize);
 /* Colors on the screen */
#define COL8_BLACK		0
#define COL8_RED		1
#define COL8_GREEN		2
#define COL8_YELLOW		3
#define COL8_BLUE		4
#define COL8_PURPLE		5
#define COL8_CYAN		6
#define COL8_WHITE		7
#define COL8_GRAY		8
#define COL8_D_RED		9
#define COL8_D_GREEN		10
#define COL8_D_YELLOW		11
#define COL8_D_BLUE			12
#define COL8_D_PURPLE		13
#define COL8_D_CYAN			14
#define COL8_D_GRAY			15

/* int.c */
void init_pic(void);
//void inthandler27(int *esp);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

/* fifo.c */
// Every task has a FIFO
#define BUF_LENGTH			256		// Buffer Size
#define KEYBOARD_OFFSET		0
#define MOUSE_OFFSET		256
#define TIMER_OFFSET		512
struct FIFO {
	unsigned int buf[BUF_LENGTH];
	int p, q, size, free, flags;
};
void fifo_init(struct FIFO *fifo);
int fifo_put(struct FIFO *fifo, unsigned int data);
unsigned int fifo_get(struct FIFO *fifo);
int fifo_status(struct FIFO *fifo);
#define FLAGS_OVERRUN		0x0001

/* bootpack.c */
void HariMain(void);
/* System fatal error: this method never returns*/
void sys_error(char * error_info);
/* Print out debug information */
void sys_debug(char * debug_info);
/* Draw a window: act: the window is active */
void make_window(unsigned char *buf, int xsize, int ysize, char *title, char act);

/* mouse.c */
/* Mouse Decode Struct*/
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void inthandler2c(int *esp);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

/* keyboard.c */
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(void);
void set_kb_led(void);
/* Turn a key on the keyboard to a character */
char key_to_char(unsigned char key);
/* Keyboard controller */
#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47
#define KEYCMD_LED				0xed

/* memory.c */
#define EFLAGS_AC_BIT			0x00040000
#define CR0_CACHE_DISABLE		0x60000000
#define FREE_MEMORY_BEGINNING	0x00400000
#define MAX_FREE_MEMORY_ENDING	0xdfffffff // 3GB; some vram starts from there
#define MIN_MEMORY_REQUIRED		0x00A00000 // At least 10 MB memory for the OS
unsigned int memtest(unsigned int start, unsigned int end);
unsigned int mm_check(void);
void mm_init(unsigned int *start, unsigned int *end);
void *mm_malloc(unsigned int size);
void mm_free(void *bptr);
void *mm_realloc(void *bptr, unsigned int size);

/* sheet.c */
#define MAX_SHEETS		256 // Maximum number of sheets that desktop can hold
/* A piece of sheet */
struct SHEET {
	unsigned char *buf;		// Content on the sheet
	// bxsize * bysize is the size of buf (the size of the sheet)
	// vx0, vy0 the location of the sheet on the screen
	// col_inv color for invisibility
	// height: the height (priority) of the sheet
	// flag: status
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
};
/* Sheet contoller */
struct SHTCTL {
	unsigned char *vram, *map; // bootinfo->vram; map: a map to map every pixel in vram to sheet
	// bootinfo->xsize, bootinfo->ysize, the height of the uppermost sheet
	int xsize, ysize, top;
	// sort sheets by height and store addresses here (only store non-negative height sheet)
	// sheets[0]->height == 0; sheets[1]->height == 1; sheets[2]->height == 2; ...
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS]; // all sheets
};
#define ADR_SHTCTL		0x268000	// address of sheet controller
#define SHEET_UNUSED	0
#define SHEET_USED		1
// see sheet.c for details
void shtctl_init(unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(void);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1, int change_visibility);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);

/* mtask.c */
#define MAX_TASKS		1000	/* Maximum number of tasks */
#define TASK_GDT0		3		/* First TSS entry in GDT */
#define MAX_TASKS_LV	100		/* The number of tasks per level */
#define MAX_TASKLEVELS	10		/* The number of levels */
/* Task Status Segment, defined by CPU */
struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};
#define TASK_UNUSED		0
#define TASK_USED		1
#define TASK_RUNNING	2
struct TASK {
	/* sel: the entry number of this task in GDT (GDT selector) */
	/* flags: the status of the task */
	/* priority: the priority of the task (10 [0.1s] ~ 1 [0.01s])*/
	/* level: the level of the task */
	int sel, flags, priority, level;
	struct FIFO fifo;
	struct TSS32 tss;
};
struct TASKLEVEL {
	int running; /* The number of tasks running */
	int now; /* Which task is running */
	struct TASK *tasks[MAX_TASKS_LV]; /* All the tasks in this level */
};
struct TASKCTL {
	int now_lv; /* The level running*/
	char lv_change; /* Whether to change a level during context switching */
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};
struct TASK *task_init(void); // Initialize multitask system, update GDT with TSS, turn mtask_on on
struct TASK *task_alloc(void); // Allocate a new task, tag flag TASK_USED
void task_run(struct TASK *task, int level, int priority); // Run a task, tag flag TASK_RUNNING, put it into the list
void task_switch(void); // Switch to the next task (always called by inthandler20)
void task_sleep(struct TASK *task); // Sleep a task, tag flag TASK_USED, drop it from the list
struct TASK *task_now(void); // Return the current running task

/* timer.c */
#define PIT_CTRL		0x0043
#define PIT_CNT0		0x0040
/* System Timer: can store 0xffffffffffffffff units of 0.01 seconds */
#define SYS_TMR_ADR		0x0026A414
struct SYS_TMR {
	unsigned int time_high, time_low;
};
struct USR_TMR {
	unsigned int start_high, start_low;
	unsigned int end_high, end_low;
};
struct TMRCTL
{
	struct TMRCTL *next;
	struct USR_TMR node_data;
	struct TASK *task;
	unsigned char handler;
};
void init_pit(void);
void inthandler20(int *esp);
// start timing using a handler. When timing finishes, the handler will be put into FIFO
void start_timing(unsigned char handler, unsigned int duration);
