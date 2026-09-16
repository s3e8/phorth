#include "forth.h"

/* 
    this forth is organized in modules with file-local globals.
    the modules are as follows (though perhaps somewhat loosely in the case of interpreter vs vm):
    - dictionary - ...
    - io - ...
    - interpreter - things having to do with interpreting logic.. is_number, etc
    - vm - all the data and return stack stuff
*/

void forth_interpret_string(const char* str) {
    forth_io_set_string_input(str);
    forth_vm_run();
}

void forth_init_defaults(void) {
    forth_interpreter_init_defaults();
    forth_dictionary_init_defaults();
    forth_vm_init_defaults();
    forth_io_init_defaults();
}

void forth_bootstrap(void) { /* temp: have to call bye to exit */
    // forth_interpret_string(": null-debugger-vector ; bye");
    // forth_interpret_string("' null-debugger-vector debugger-vector ! bye");
    forth_interpret_string(": aligned            cellsize 1- + cellsize 1- invert and ; bye");
    forth_interpret_string(": align              here @ aligned here ! ; bye");
    forth_interpret_string(": allot              here @ swap here +! align ; bye");
    forth_interpret_string(": toggle-flag        dup @ rot xor swap ! ; bye"); /* ( flag addr -- ) */
    forth_interpret_string(": toggle-flag-inline f_inline latest @ toggle-flag ; bye");
    forth_interpret_string(": set-flag           dup @ rot or swap ! ; bye"); /* ( flag addr -- ) */
    forth_interpret_string(": set-flag-inline    f_inline latest @ set-flag ; bye");
    forth_interpret_string(": make-inline        latest @ dup @ f_inline xor swap ! ; bye");
    forth_interpret_string(": compile-lit        ' lit , , ' exit , ' eow , ; bye"); /* ( val -- )*/
    forth_interpret_string(": create-variable    allot create toggle-flag-inline compile-lit ; bye"); /* ( size -- ) */
    forth_interpret_string(":   variable         cellsize      create-variable ; bye");
    forth_interpret_string(":  fvariable         floatsize     create-variable ; bye");
    forth_interpret_string(": v3variable       3 floatsize *   create-variable ; bye");
    forth_interpret_string(": m3variable   3 3 * floatsize *   create-variable ; bye");
    forth_interpret_string(": if      immediate  ' 0branch , here @ 0 ,  ; bye");
    forth_interpret_string(": else    immediate  '  branch , here @ 0 , swap dup here @ swap - swap ! ; bye");
    forth_interpret_string(": then    immediate  dup here @ swap - swap ! ; bye");
    forth_interpret_string(": recurse immediate  ' call , latest @ >xt , ; bye");
    forth_interpret_string(": begin   immediate  here @ ; bye");
    forth_interpret_string(": until   immediate  ' 0branch , here @ - , ; bye");
    forth_interpret_string(": again   immediate  '  branch , here @ - , ; bye");
    forth_interpret_string(": while   immediate  ' 0branch , here @ 0 , ; bye");
    forth_interpret_string(": repeat immediate   '  branch , swap here @ - , dup here @ swap - swap ! ; bye");
}

int main(void) {
    forth_init_defaults();    
    // forth_io_set_input_file("forth_bootstrap.f");
    forth_bootstrap();
    forth_vm_run();

    return 0;
}
