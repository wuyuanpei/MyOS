/* System timer */

#include "bootpack.h"

extern char mtask_on;

struct TMRCTL* first;

void init_pit(void)
{
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e); // 1 interrupt per 0.01 second
	struct SYS_TMR *tmr = (struct SYS_TMR *)SYS_TMR_ADR;
	tmr->time_high = 0;
	tmr->time_low = 0;
	// Initialize the link list prologue
	first = 0;
}

// record the current time and record the expected ending time
static void start_usr_timing(struct USR_TMR *usr_tmr, unsigned int duration)
{
	struct SYS_TMR *tmr = (struct SYS_TMR *)SYS_TMR_ADR;
	usr_tmr->start_high = tmr->time_high;
	usr_tmr->start_low = tmr->time_low;
	usr_tmr->end_high = tmr->time_high;
	usr_tmr->end_low = tmr->time_low;
	// Overflow
	if(usr_tmr->end_low + duration < usr_tmr->end_low){
		usr_tmr->end_high++;
	}
	usr_tmr->end_low += duration;
}

// test whether the usr_tmr passes the expected ending time, if so return 1
static int test_usr_timing(struct USR_TMR *usr_tmr)
{
	struct SYS_TMR *tmr = (struct SYS_TMR *)SYS_TMR_ADR;
	if(tmr->time_high >= usr_tmr->end_high && tmr->time_low >= usr_tmr->end_low){
		return 1;
	}else{
		return 0;
	}
}

void inthandler20(int *esp)
{
	struct SYS_TMR *tmr = (struct SYS_TMR *)SYS_TMR_ADR;
	struct TMRCTL* tmp;
	io_out8(PIC0_OCW2, 0x60);	/* Send to PIC that IRQ-00 finished receiving data */
	// Update system timer
	tmr->time_low++;
	if(tmr->time_low == 0)
		tmr->time_high++;
	// Test nodes in the user timer list
	if(first == 0) goto context_switching;
	while(test_usr_timing(& first->node_data)){
		fifo_put(& first->task->fifo,((int)first->handler) + TIMER_OFFSET);
		task_run(first->task, -1, 0);
		tmp = first;
		first = first->next;
		// corresponding to: struct TMRCTL *node = (struct TMRCTL *)mm_malloc(sizeof(struct TMRCTL));
		mm_free(tmp);
		if(first == 0) goto context_switching;
	}
context_switching:
	if(mtask_on)
		task_switch();
}

void start_timing(unsigned char handler, unsigned int duration){
	int eflags;
	struct TMRCTL *node = (struct TMRCTL *)mm_malloc(sizeof(struct TMRCTL));
	struct TMRCTL *tmp = first;
	struct TMRCTL **tmp_adr = &first;
	struct TMRCTL *next;
	node->handler = handler;
	node->task = task_now();
	start_usr_timing(& node->node_data, duration);
	eflags = io_load_eflags();
	io_cli();
	while(tmp){
		if(tmp->node_data.end_high > node->node_data.end_high){
			break;
		}else if(tmp->node_data.end_high == node->node_data.end_high){
			if(tmp->node_data.end_low > node->node_data.end_low){
				break;
			}else{
				tmp_adr = & tmp->next;
				tmp = tmp->next;
			}
		}else{
			tmp_adr = & tmp->next;
			tmp = tmp->next;
		}
	}
	next = tmp;
	*tmp_adr = node;
	node->next = next; 
	io_store_eflags(eflags);	/* Restore INT */
}
