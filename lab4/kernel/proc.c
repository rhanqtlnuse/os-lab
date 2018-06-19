
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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

extern void disable_int();
extern void enable_int();

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	// 找出剩余处理时间最长的进程
	// 如果所有进程的剩余处理时间都为 0，则将每个进程的处理时间赋值为其优先级
	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			if (!p->blocked && p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				if (!p->blocked) {
					p->ticks = p->priority;
				}
			}
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC void sys_sem_p(semaphore *s) {
	disable_int();
	s->value--;
	if (s->value < 0) {
		s->waiting_queue[s->tail] = p_proc_ready;
		s->tail = (s->tail + 1) % 16;
		p_proc_ready->blocked = TRUE;
		schedule();
	}
	enable_int();
}

PUBLIC void sys_sem_v(semaphore *s) {
	disable_int();
	s->value++;
	if (s->value <= 0) {
		PROCESS *p = s->waiting_queue[s->head];
		s->head = (s->head + 1) % 16;
		p->blocked = FALSE;
		schedule();
	}
	enable_int();
}

PUBLIC void sys_process_sleep(int milli_seconds) {
	// clock.c
	// 时间片数 * 1000 / HZ = 毫秒数
	// 则 时间片数 = 毫秒数 * HZ / 1000
	p_proc_ready->sleep = milli_seconds * HZ / 1000;
	schedule();
}

PUBLIC void sys_disp_str(char *str) {
	disp_str(str);
}

PUBLIC void sys_disp_color_str(char *str, int color) {
	disp_color_str(str, color);
}