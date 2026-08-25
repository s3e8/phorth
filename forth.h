/* forth.h */

#ifndef FORTH_H
#define FORTH_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <float.h>
// #include <unistd.h>
// #include <sys/syscall.h>
// #include <sys/mman.h>
// #include <dyncall.h>
// #include <dynload.h>
// #include <readline/readline.h>
// #include <readline/history.h>

#ifdef USE_GC
    #include <gc.h>
    #define MALLOC(x) GC_MALLOC(x)
    #define MALLOC_ATOMIC(x) GC_MALLOC_ATOMIC(x)
    #define REALLOC(ptr,newlen) GC_REALLOC(ptr,newlen)
    #define RUNGC() GC_gcollect()
    #define FREE(x)
#else
    #define MALLOC(x) malloc(x)
    #define MALLOC_ATOMIC(x) malloc(x)
    #define REALLOC(ptr,newlen) realloc(ptr,newlen)
    #define RUNGC()
    #define FREE(x) free(x)
#endif

#define FORTH_VERSION 1

#define DEFAULT_DICTIONARY_SIZE         10*1024*1024
#define DEFAULT_WORD_NAME_MAX_LENGTH    128
#define DEFAULT_NESTINGSTACK_MAX_DEPTH  128

#define BIT(x) (1<<(x))
#define FLAG_HIDDEN      BIT(0)
#define FLAG_IMMEDIATE   BIT(1)
#define FLAG_BUILTIN     BIT(2)
#define FLAG_HASARG      BIT(3)
#define FLAG_INLINE      BIT(4)
#define FLAG_DEFERRED    BIT(5)

#define STATE_IMMEDIATE 0
#define STATE_COMPILE   1

/* the most important type, the cell. MUST be exactly of the pointer length! */
/* preprocessor trick to test sizeof(long)==sizeof(void*)? */
typedef intptr_t cell; // todo: use long long or intptr or uintptr?
typedef void**   xt;
typedef struct   word_header {  /* dictionary definition header. NEVER change the order of these fields, it's crucial! */
    cell                flags;
    struct word_header* next;
    char                name[DEFAULT_WORD_NAME_MAX_LENGTH];
} word_header_t;

/* forth dictionary */
/* utility structure for creating builtins */ // todo: rm builtin struct
typedef struct builtin_word_t {
     char* name;
     void* code;
     cell flags;
} builtin_word_t;
extern word_header_t*   dictionary_latest;
extern void*            dictionary_pointer; /* todo: dp? */
extern void*            dictionary_base;
extern cell             dictionary_size;
extern word_header_t*   forth_dictionary_create_word(const char* name, cell flags);
extern word_header_t*   forth_dictionary_find_word(const char* name);
extern void             forth_dictionary_compile(cell value);
extern void**           forth_dictionary_get_cfa(word_header_t* word);
// extern word_header_t*   forth_dictionary_find_word_by_cfa(const char* name);
// extern void**           forth_dictionary_get_cfa_by_name(char* name);
extern void* get_builtin(const char* name);
extern void assemble_word(const char *name, cell flags, void **code, cell codesize);
extern void create_constant(const char *name, cell value);
extern void create_fconstant(const char *name, float value);
extern void create_builtin(builtin_word_t *b);
extern void* getcode(const char* name);
extern void forth_dictionary_defcode(const char* name, void* code, cell flags);
extern void forth_dictionary_defword(const char* name, void* code[], int wordcount, cell flags);
extern void forth_dictionary_defconst(const char* name, cell value);
extern void forth_dictionary_deffconst(const char* name, float value);

/* forth io */
extern void  forth_io_set_input_stream(FILE* input_stream);
extern void  forth_io_set_output_stream(FILE* output_stream);
extern void  forth_io_set_wordbuf(char* wordbuf, int size);
extern void  forth_io_set_linebuf(char* linebuf, int size);
extern FILE* forth_io_open_or_create_file(const char* filename, const char* mode);

typedef struct reader_state_t 
{
    FILE* stream;
    cell  linebuf_size;
    char* linebuf;
    char* remaining_chars;
} reader_state_t;
extern void             init_reader_state(reader_state_t* state, char* linebuf, cell linebuf_size, FILE* fp);
extern reader_state_t*  open_file(const char *filename, const char *mode);
extern void             close_file(reader_state_t* fp);
extern void             skip_whitespaces(reader_state_t* state); /* todo: can stay local */
extern cell             is_eol(reader_state_t* state);
extern cell             is_eof(reader_state_t* fp);
extern char*            forth_io_get_next_line(reader_state_t* state);
extern char*            prompt_line(const char* prompt, reader_state_t* state);
extern int              read_key(reader_state_t *state);
extern char*            forth_io_get_next_word(reader_state_t *state, char* tobuf);
extern void             emit_char(int c, FILE* fp);

// typedef struct forth_vm_s {
//     xt      instruction_pointer;
//     xt*     returnstack;
//     xt*     returnstack_base;
//     // xt*     current_ns;
//     // xt*     nestingstack_base;
//     cell*   datastack;
//     cell*   datastack_base;
//     cell*   tempstack;
//     cell*   tempstack_base;
//     float*  floatstack;
//     float*  floatstack_base;

//     // char*               thread_name;
//     cell                killed;
//     struct forth_vm_s*  next;
// } forth_vm_t;
// extern forth_vm_t* forth_vm_init_thread();

// extern int state;
// extern int base;

// extern void builtin_immediatebuf[2];
// extern void word_immediatebuf[3];
// extern xt call_code;
// extern xt lit_code;

// extern char* current_wordbuf;
// extern int   current_wordbuf_size;


extern void interpret(void **current_ip, cell *current_ds, void ***current_rs, reader_state_t *inputstate, FILE *outp, int argc, char **argv);
 

#endif /* FORTH_H */
