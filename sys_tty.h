#ifndef SYS_TTY_H
#define SYS_TTY_H

extern void sys_tty_enable_raw_mode(void);
extern void sys_tty_disable_raw_mode(void);
extern void sys_tty_clear_screen(void);
extern void sys_tty_get_terminal_window_size(int* rows, int* cols);
extern int  sys_tty_read_byte(void);
extern void sys_tty_write(const char* buf, int len);
extern void sys_tty_clear_screen(void);

#endif /* SYS_TTY_H */