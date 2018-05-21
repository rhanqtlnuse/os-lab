
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

#define LINES 25
#define COLUMNS 80
#define PATTERN_SIZE 512

enum State {
	typing,
	searching,
	done
};

int state;

char input[LINES][COLUMNS];
int current_row;
int current_col;

char pattern[PATTERN_SIZE];
int p_pattern;

PUBLIC void clear_screen();
PRIVATE int strncmp(const char *s1, const char *s2, int len);
PRIVATE void reset_search(TTY *p_tty);

int search();
int strlen(const char *str);

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;

	init_keyboard();

	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);
	while (1) {
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);

	for (int i = 0; i < LINES; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			input[i][j] = 0x00;
		}
	}
	for (int i = 0; i < PATTERN_SIZE; i++) {
		pattern[i] = 0x00;
	}
	// 默认为输入模式
	state = typing;
	p_pattern = 0;
	current_row = 0;
	current_col = 0;
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key)
{
        char output[2] = {'\0', '\0'};
		
			if (!(key & FLAG_EXT)) {
				if (state == typing) {
					put_key(p_tty, key);
					input[current_row][current_col++] = key;
				} else if (state == searching) {
					pattern[p_pattern++] = key;
                    int tmp = disp_pos;
                    disp_pos = 160*23;
                    disp_str(pattern);
                    disp_pos = tmp;
					out_color_char(p_tty->p_console, key, 0x0b);
				}
			}
			else {
				int cursor;
				int space_cnt;
				int raw_code = key & MASK_RAW;
				switch(raw_code) {
					case ESC:
						if (state == typing) {
						    int tmp = disp_pos;
						    disp_pos = 160*24;
						    disp_str("typing to searching");
						    disp_pos = tmp;
							state = searching;
							disable_irq(CLOCK_IRQ);
						} else if (state == done) {
                            int tmp = disp_pos;
                            disp_pos = 160*24;
                            disp_str("done to typing");
                            disp_pos = tmp;
							reset_search(p_tty);
							state = typing;
							enable_irq(CLOCK_IRQ);
						}
						break;
					case TAB:
					    // TODO
					    input[current_row][current_col++] = '\t';
						cursor = p_tty->p_console->cursor;
						space_cnt = 4 - (cursor % 4);
						for (int i = 0; i < space_cnt; i++) {
							put_key(p_tty, ' ');
						}
						break;
					case ENTER:
						if (state == typing) {
							put_key(p_tty, '\n');
							input[current_row][current_col] = '\n';
							current_row++;
							current_col = 0;
						} else if (state == searching) {
                            int tmp = disp_pos;
                            disp_pos = 160 * 21;
                            disp_str(input[0]);
                            disp_pos = 160 * 22;
                            disp_str(input[1]);
                            disp_pos = tmp;
							search();
						}
						break;
					case BACKSPACE:
					    // TODO
					    if (state == typing) {
					        if (current_col > 0) {
                                input[current_row][--current_col] = 0x00;
                                put_key(p_tty, '\b');
                            }
                        } else if (state == searching) {
					        if (p_pattern > 0) {
                                pattern[--p_pattern] = 0x00;
                                put_key(p_tty, '\b');
                            }
					    }
						break;
					case UP:
					    if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					        scroll_screen(p_tty->p_console, SCR_DN);
					    }
					    break;
					case DOWN:
					    if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					        scroll_screen(p_tty->p_console, SCR_UP);
					    }
					    break;
					case F1:
					case F2:
					case F3:
					case F4:
					case F5:
					case F6:
					case F7:
					case F8:
					case F9:
					case F10:
					case F11:
					case F12:
						/* Alt + F1~F12 */
						if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
							select_console(raw_code - F1);
						}
						break;
					default:
						break;
				}
			}
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}

int search() {
	if (strlen(pattern) == 0) {
		return;
	}

	disp_pos = 0;
	for (int i = 0; i <= current_row; i++) {
		for (int j = 0; j < strlen(input[i]); j++) {
			int result = strncmp(input[i] + j, pattern, strlen(pattern));
			if (result != 0) {
				char output[2] = {'\0', '\0'};
				output[0] = input[i][j];
				disp_color_str(output, DEFAULT_CHAR_COLOR);
			} else {
				disp_color_str(pattern, 0x0b);
				j += strlen(pattern) - 1;
			}
		}
	}

	state = done;
}

void reset_search(TTY *p_tty) {
	for (int i = strlen(pattern) - 1; i >= 0; i--) {
		pattern[i] = 0x00;
		put_key(p_tty, '\b');
	}

	disp_pos = 0;
	for (int i = 0; i <= current_row; i++) {
		disp_color_str(input[i], DEFAULT_CHAR_COLOR);
	}
}

PRIVATE int strncmp(const char *s1, const char *s2, int len) {
    for (int i = 0; i < len; i++) {
		if (s1[i] != s2[i]) {
			return s1[i] - s2[i];
		} else {
            if (s1[i] == '\0') {
                return 0;
            }
		}
	}
    return 0;
}

int strlen(const char *str) {
	int len = 0;
	while (str[len] != '\0') {
		len++;
	}
	return len;
}

PUBLIC void clear_screen() {
	disp_pos = 0;
	for (int i = 0; i < LINES; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			disp_color_str(" ", 0x08);
		}
	}
	disp_pos = 0;
	for (TTY *p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_screen(p_tty);
	}
	for (int i = 0; i <= current_row; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			input[i][j] = 0x00;
		}
	}
}
