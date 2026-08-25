/* FORTH_H */
#ifndef FORTH_H
#define FORTH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "forth_ops.h"

#define DEFAULT_DICTIONARY_SIZE         10*1024*1024
#define DEFAULT_WORD_NAME_MAX_LENGTH    32

#define BIT(x) (1<<(x))
#define FLAG_HIDDEN     BIT(0)
#define FLAG_IMMEDIATE  BIT(1)
#define FLAG_BUILTIN    BIT(2)
#define FLAG_HASARG     BIT(3)
#define FLAG_INLINE     BIT(4)
#define FLAG_DEFERRED   BIT(5)

#define STATE_IMMEDIATE 0
#define STATE_COMPILE   1

typedef intptr_t    cell;
typedef void**      xt;
typedef struct      word_header {
    cell                flags;
    struct word_header* next;
    char                name[DEFAULT_WORD_NAME_MAX_LENGTH];
} word_header_t;

/* dictionary */
extern word_header_t*   latest;
extern void*            dictionary_pointer; /* todo: dp? */
extern void*            dictionary_base;
extern cell             dictionary_size;

/* interpreter */
extern int state;
extern int base;
extern void* builtin_immediatebuf[2];/* todo: this is arguably "vm" */
extern void* word_immediatebuf[3]; /* as is this... */
extern xt call_code; /* todo: codes not working as global */
extern xt lit_code;

/* forth interpreter */
extern void forth_interpreter_init_defaults(void);
extern int  forth_interpreter_parse_number(const char* wordbuf, int* result);
extern int  forth_interpreter_interpret(void);
extern int  forth_interpreter_interpret_string(const char* str);

/* forth io */
extern void  forth_io_init_defaults(void);
extern void  forth_io_set_input_file(const char* filename);
extern char* forth_io_get_next_word(void);                                                  /* WORD */
extern void  forth_io_read_string(const char* str);
/* other forth io ops */
void forth_io_emit(int ch);
void forth_io_tell(const char* str);

// void forth_io_test_all(void);

/* forth dictionary */
extern void             forth_dictionary_init_defaults(void);
extern word_header_t*   forth_dictionary_find_word(const char* name);                       /* FIND   */
extern word_header_t*   forth_dictionary_create_word(const char* name, cell flags);         /* CREATE */
extern void             forth_dictionary_compile(cell value);                               /* COMMA  */
extern xt               forth_dictionary_get_cfa(word_header_t* word);                      /* CFA    */
extern xt               forth_dictionary_get_cfa_by_name(const char* name);
extern const char*      forth_dictionary_get_name_by_cfa(xt cfa);
extern void             forth_dictionary_defcode(const char* name, void* code, cell flags);
extern void             forth_dictionary_defconst(const char* name, cell value);

/* forth vm */
extern void forth_vm_init_defaults(void);
extern int  forth_vm_run(void);
extern void forth_vm_push_ns(void);
extern void forth_vm_push_ds(cell number);
extern void forth_vm_schedule_builtin(xt code);
extern void forth_vm_schedule_word(xt code);

extern void forth_vm_dbg_print_ds(void);
extern void forth_vm_dbg_print_rs(void);

/* tests */
void forth_io_test_all(void);

#endif /* FORTH_H */
