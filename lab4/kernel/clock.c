
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
	
	// 减少睡眠时间
	for (int i = 0; i < NR_TASKS; i++) {
		if (proc_table[i].sleep > 0 && !proc_table[i].blocked) {
			proc_table[i].sleep--;
		}
	}

	if (k_reenter != 0) {
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

