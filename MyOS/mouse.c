#include "bootpack.h"

extern struct TASK *task_kernal;

/* Initialize Mouse */
void enable_mouse(struct MOUSE_DEC *mdec)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0;
	return; /* Keyboard Controller will send back ACK(0xfa) */
}

/* Put the data into the decode buffer and update it */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/* Ignore 0xfa (initialization finished) and move to phase 1 */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* phase 1, dat must be [0~3][8~f]*/
		if ((dat & 0xc8) == 0x08) {
			/* check validity */
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		/* phase 2 (x) */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* phase 3 (y) */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		/* store btn information */
		mdec->btn = mdec->buf[0] & 0x07;
		/* store x, y information */
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		/* Base on buf[0], reset x and y*/
		if ((mdec->buf[0] & 0x10) != 0) { // 1 or 3 means moving leftward
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) { // 2 or 3 means moving downward
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; /* y increases as mouse move downward */
		return 1;
	}
	return -1; // never reaches here
}

/* PS/2 Mouse Interrupt */
void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);	/* Inform slave PIC that IRQ-12 finished */
	io_out8(PIC0_OCW2, 0x62);	/* Inform master PIC that IRQ-02 finished */
	data = io_in8(PORT_KEYDAT);
	fifo_put(&task_kernal->fifo, ((int)data) + MOUSE_OFFSET);
	task_run(task_kernal, -1, 0);
}
