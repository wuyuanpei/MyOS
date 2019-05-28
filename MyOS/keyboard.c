#include "bootpack.h"

extern struct TASK *task_kernal;

static char shift = 0; //SHIFT state

static int key_leds = -1; // LOCKS state

/* Wait for Keyboard Controller to receive signal */
void wait_KBC_sendready(void)
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			return;
		}
	}
}

/* Initialize Keyboard Controller */
void init_keyboard(void)
{
	// Address for BOOTINFO to get CAPSLK information
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	// First time-> Initialize keyboard state from binfo->leds
	if(key_leds == -1){
		key_leds = (binfo->leds >> 4) & 7; // bit4~6
	}
}

/* PS/2 Keyboard Interrupt */
void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	/* Inform PIC that IRQ-01 finished */
	data = io_in8(PORT_KEYDAT);
	// Adjust shift
	if((data == 0x2A) || (data == 0x36)){
		shift = 1;
	}
	if((data == 0xAA) || (data == 0xB6)){
		shift = 0;
	}
	// Adjust CAPSLK
	if(data == 0xBA){
		key_leds ^= 4;
	}
	fifo_put(&task_kernal->fifo,((int)data) + KEYBOARD_OFFSET);
	task_run(task_kernal, -1, 0);
}

/* Turn a key on the keyboard to a character */
char key_to_char(unsigned char key)
{
	char result;
	static char keytable1[0x54] = {
		0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
		'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0, 'a', 's',
		'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
		'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	static char keytable2[0x54] = {
		0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	if(key >= 0x54)
		return 0;
	if(shift)
		result = keytable2[key];
	else
		result = keytable1[key];
	// If CAPSLK is on
	if((key_leds & 4)){
		// If result is English character, change the case
		if(result>='a' && result<='z' || result>='A' && result<='Z'){
			result ^= 0x20;
		}
	}
	return result;
}
