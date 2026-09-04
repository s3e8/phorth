/* forth_interpreter.c */
#include "forth.h"

int state = STATE_IMMEDIATE;
int base  = 10;  /* todo: is this interpreter or vm? */

void* call_code;
void* lit_code;

// void forth_interpreter_debug_state(void) {
//     int ch = forth_io_get_char();
//     switch(ch) {
//         case atoi("s"):
            // break;
//     }
// }

void forth_interpreter_init_defaults(void) {
    state = STATE_IMMEDIATE; /* todo: probably don't need this here */
    base  = 10;
}

int forth_interpreter_parse_number(const char* wordbuf, int* result) {
    char* endptr = NULL;
    cell val = (cell)strtol(wordbuf, &endptr, base);

    printf("parse_number: '%s' -> val=%ld, endptr points to '%s', base=%d\n", 
        wordbuf, (long)val, endptr, base);
    
    if(*endptr == '\0') {
        *result = val;
        return 1;
    }

    return 0;
}

int forth_interpreter_interpret(void) {
    /* todo: if !forth_initialized return 1 with err */
    char* wordbuf = forth_io_get_next_word();
    if(!wordbuf) return 1;

    word_header_t* word = forth_dictionary_find_word(wordbuf);
    if(word) {
        void* code = forth_dictionary_get_xt(word);
        if(state == STATE_COMPILE && !(word->flags & FLAG_IMMEDIATE)) {
            if(word->flags & FLAG_BUILTIN) {
                forth_dictionary_compile((cell)code);
            }
            else {
                forth_dictionary_compile((cell)call_code);
                forth_dictionary_compile((cell)code);
            }
        } else { /* execute */
            forth_vm_push_ns();
            // *--nestingstack = current_ip;
            if(word->flags & FLAG_BUILTIN) {
                // builtin_immediatebuf[0] = *code;
                forth_vm_schedule_builtin(code);
                // current_ip = builtin_immediatebuf;
            }
            else { /* interpret */
                // word_immediatebuf[1] = (void*)code;
                forth_vm_schedule_word(code);
                // current_ip = word_immediatebuf;
            }
            /* move on to NEXT() and run ip */
            // NEXT();
            return 0;
        }
    }

    else { /* check if word is a number */
        int number; /* todo: make is_number more forth friendly for builtin? 
            * maybe not cause we define a new is_number in forth later */
        int is_number = forth_interpreter_parse_number(wordbuf, &number);

        /* if number, compile or interpret (push to stack) */
        if(is_number) {
            if(state == STATE_COMPILE) {
                forth_dictionary_compile((cell)lit_code);
                forth_dictionary_compile((cell)number);
            }
            else forth_vm_push_ds((cell)number);
        }
        else fprintf(stderr, "Error: no such word: %s\n", wordbuf);

        /* move on to NEXT() and run ip */
        // NEXT();
    }

    return 0;
}
