/**
关于多任务系统，我们将任务们划分为多个等级
每个任务有自己的等级(level)和运行时间(priority)
每当一个时间中断来临，如果正好一个任务的运行时间结束
我们的多任务系统会先找到此刻含有任务的最高等级
然后在这个等级中执行任务
如果这个等级和这个正好结束的任务是一个等级
将会运行这个等级的下一个任务
如果该等级只有只有这一个任务 将会重新运行这个任务

因此，如果一个高等级的任务不结束，低等级的任务永远无法运行
和正在运行的任务同等级的任务将会依次运行

当task_run方法改变了一个任务的等级，或在某个等级加入了一个新任务
下一次切换任务时将重新检索等级
*/
#include "bootpack.h"

char mtask_on = 0;

struct TASKCTL *taskctl;

/* Return the current running task */
struct TASK *task_now(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}

/* Add a task to the corresponding level */
static void task_add(struct TASK *task)
{
	struct TASKLEVEL *tl = & taskctl->level[task->level];
	if(tl->running < MAX_TASKS_LV){
		tl->tasks[tl->running] = task;
		tl->running++;
		task->flags = TASK_RUNNING;
	}
}

/* Remove a task from the corresponding level */
static void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];

	/* Iterate the list to find task */
	for (i = 0; i < tl->running; i++) {
		if (tl->tasks[i] == task) {
			break;
		}
	}

	tl->running--;
	if (i < tl->now) {
		tl->now--; /* now-- for running the same task after the movings */
	}
	if (tl->now >= tl->running) {
		/* Adjust now */
		tl->now = 0;
	}
	task->flags = TASK_USED;
	/* Move */
	for (; i < tl->running; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}
}

/* Decide whether or not to change level during each switching */
static void task_switchsub(void)
{
	int i;
	/* Find the highest level that contains at least one running task */
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].running > 0) {
			break;
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
}

/* An idle task at the least important level. It will be run when no task left */
static void task_idle(void)
{
	while(1)
		io_hlt();
}

struct TASK *task_init(void)
{
	int i;
	struct TASK *task, *idle;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl = (struct TASKCTL *) mm_malloc(sizeof (struct TASKCTL)); // 11d2d8 B
	// Initialize tasks0 array and GDT
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].priority = 1; // Default priority for all tasks
		taskctl->tasks0[i].flags = TASK_UNUSED;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	// Initialize all levels
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].running = 0;
		taskctl->level[i].now = 0;
	}
	// Set the system kernal as a task
	task = task_alloc();
	task->flags = TASK_RUNNING; // System kernal is running
	task->level = 0; // Highest level
	task_add(task); // Add the task
	taskctl->now_lv = 0; // Set the current level
	taskctl->lv_change = 0; // Set the lv_change tag
	load_tr(task->sel);	// Set task register

	idle = task_alloc();
	idle->tss.esp = mm_malloc(64) + 64; // 64 B stack
	idle->tss.eip = (int) &task_idle;
	idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8; // The same segment as the kernal
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 0);

	mtask_on = 1;
	return task;
}

struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == TASK_UNUSED) {
			task = & taskctl->tasks0[i];
			fifo_init(&task->fifo); /* Init the fifo of this task */
			task->flags = TASK_USED; /* The task is used */
			task->tss.eflags = 0x00000202; /* IF = 1; allow interrupt */
			task->tss.eax = 0; /* Set all registers to 0 first */
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0; /* No unused task */
}

void task_run(struct TASK *task, int level, int priority)
{
	if (level < 0) {
		level = task->level; /* Do not change the level */
	}
	// priority == 0, keep original priority
	if(priority > 0){
		task->priority = priority;
	}
	/* Level changed */
	if (task->flags == TASK_RUNNING && task->level != level) {
		task_remove(task); /* flags changed to TASK_USED, the following If will be run */
	}
	if (task->flags != TASK_RUNNING) {
		/* Add the task to the list */
		task->level = level;
		task_add(task);
	}

	taskctl->lv_change = 1; /* Need to check LEVEL during next time context switching */
}

// Count tmr_count to taskctl->tasks[taskctl->now]->priority to switch the task
static int tmr_count = 0;

void task_switch(void)
{	
	struct TASKLEVEL *tl = & taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->tasks[tl->now];
	tmr_count++;
	// The task has to run for the defined time 
	if(tmr_count == now_task->priority){
		tmr_count = 0;
	}else{
		return;
	}

	tl->now++; // Move to the next task
	if (tl->now == tl->running) {
		tl->now = 0;
	}
	// Change to a new level if necessary
	if (taskctl->lv_change != 0) {
		task_switchsub();
		tl = &taskctl->level[taskctl->now_lv];
	}
	// New_task
	new_task = tl->tasks[tl->now];
	if (new_task != now_task) {
		farjmp(0, new_task->sel);
	}
}

void task_sleep(struct TASK *task)
{
	struct TASK *now_task;
	if (task->flags == TASK_RUNNING) {
		now_task = task_now();
		task_remove(task); /* Remove the task */
		if (task == now_task) {
			/* If a task wants to remove itself */
			task_switchsub();
			now_task = task_now(); /* Get the current task */
			farjmp(0, now_task->sel);
		}
	}
	return;
}
