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
    forth_interpret_string(": aligned             cellsize 1- + cellsize 1- invert and ; bye");
    forth_interpret_string(": align               here @ aligned here ! ; bye");
    forth_interpret_string(": allot               here @ swap here +! align ; bye");
    forth_interpret_string(": ?builtin            @ f_builtin and ; bye");
    forth_interpret_string(": set-flag            dup @ rot  or swap ! ; bye"); /* ( flag addr -- ) */
    forth_interpret_string(": toggle-flag         dup @ rot xor swap ! ; bye"); /* ( flag addr -- ) */
    forth_interpret_string(": set-flag-inline     f_inline latest @    set-flag ; bye");
    forth_interpret_string(": toggle-flag-inline  f_inline latest @ toggle-flag ; bye");
    forth_interpret_string(": inline immediate    toggle-flag-inline ; bye");
    forth_interpret_string(": compile-lit         ' lit , , ' exit , ' eow , ; bye"); /* ( val -- )*/
    forth_interpret_string(": create-variable     allot create toggle-flag-inline compile-lit ; bye"); /* ( size -- ) */
    forth_interpret_string(":   variable          cellsize      create-variable ; bye");
    forth_interpret_string(":  fvariable          floatsize     create-variable ; bye");
    forth_interpret_string(": v3variable        3 floatsize *   create-variable ; bye");
    forth_interpret_string(": m3variable    3 3 * floatsize *   create-variable ; bye");
    forth_interpret_string(": if      immediate   ' 0branch , here @ 0 ,  ; bye");
    forth_interpret_string(": else    immediate   '  branch , here @ 0 , swap dup here @ swap - swap ! ; bye");
    forth_interpret_string(": then    immediate   dup here @ swap - swap ! ; bye");
    forth_interpret_string(": recurse immediate   ' call , latest @ >xt , ; bye");
    forth_interpret_string(": begin   immediate   here @ ; bye");
    forth_interpret_string(": until   immediate   ' 0branch , here @ - , ; bye");
    forth_interpret_string(": again   immediate   '  branch , here @ - , ; bye");
    forth_interpret_string(": while   immediate   ' 0branch , here @ 0 , ; bye");
    forth_interpret_string(": repeat  immediate   '  branch , swap here @ - , dup here @ swap - swap ! ; bye");
    forth_interpret_string(": compile-call        dup ?builtin if >xt , else ' call , >xt , then ; bye");
    forth_interpret_string(": [compile] immediate word find compile-call ; bye");
    forth_interpret_string(": unless  immediate   ' 0= , [compile] if ; bye");
    forth_interpret_string(": case    immediate   0 ; bye");
    forth_interpret_string(": (of)    immediate   [compile] if ' drop , ; bye");
    forth_interpret_string(":  of     immediate   ' over , ' = , [compile] if ' drop , ; bye");
    forth_interpret_string(": endof   immediate   [compile] else ; bye");
    forth_interpret_string(": endcase immediate   ' drop , begin ?dup while [compile] then repeat ; bye");
    forth_interpret_string(": '\n'    inline      10 ; bye");
    forth_interpret_string(":  cr     inline      10 emit ; bye");
    forth_interpret_string(": literal immediate   ' lit , , ; bye");
    forth_interpret_string(": char                word c@ ; bye");
    forth_interpret_string(": ':'     inline      [ char  : ] literal ; bye");
    forth_interpret_string(": ';'     inline      [ char  ; ] literal ; bye");
    forth_interpret_string(": '('     inline      [ char  ( ] literal ; bye");
    forth_interpret_string(": ')'     inline      [ char  ) ] literal ; bye");
    forth_interpret_string(": '\"'    inline      [ char \" ] literal ; bye");
    forth_interpret_string(": 'A'     inline      [ char  A ] literal ; bye");
    forth_interpret_string(": '0'     inline      [ char  0 ] literal ; bye");
    forth_interpret_string(": '-'     inline      [ char  - ] literal ; bye");
    forth_interpret_string(": '.'     inline      [ char  . ] literal ; bye");
    forth_interpret_string(": cell+   inline      cellsize + ; bye");
    forth_interpret_string(": cell-   inline      cellsize - ; bye");

    forth_interpret_string(": constalign          consthere @    aligned           consthere ! ; bye");
    forth_interpret_string(": c,                  here      @ c! here      @ 1+    here      ! ; bye");
    forth_interpret_string(": const,              consthere @  ! consthere @ cell+ consthere ! ; bye");
    forth_interpret_string(": constc,             consthere @ c! consthere @ 1+    consthere ! ; bye");

    // forth_interpret_string(": read-string-into-const   consthere @ begin key dup '\"' <> while constc,    repeat drop 0 constc, ; bye"); /* -- startpos ) */ 
    // forth_interpret_string(": read-string-into-scratch consthere @ begin key dup '\"' <> while over c! 1+ repeat drop 0 over c! drop consthere @ ; bye"); /* ( -- startpos ) */
    // forth_interpret_string(": read-string-into-scratch consthere @ dup begin key dup '\"' <> while over c! 1+ repeat drop 0 over c! drop ; bye");
    // forth_interpret_string(": s\" immediate            state @ if read-string-into-const ' lit , , constalign else read-string-into-scratch then ; bye");
    forth_interpret_string(": s\" immediate state @ if consthere @ begin key dup '\"' <> while constc, repeat drop 0 constc, ' lit , , constalign else consthere @ begin key dup '\"' <> while over c! 1+ repeat drop 0 over c! drop consthere @ then ; bye");
}

int main(void) {
    forth_init_defaults();    
    forth_io_set_input_file("forth_bootstrap.f");
    // forth_bootstrap();
    forth_vm_run();

    return 0;
}
