/* Circular Queue */

#include "bootpack.h"

extern struct TASK *task_kernal;

void fifo_init(struct FIFO *fifo)
/* Initialize FIFO */
{
	fifo->size = BUF_LENGTH;
	fifo->free = BUF_LENGTH; /* buffer size */
	fifo->flags = 0;
	fifo->p = 0; /* next write position */
	fifo->q = 0; /* next read position */
	return;
}

// This method is always called by inthandler
int fifo_put(struct FIFO *fifo, unsigned int data)
/* put data in FIFO */
{
	if (fifo->free == 0) {
		/* No space left: Overrun */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	return 0;
}

// This method is always called by system, thus INT safe
unsigned int fifo_get(struct FIFO *fifo)
/* get data from FIFO */
{
	unsigned int data;
	int eflags;
	if (fifo->free == fifo->size) {
		/* FIFO is empty, return 0xffffffff */
		return 0xffffffff;
	}
	eflags = io_load_eflags();	/* Record current INT state */
	io_cli();
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	io_store_eflags(eflags);	/* Restore INT */
	return data;
}

/* return the current length of the circular queue */
int fifo_status(struct FIFO *fifo)
{
	return fifo->size - fifo->free;
}
