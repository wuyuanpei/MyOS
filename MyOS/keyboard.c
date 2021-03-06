
#include "bootpack.h"

extern struct TASK *task_kernal;

char shift = 0; //SHIFT state

char ctrl = 0; //CTRL state

static int key_leds = -1; // LOCKS state

/* Wait for Keyboard Controller to receive signal */
void wait_KBC_sendready(void)
{
	while(1) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			return;
		}
	}
}

static char sendback_ack;

/* Set the LEDs on the keyboard*/
void set_kb_led(void){
reset:
	sendback_ack = 0;
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KEYCMD_LED);
	while(sendback_ack == 0){
		io_hlt();
	}
	if(sendback_ack == -1) goto reset;
	sendback_ack = 0;
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, key_leds);
	while(sendback_ack == 0){
		io_hlt();
	}
	if(sendback_ack == -1) goto reset;
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
	if(data == 0xfa){			/* Status for set_kb_led */
		sendback_ack = 1;
		return;
	}
	sendback_ack = -1;
	// Adjust ctrl
	if(data == 0x1D){
		ctrl = 1;
	}
	if(data == 0x9D){
		ctrl = 0;
	}
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
	fifo_put(& task_kernal->fifo,((int)data) + KEYBOARD_OFFSET);
	task_run(task_kernal, -1, 0);
}

/* Turn a key on the keyboard to a character.
 * If a key cannot turn into a character, return 0
 */
char key_to_char(unsigned char key)
{
	char result;
	static char keytable1[0x54] = {
		0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8, 0,
		'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 10, 0, 'a', 's',
		'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
		'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	static char keytable2[0x54] = {
		0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 10, 0, 'A', 'S',
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
		if((result>='a' && result<='z') || (result>='A' && result<='Z')){
			result ^= 0x20;
		}
	}
	return result;
}
