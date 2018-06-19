
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

#define CHAIRS 2

void cut_hair();
void customer();
void get_hair_cut(int id);
void leave(int id, int flag);
void clear_screen();

#define BUF_SIZE 255

char output[BUF_SIZE];

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
		p_proc->blocked = FALSE;
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
	while (1) {}
}

// 理发师
void TestB() {
	while (1) {
		sem_p(&customers);
		sem_p(&mutex);
		waiting--;
		sem_v(&mutex);
		sem_v(&barbers);
		cut_hair();
	}
}

void cut_hair() {
	my_disp_color_str("barber cuts hair\n", BLUE);
	process_sleep(20000);
}

void customer() {
	while (1) {
		sem_p(&mutex);
		customer_id++;
		int tmp = customer_id;
		if (waiting < CHAIRS) {
			waiting++;

			// 这里出现令人智熄的输出不完整的问题，所以决定先拼在一起，放到一个函数里
			// my_disp_color_str("customer #", GREEN);
			// disp_color_int(customer_id, GREEN);
			// my_disp_color_str(" comes and wait\n", GREEN);
			// 以及为什么在同一个函数中使用两次 disp_str 会 gg？
			process_sleep(10000);
			come(tmp);
			sem_v(&mutex);

			sem_v(&customers);
			sem_p(&barbers);
			process_sleep(5000);
			get_hair_cut(tmp);
		} else {
			process_sleep(5000);
			leave(tmp, 1);
			sem_v(&mutex);
		}
	}
}

void come(int id) {
	strcpy(output, "customer ");
	itoa(output + 9, id);
	strcpy(output + strlen(output), " comes and wait\n");
	my_disp_color_str(output, GREEN);
	clear_buffer();
}

void get_hair_cut(int id) {
	strcpy(output, "customer ");
	itoa(output + 9, id);
	strcpy(output + strlen(output), " gets hair cut\n");
	my_disp_color_str(output, PURPLE);
	clear_buffer();	
	leave(id, 0);
}

void leave(int id, int flag) {
	if (flag) {
		strcpy(output, "no seats, ");
	} else {
		strcpy(output, "got hair cut, ");
	}
	strcpy(output + strlen(output), "customer ");
	itoa(output + strlen(output), id);
	strcpy(output + strlen(output), " leaves\n");
	my_disp_color_str(output, flag ? WHITE : PURPLE);
	clear_buffer();
}

// 顾客
void ProcessC() {
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

void clear_buffer() {
	for (int i = 0; i < BUF_SIZE; i++) {
		output[i] = '\0';
	}
}

int strlen(const char *str) {
	int length = 0;
	int i = 0;
	while (str[i] != '\0') {
		length++;
		i++;
	}
	return length;
}