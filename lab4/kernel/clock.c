
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c
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
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	ticks++;

	// 只有在未睡眠的情况下才能继续执行
	if (p_proc_ready->sleep == 0 && !p_proc_ready->blocked) {
		p_proc_ready->ticks--;
	}
	
	// 减少睡眠时间
	for (int i = 0; i < NR_TASKS; i++) {
		if (proc_table[i].sleep > 0 && !proc_table[i].blocked) {
			proc_table[i].sleep--;
		}
	}

	if (k_reenter != 0) {
		return;
	}

	if (p_proc_ready->ticks > 0) {
		return;
	}

	schedule();

}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
    int t = get_ticks();

    while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

