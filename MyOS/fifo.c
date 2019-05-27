/* Circular Queue */

#include "bootpack.h"

extern struct TASK *task_kernal;

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
/* Initialize FIFO */
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* buffer size */
	fifo->flags = 0;
	fifo->p = 0; /* next write position */
	fifo->q = 0; /* next read position */
	return;
}

// This method is always called by inthandler
int fifo8_put(struct FIFO8 *fifo, unsigned char data)
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
	task_run(task_kernal, -1, 0);
	return 0;
}

// This method is always called by system, thus INT safe
unsigned char fifo8_get(struct FIFO8 *fifo)
/* get data from FIFO */
{
	unsigned char data;
	int eflags;
	if (fifo->free == fifo->size) {
		/* FIFO is empty, return -1 */
		return -1;
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
int fifo8_status(struct FIFO8 *fifo)
{
	return fifo->size - fifo->free;
}
