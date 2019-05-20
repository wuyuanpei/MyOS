/* Interrupt */

#include "bootpack.h"

void init_pic(void)
/* initialize PIC */
{
	io_out8(PIC0_IMR,  0xff  ); /* Block all interrupts */
	io_out8(PIC1_IMR,  0xff  ); /* Block all interrupts */

	io_out8(PIC0_ICW1, 0x11  ); /* edge trigger mode */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7 -> INT20-27 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1 connected by IRQ2 */
	io_out8(PIC0_ICW4, 0x01  ); /* No buffer mode */

	io_out8(PIC1_ICW1, 0x11  ); /* edge trigger mode */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15 -> INT28-2f */
	io_out8(PIC1_ICW3, 2     ); /* PIC1 connected bt IRQ2 */
	io_out8(PIC1_ICW4, 0x01  ); /* No buffer mode */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 Only leave IRQ2 (PIC1) */
	io_out8(PIC1_IMR,  0xff  ); /* Block all interrupts */
}

//void inthandler27(int *esp)
///* Unstable PIC0 Interrupt (Ignore) */
//{
	//io_out8(PIC0_OCW2, 0x67); /* Inform PIC IRQ-07 finished*/
	//return;
//}
