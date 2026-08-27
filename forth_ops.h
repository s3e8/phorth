/* forth_ops.h */
#ifndef FORTH_OPS_H
#define FORTH_OPS_H

/* ops helpers */
/* todo: macro that inserts goto CODE(DEBUG) maybe.. not sure what best practices are here */
#define NEXT() goto **current_ip++
#define   OP(name)    op_##name
#define CODE(name)  &&op_##name /* todo: rename to LABEL? */
// #define EXTERNAL(fn) { fn; CODE(EXTERNAL) } /* todo: this (for "third-party" builtins defined after initialization)*/
#define OFFSET(x)   (void*)(x * sizeof(cell))
#define RS_ARG()    (*current_ip++)
#define RS_INTARG() ((cell)(*current_ip++))

#define AT(x)       (*(current_ds+(x)))
#define FAT(x)      (*(current_fs+(x)))

/* ops */ /* todo: should I add _CODE suffix for clarity? */
#define DIE()           return 0;
#define BYE()           goto OP(DIE);
#define EOW()           /* do nothing */
#define NOOP()          /* do nothing */
#define EXIT()          current_ip = forth_vm_pop_rs();
#define IRETURN()       current_ip = *nestingstack++;
#define LIT()           forth_vm_push_ds(RS_INTARG());
#define LEFT_BRACKET()  state = STATE_COMPILE;
#define RIGHT_BRACKET() state = STATE_IMMEDIATE;
#define LATEST()        forth_vm_push_ds((cell)&latest);
#define IMMEDIATE()     latest->flags ^= FLAG_IMMEDIATE;
#define EMIT()          forth_io_emit((int)forth_vm_pop_ds());
#define TELL()          forth_io_tell((char*)forth_vm_pop_ds());
#define DOT()           forth_io_dot(forth_vm_pop_ds());

#define BRANCH()        \
    temp = RS_INTARG(); \
    current_ip += (temp / sizeof(void*)) - 1;

#define CALL()                      \
    void* fn = RS_ARG();            \
    forth_vm_push_rs(current_ip);   \
    current_ip = fn;

#define COLON()                                         \
    char* name = forth_io_get_next_word();              \
    forth_dictionary_create_word(name, FLAG_HIDDEN);    \
    state = STATE_COMPILE;

#define ZERO_BRANCH()   \
    temp = RS_INTARG(); \
    if(!forth_vm_pop_ds()) current_ip += (temp / sizeof(void*)) - 1;

/* todo: make sure the naming for branch bytecodes is correct */
#define IF_BRANCH()     \
    temp = RS_INTARG(); \
    if(forth_vm_pop_ds()) current_ip += (temp / sizeof(void*)) - 1;

#define JUMP()              \
    void* fn = RS_ARG();    \
    current_ip = fn;

#define CREATE()                                \
    char* next_word = forth_io_get_next_word(); \
    forth_dictionary_create_word(next_word, 0);

#define WORD()                                  \
    char* next_word = forth_io_get_next_word(); \
    forth_vm_push_ds((cell)next_word);

#define FIND()                              \
    char* word = (char*)forth_vm_pop_ds();  \
    forth_dictionary_find_word(word);

#define HIDDEN()                                                \
    word_header_t* word = (word_header_t*)forth_vm_pop_ds();    \
    word->flags ^= FLAG_HIDDEN;

#define TICK() /* todo: cleanup */                                  \
    char* next_word = forth_io_get_next_word();                     \
    word_header_t* word = forth_dictionary_find_word(next_word);    \
    cell code;                                                      \
    if(word == NULL) {                                              \
        fprintf(stderr, "Error: no such word: %s\n", next_word);    \
        NEXT();                                                     \
    } else {                                                        \
        void* code = forth_dictionary_get_xt(word);                 \
}                                                                   \
    if(state == STATE_IMMEDIATE) forth_vm_push_ds(code);            \
    else {                                                          \
        forth_dictionary_compile((cell)CODE(LIT));                  \
        forth_dictionary_compile(code);                             \
    }

#define COMMA()                     \
    cell val = forth_vm_pop_ds();   \
    forth_dictionary_compile(val);

#define FETCH()                                 \
    cell* address = (cell*)forth_vm_pop_ds();   \
    forth_vm_push_ds(*address);     

#define STORE() \
    cell* ptr = (cell*)forth_vm_pop_ds(); \
    temp = forth_vm_pop_ds(); \
    *ptr = temp;

#define CSTORE() \
    char* ptr = (char*)forth_vm_pop_ds(); \
    temp = forth_vm_pop_ds(); \
    *ptr = (char)temp;  

#define ADD() \
    temp = forth_vm_pop_ds(); \
    AT(0) += temp;

#define INTERPRET()                                                         \
    char* wordbuf = forth_io_get_next_word();                               \
    if(!wordbuf) return 1;                                                  \
    word_header_t* word = forth_dictionary_find_word(wordbuf);              \
    if(word) {                                                              \
        void* code = forth_dictionary_get_xt(word);                           \
        if(state == STATE_COMPILE && !(word->flags & FLAG_IMMEDIATE)) {     \
            if(word->flags & FLAG_BUILTIN) {                                \
                forth_dictionary_compile((cell)code);                      \
            } else { /* todo: use getcode here? so we can move the          \
                function into the interpreter module                        \
                and interpret outside of the outer interpreter loop */      \
                forth_dictionary_compile((cell)CODE(CALL));                 \
                forth_dictionary_compile((cell)code);                       \
            }                                                               \
        } else {                                                            \
            forth_vm_push_ns();                                             \
            if(word->flags & FLAG_BUILTIN) {                                \
                builtin_immediatebuf[0] = code;                             \
                current_ip = builtin_immediatebuf;                          \
            } else {                                                        \
                word_immediatebuf[1] = code;                                \
                current_ip = word_immediatebuf;                             \
            }                                                               \
            NEXT();                                                         \
        }                                                                   \
    }                                                                       \
    else {                                                                  \
        /* check if word is a number */                                     \
        int number; /* todo: make is_number more forth-friendly             \
        for builtin? maybe not cause we define a new is_number              \
        in forth later */                                                   \
        int is_number = forth_interpreter_parse_number(wordbuf, &number);   \
        if(is_number) {                                                     \
            if(state == STATE_COMPILE) {                                    \
                forth_dictionary_compile((cell) CODE(LIT));                 \
                forth_dictionary_compile((cell) number);                    \
            }                                                               \
            else forth_vm_push_ds((cell)number);                            \
        }                                                                   \
        else fprintf(stderr, "Error: no such word: %s\n", wordbuf);         \
        /* move on to NEXT() and run ip */                                  \
        NEXT();                                                             \
    }

// #define INTERPRET() {                 \
//     forth_interpreter_interpret();  \
//         NEXT();                     \
//     }

#endif /* FORTH_OPS_H */