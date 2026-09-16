/* tty.h */
#ifndef TTY_H
#define TTY_H

/* todo: should these return statuses or void? */
extern void tty_enable_raw_mode(void);
extern void tty_disable_raw_mode(void);
extern void tty_clear_screen(void);
extern void tty_get_terminal_width(void);
extern void tty_get_terminal_height(void);

#endif /* TTY_H */