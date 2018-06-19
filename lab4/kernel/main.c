
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

#define DEFAULT_PRIORITY 50
#define CHAIRS 3

void cut_hair();
void customer();
void get_hair_cut(int id);
void leave(int id);
void clear_screen();

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid
		p_proc->sleep = 0;
		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	for (i = 0; i < NR_TASKS; i++) {
		proc_table[i].sleep = 0;
		proc_table[i].blocked = FALSE;
		proc_table[i].ticks = proc_table[i].priority = DEFAULT_PRIORITY;
	}

	clear_screen();

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	initialize();

        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	restart();

	while (1) {}
}

void initialize() {
	waiting = 0;

	customer_id = 0;

	customers.value = 0;
	customers.head = 0;
	customers.tail = 0;

	barbers.value = 0;
	barbers.head = 0;
	barbers.tail = 0;

	mutex.value = 1;
	mutex.head = 0;
	mutex.tail = 0;
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	while (1) {
		milli_delay(10);
	}
}

// 理发师
void TestB()
{
	while(1) {
		sem_p(&customers);
		sem_p(&mutex);
		waiting--;
		sem_v(&barbers);
		sem_v(&mutex);
		cut_hair();
	}
}

void cut_hair() {
	my_disp_color_str("barber cuts hair\n", BLUE);
	milli_delay(2000);
}

void customer() {
	while (1) {
		sem_p(&mutex);
		customer_id++;
		if (waiting < CHAIRS) {
			waiting++;
			my_disp_color_str("customer #", GREEN);
			disp_color_int(customer_id, GREEN);
			my_disp_color_str(" comes and wait\n", GREEN);
			milli_delay(1000);
			sem_v(&customers);
			sem_v(&mutex);
			sem_p(&barbers);
			get_hair_cut(customer_id);
		} else {
			sem_v(&mutex);
			leave(customer_id);
		}
	}
}

void get_hair_cut(int id) {
	my_disp_color_str("customer #", PURPLE);
	disp_color_int(id, PURPLE);
	my_disp_color_str(" gets hair cut\n", PURPLE);
	milli_delay(2000);
	my_disp_color_str("customer #", PURPLE);
	disp_color_int(id, PURPLE);
	my_disp_color_str(" leaves\n", PURPLE);
}

void leave(int id) {
	milli_delay(1000);
	my_disp_color_str("no seats, customer #", WHITE);
	disp_color_int(id, WHITE);
	my_disp_color_str(" leaves\n", WHITE);
}

// 顾客
void TestC() {
	customer();
}

// 顾客
void ProcessD() {
	customer();
}

// 顾客
void ProcessE() {
	customer();
}

void clear_screen() {
	disp_pos = 0;
	for (int i = 0; i < 80 * 25; i++) {
		disp_str(" ");
	}
	disp_pos = 0;
}
