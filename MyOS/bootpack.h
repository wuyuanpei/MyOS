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
void load_gdtr(int limit, int addr); // Load gdtr register
void load_idtr(int limit, int addr); // Load idtr register
int load_cr0(void); // Load CR0
void store_cr0(int cr0); // Store CR0
void asm_inthandler21(void); // Inthandler preset
//void asm_inthandler27(void);
void asm_inthandler2c(void); // Inthandler preset
// Memory test
unsigned int memtest_sub(unsigned int start, unsigned int end);

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
#define COL8_D_CYAN			12
#define COL8_D_PURPLE		13
#define COL8_D_BLUE			14
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
struct FIFO8 {
	unsigned char *buf;
	int p, q, size, free, flags;
};
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
unsigned char fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);
#define FLAGS_OVERRUN		0x0001

/* bootpack.c */
void HariMain(void);
/* System fatal error: this method never returns*/
void sys_error(char * error_info);

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
#define MOUSE_BUF_LEN			128		// FIFO Buffer Length

/* keyboard.c */
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(void);
/* Keyboard controller */
#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47
#define KEY_BUF_LEN				32		// FIFO Buffer Length

/* memory.c */
#define EFLAGS_AC_BIT			0x00040000
#define CR0_CACHE_DISABLE		0x60000000
#define FREE_MEMORY_BEGINNING	0x00400000
#define MAX_FREE_MEMORY_ENDING	0xdfffffff
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
	unsigned char *vram; // bootinfo->vram
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
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
