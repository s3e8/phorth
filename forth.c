#include "forth.h"

/* 
    this forth is organized in modules with file-local globals.
    the modules are as follows (though perhaps somewhat loosely in the case of interpreter vs vm):
    - dictionary - ...
    - io - ...
    - interpreter - things having to do with interpreting logic.. is_number, etc
    - vm - all the data and return stack stuff
*/

int main(void) {
    forth_interpreter_init_defaults();
    forth_dictionary_init_defaults();
    forth_vm_init_defaults();
    forth_io_init_defaults();
    forth_io_set_input_file("forth_bootstrap.f");

    forth_vm_run();

    return 0;
}
