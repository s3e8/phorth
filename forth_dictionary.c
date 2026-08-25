/* forth_dictionary.c */

#include "forth.h"

void* dictionary_pointer; /* todo: dp? */
void* dictionary_base;
cell  dictionary_size;

word_header_t* dictionary_latest = NULL;

word_header_t* forth_dictionary_find_word(const char *name) 
{
    if(!name) return NULL;

    word_header_t* hdr = dictionary_latest;
    while(hdr) {
        if(!(hdr->flags & FLAG_HIDDEN) && !strncmp(hdr->name, name, DEFAULT_WORD_NAME_MAX_LENGTH)) 
        return hdr;

        hdr = hdr->next;
    }

    return NULL;
}

word_header_t* forth_dictionary_create_word(const char* name, cell flags) 
{
    if(!name) name="\0";  /* for creating unnamed words */

    word_header_t* new = (word_header_t*)dictionary_pointer;
    dictionary_pointer += sizeof(word_header_t);
    strncpy(new->name, name, DEFAULT_WORD_NAME_MAX_LENGTH);
    
    new->flags = flags;
    new->next = dictionary_latest;
    dictionary_latest = new;

    return new;
}

void forth_dictionary_compile(cell value) {
    *(cell*)dictionary_pointer = value;
    dictionary_pointer += sizeof(cell);
}

void** forth_dictionary_get_cfa(word_header_t* word) {
  return (void**)(word+1);
}

void* get_builtin(const char* name) {
    word_header_t *hdr = forth_dictionary_find_word(name);
    return *(forth_dictionary_get_cfa(hdr));
  }

void assemble_word(const char *name, cell flags, void **code, cell codesize) {
    int i;
    forth_dictionary_create_word(name, flags);
    for(i=0; i<codesize/sizeof(void*); i++) {
      forth_dictionary_compile((cell)code[i]);
    }
    forth_dictionary_compile((cell)get_builtin("eow"));
  }
  
void create_constant(const char *name, cell value) {
    void *flagdef[] = { get_builtin("lit"), 0, get_builtin("exit") };
    flagdef[1] = (void*)value;
    assemble_word(name, FLAG_INLINE, flagdef, sizeof(flagdef));
  }
  
typedef float aliasingfloat __attribute__((__may_alias__));
void create_fconstant(const char *name, float value) {
    void *flagdef[] = { get_builtin("flit"), 0, get_builtin("exit") };
    *(aliasingfloat*)(&flagdef[1]) = value;
    assemble_word(name, FLAG_INLINE, flagdef, sizeof(flagdef));
  }
  
void create_builtin(builtin_word_t *b) {
    forth_dictionary_create_word(b->name, b->flags | FLAG_BUILTIN);
    forth_dictionary_compile((cell)b->code);
  }
  
  
  /// NEW DEFINING WORDS!!!!
  // get_builtin
void* getcode(const char* name)
  {
    word_header_t* word = forth_dictionary_find_word(name);
    return *(forth_dictionary_get_cfa(word));
  }
  
  // create_builtin
void forth_dictionary_defcode(const char* name, void* code, cell flags)
  {
    forth_dictionary_create_word(name, flags | FLAG_BUILTIN);
    forth_dictionary_compile((cell)code);
  }
  
  // assemble_word
void forth_dictionary_defword(const char* name, void* code[], int wordcount, cell flags)
  {
    forth_dictionary_create_word(name, flags); // todo: /sizeof(void*)?
    for(int i = 0; i < wordcount; i++) forth_dictionary_compile((cell)code[i]);
    forth_dictionary_compile((cell)getcode("eow"));
  }
  
  // create_constant
void forth_dictionary_defconst(const char* name, cell value)
  {
    void* flagdef[] = { getcode("lit"), (void*)value, getcode("exit") };
    forth_dictionary_defword(name, flagdef, 3, FLAG_INLINE);
  }
  
// create_fconstant
// typedef float aliasingfloat __attribute__((__may_alias__));
void forth_dictionary_deffconst(const char* name, float value) {
    // *(aliasingfloat*)(&flagdef[1]) = value; // todo: ??
    // void* flagdef[] = { getcode("flit"), (aliasingfloat*)value, getcode("exit") };
    void *flagdef[] = { get_builtin("flit"), 0, get_builtin("exit") };
    *(aliasingfloat*)(&flagdef[1]) = value;
    forth_dictionary_defword(name, flagdef, 3, FLAG_INLINE);
}
