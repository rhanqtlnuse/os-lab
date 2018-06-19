
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

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule() {
	do {
		p_proc_ready++;
		if (p_proc_ready >= proc_table + NR_TASKS) {
			p_proc_ready = proc_table;
		}
	} while (p_proc_ready->sleep > 0 || p_proc_ready->blocked);
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks() {
	return ticks;
}

PUBLIC void sys_sem_p(semaphore *s) {
	s->value--;
	if (s->value < 0) {
		s->waiting_queue[s->tail] = p_proc_ready;
		s->tail = (s->tail + 1) % 16;
		p_proc_ready->blocked = TRUE;
		schedule();
	}
}

PUBLIC void sys_sem_v(semaphore *s) {
	s->value++;
	if (s->value <= 0) {
		PROCESS *p = s->waiting_queue[s->head];
		s->head = (s->head + 1) % 16;
		p->blocked = FALSE;
		schedule();
	}
}

PUBLIC void sys_process_sleep(int milli_seconds) {
	// clock.c
	// 时间片数 * 1000 / HZ = 毫秒数
	// 则 时间片数 = 毫秒数 * HZ / 1000
	p_proc_ready->sleep = milli_seconds * HZ / 1000;
	schedule();
}

PUBLIC void sys_disp_str(char *str) {
	wind();
	disp_str(str);
}

PUBLIC void sys_disp_color_str(char *str, int color) {
	wind();
	disp_color_str(str, color);
}

void wind() {
	if (disp_pos >= 80 * 25 * 2) {
		clear_screen();
		disp_pos = 0;
	}
}