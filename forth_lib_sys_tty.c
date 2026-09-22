/* forth_lib_sys_tty.c */
#include   "forth.h"

#ifdef INCLUDE_LIB_SYS_TTY

#include "sys_tty.h"

void forth_cleanup_lib_sys_tty(void) {
    sys_tty_disable_raw_mode();
}

void forth_include_lib_sys_tty(void) {
    forth_dictionary_defextern("sys-hello", sys_hello, 0);
    forth_dictionary_defextern("sys-tty-enable-raw-mode",  sys_tty_enable_raw_mode,  0);
    forth_dictionary_defextern("sys-tty-disable-raw-mode", sys_tty_disable_raw_mode, 0);
}

#endif