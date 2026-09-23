/* forth_lib_sys_tty.c */
#include   "forth.h"

#ifdef INCLUDE_LIB_SYS_TTY

#include "sys_tty.h"

void forth_cleanup_lib_sys_tty(void) {
    sys_tty_disable_raw_mode();
}

void forth_lib_sys_tty_enable_raw_mode(void) {
    sys_tty_enable_raw_mode();
}

void forth_lib_sys_tty_disable_raw_mode(void) {
    sys_tty_disable_raw_mode();
}

void forth_lib_sys_tty_clear_screen(void) {
    sys_tty_clear_screen();
}

void forth_lib_sys_tty_read_byte(void) {
    forth_vm_push_ds(sys_tty_read_byte());
}

void forth_include_lib_sys_tty(void) {
    forth_dictionary_defextern("sys-hello", sys_hello, 0);
    forth_dictionary_defextern("tty-enable-raw-mode",  forth_lib_sys_tty_enable_raw_mode, 0);
    forth_dictionary_defextern("tty-disable-raw-mode", forth_lib_sys_tty_disable_raw_mode, 0);
    forth_dictionary_defextern("tty-clear-screen",     forth_lib_sys_tty_clear_screen, 0);
    forth_dictionary_defextern("tty-read-byte",        forth_lib_sys_tty_read_byte, 0);    
}

#endif