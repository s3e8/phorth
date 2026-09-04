#include "forth.h"

void forth_debug_breakpoint(void) {
    forth_vm_print_ds();
    forth_vm_print_rs();
    printf("[bp] press enter to continue...");
    fflush(stdout);
    getchar();
}