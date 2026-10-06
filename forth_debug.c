#include "forth.h"

/* todo: is this core or is this lib */

void forth_debug_breakpoint(void) {
    forth_vm_print_ds();
    forth_vm_print_rs();
    printf("[bp] press enter to continue...");
    fflush(stdout);
    getchar();
}

/* todo: is this core or lib.. or ext */