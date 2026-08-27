/* forth_dictionary.c */
#include "forth.h"

void* dictionary_pointer; /* todo: dp? */
void* dictionary_base;
cell  dictionary_size;

word_header_t* latest = NULL;

void forth_dictionary_init_defaults() {
    dictionary_base     = malloc(DEFAULT_DICTIONARY_SIZE);
    dictionary_pointer  = dictionary_base;
    dictionary_size     = DEFAULT_DICTIONARY_SIZE;
}

word_header_t* forth_dictionary_create_word(const char* name, cell flags) {
    if(!name) name = "\0";

    printf("creating word: '%s'\n", name);

    word_header_t* new = (word_header_t*)dictionary_pointer;
    dictionary_pointer += sizeof(word_header_t); /* todo: size of type or struct or either? */

    strncpy(new->name, name, DEFAULT_WORD_NAME_MAX_LENGTH);
    new->flags = flags;
    new->next  = latest;
    latest = new;

    return new;
}

word_header_t* forth_dictionary_find_word(const char* name) {
    if(!name) return NULL;

    printf("finding word: '%s'\n", name);

    word_header_t* word = latest;
    while(word) {
        if (!(word->flags & FLAG_HIDDEN) && 
            !strncmp(word->name, name, DEFAULT_WORD_NAME_MAX_LENGTH)) {
            return word;
        }
        word = word->next;
    }

    return NULL;
}

/* todo: rename xt to cfa? */
/* todo: do we need this helper? */
void** forth_dictionary_get_cfa(word_header_t* word) { return (void**)(word + 1); }

void** forth_dictionary_get_cfa_by_name(const char* name) {
    word_header_t* word = forth_dictionary_find_word(name); /* todo: err_not_found? */
    return forth_dictionary_get_cfa(word);;
}

void* forth_dictionary_get_xt(word_header_t* word) {
    if(word->flags & FLAG_BUILTIN) return *forth_dictionary_get_cfa(word);
    else return forth_dictionary_get_cfa(word);
}

void* forth_dictionary_get_xt_by_name(const char* name) {
    word_header_t* word = forth_dictionary_find_word(name); /* todo: err_not_found? */
    return forth_dictionary_get_xt(word);
}

/* needs to get name by xt, taking into account builtin vs non-builtin */
// const char* forth_dictionary_get_name_by_cfa(void** cfa) {
//     if(!cfa) return NULL;
//     for(word_header_t* w = latest; w; w = w->next) {
//         if(forth_dictionary_get_cfa(w) == cfa) return w->name;
//     }
//     return NULL;
// }

void forth_dictionary_compile(cell value) {
    *(cell*)dictionary_pointer = value;
    dictionary_pointer += sizeof(cell);
}

void forth_dictionary_defcode(const char* name, void* code, cell flags) {
    forth_dictionary_create_word(name, flags | FLAG_BUILTIN);
    forth_dictionary_compile((cell)code);
}

void forth_dictionary_defword(const char* name, void** code, int codesize, cell flags) {
    forth_dictionary_create_word(name, flags);
    int i;
    for(i = 0; i < codesize; i++) forth_dictionary_compile((cell)code[i]);
    forth_dictionary_compile((cell)forth_dictionary_get_cfa_by_name("eow"));
}

void forth_dictionary_defconst(const char* name, cell value) {
    void* code[] = { 
        forth_dictionary_get_xt_by_name("lit"), 
        (void*)value, 
        forth_dictionary_get_xt_by_name("exit") 
    };
    forth_dictionary_defword(name, code, 3, FLAG_INLINE);
}

typedef float aliasingfloat __attribute__((__may_alias__));
void deffconst(const char* name, cell value) {
    void* code[] = { forth_dictionary_get_xt_by_name("flit"), (aliasingfloat*)value, forth_dictionary_get_xt_by_name("exit") }; 
    forth_dictionary_defword(name, code, 3, FLAG_INLINE);
}



/* debug stuff */
// /* todo: remake into find_by_xt? */
// word_header_t* forth_dictionary_find_word_by_cfa(void** cfa) {
//     if(!cfa) return NULL;
//     printf("finding cfa: '%p'\n", cfa);
//     word_header_t* word = latest;
//     while(word) {
//         if (!(word->flags & FLAG_HIDDEN) && ((void**)(word + 1) == cfa)) {
//             return word;
//         }
//         word = word->next;
//     }
//     return NULL;
// }
