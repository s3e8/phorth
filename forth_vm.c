
#include "forth.h"

/* todo:
    -- semantics: overflow vs underflow when downward growing stack..
    -- return interpret/vm_run function with 0 or 1 for err value..
    -- clean up execute opcodes with scheduling functions?
    -- thread names?
*/

// #define DEFAULT_MAX_THREADS             5 /* unnecessary cause threadstack will be linked list */
#define DEFAULT_RETURNSTACK_SIZE        512
#define DEFAULT_NESTINGSTACK_MAX_DEPTH  512
#define DEFAULT_DATASTACK_SIZE          1024
#define DEFAULT_TEMPSTACK_SIZE          1024
#define DEFAULT_FLOATSTACK_SIZE         1024

static int forth_initialized = 0;

static void**  current_ip; /* ip points to subroutines (array of xt/void*) */
static void*** current_rs; /* array of subroutines */
static void*** current_r0;
static cell*   current_ds; /* data stack */
static cell*   current_d0;
static cell*   current_ts; /* temp stack */
static cell*   current_t0;
static float*  current_fs; /* float stack */
static float*  current_f0;
static int     current_rs_size;
static int     current_ns_size;
static int     current_ds_size;
static int     current_ts_size;
static int     current_fs_size;

static void** default_returnstack[DEFAULT_RETURNSTACK_SIZE];
static void** default_nestingstack[DEFAULT_NESTINGSTACK_MAX_DEPTH];
static cell   default_datastack[DEFAULT_DATASTACK_SIZE];
static cell   default_tempstack[DEFAULT_TEMPSTACK_SIZE];
static float  default_floatstack[DEFAULT_FLOATSTACK_SIZE];

/* todo: rename this stuff to fit everything else */
void**  nestingstack_space[DEFAULT_NESTINGSTACK_MAX_DEPTH];
void*** nestingstack = nestingstack_space + DEFAULT_NESTINGSTACK_MAX_DEPTH;

void* builtin_immediatebuf[2];
void*    word_immediatebuf[3];

typedef struct forth_vm_s {
    void**      instruction_pointer;
    void***     returnstack;
    void***     returnstack_base;
    // void***     current_ns;
    // void***     nestingstack_base;
    cell*   datastack;
    cell*   datastack_base;
    cell*   tempstack;
    cell*   tempstack_base;
    float*  floatstack;
    float*  floatstack_base;

    // char*               thread_name;
    cell                killed;
    struct forth_vm_s*  next;
} forth_vm_t;

forth_vm_t* current_thread = NULL;
// char* current_thread_name  = NULL;

void forth_vm_print_state(void) {
    
}

forth_vm_t* forth_vm_init_thread(
    void**     ip, /* entrypoint */
    void***    r0, 
    // void***    n0, /* todo: do we need nesting stack here? */
    cell*  d0, 
    cell*  t0,
    float* f0
) {
    forth_vm_t* new = malloc(sizeof(forth_vm_t));
    new->killed = 0;
    new->instruction_pointer = ip;
    new->returnstack_base    = r0;
    // new->nestingstack_base   = n0;
    new->datastack_base      = d0;
    new->tempstack_base      = t0;
    new->floatstack_base     = f0;
    new->returnstack    = new->returnstack_base;
    // new->current_ns   = new->nestingstack_base;
    new->datastack      = new->datastack_base;
    new->tempstack      = new->tempstack_base;
    new->floatstack     = new->floatstack_base;

    if(!current_thread) {
        current_thread = new;
        current_ip = new->instruction_pointer;
        current_rs = new->returnstack;
        current_r0 = new->returnstack_base;
        // current_ns = new->current_ns;
        // nestingstack = new->nestingstack_base;
        current_ds = new->datastack;
        current_d0 = new->datastack_base;
        current_ts = new->tempstack;
        current_t0 = new->tempstack_base;
        current_fs = new->floatstack;
        current_f0 = new->floatstack_base;
        new->next = new; /* todo: ??? */
    } else {
        new->next = current_thread->next;
        current_thread->next = new;
    }
    /* todo: make new thread current? */
    // current_thread = new;
    
    return new;
}

forth_vm_t* forth_vm_create_thread(
    void**  entrypoint,
    int rs_size,
    // int ns_size,
    int ds_size,
    int ts_size,
    int fs_size
) {
    return forth_vm_init_thread(
        entrypoint,
        (void***)    malloc(rs_size * sizeof(void**)),
        // (void***)    malloc(ns_size * sizeof(void**)),
        (cell*)  malloc(ds_size * sizeof(cell)),
        (cell*)  malloc(ts_size * sizeof(cell)),
        (float*) malloc(fs_size * sizeof(float))
    );
}

forth_vm_t* forth_vm_kill_thread(forth_vm_t* thread) {
    return NULL;
}

void forth_vm_init_defaults(void) {
    current_rs_size = DEFAULT_RETURNSTACK_SIZE;
    // current_ns_size = DEFAULT_NESTINGSTACK_MAX_DEPTH;
    current_ds_size = DEFAULT_DATASTACK_SIZE;
    current_ts_size = DEFAULT_TEMPSTACK_SIZE;
    current_fs_size = DEFAULT_FLOATSTACK_SIZE;
    current_r0 = default_returnstack  + current_rs_size;
    // nestingstack = default_nestingstack + current_ns_size;
    current_d0 = default_datastack    + current_ds_size;
    current_t0 = default_tempstack    + current_ts_size;
    current_f0 = default_floatstack   + current_fs_size;
    current_rs = current_r0;
    current_ds = current_d0;
    current_ts = current_t0;
    current_fs = current_f0;
}

int check_stack_overflow(void) {
    if((current_ds - 1) < (current_d0 - current_ds_size)) {
        fprintf(stderr, "Data stack overflow\n");
        return 1;
    }
    return 0;
}

int check_stack_underflow(void) {
    if(current_ds >= current_d0) {
        fprintf(stderr, "Data stack underflow\n");
        return 1;
    }
    return 0;
}

void forth_vm_push_ds(cell value) {
    // printf("pushing '%d'...\n", (int)value);
    if(check_stack_overflow()) return;
    *--current_ds = value;
}

/* todo: change to int? */
cell forth_vm_pop_ds(void) {
    if(check_stack_underflow()) return 0;
    return *current_ds++;
}

void forth_vm_push_rs(void** code) {
    if(current_rs - 1 < current_r0 - current_rs_size) {
        fprintf(stderr, "Return stack underflow\n");
        return;
    }
    // printf("pushing '%p' to return stack...\n", code);
    // if(check_stack_overflow()) return;
    *--current_rs = code;
}

xt forth_vm_pop_rs(void) {
    if(current_rs >= current_r0) {
        fprintf(stderr, "Return stack underflow\n");
        return NULL;
    }
    return *current_rs++;
}

void forth_vm_push_ns(void) {
    // if(current_ns - 1 < nestingstack - current_ns_size) {
    //     fprintf(stderr, "Return stack underflow\n");
    //     return;
    // }
    // printf("pushing '%p' to return stack...\n", code);
    // printf("pushing '%p' to nesting stack...\n", current_ip);
    *--nestingstack = current_ip;
}

xt forth_vm_pop_ns(void) {
    // if(current_ns >= nestingstack) {
    //     fprintf(stderr, "Return stack underflow\n");
    //     return NULL;
    // }
    return *nestingstack++;
}

void forth_vm_schedule_builtin(void** code) {
    // printf("pushed current rs to nestingstack\n");
    printf("scheduling builtin...\n");
    builtin_immediatebuf[0] = *code;
    current_ip = builtin_immediatebuf;
    printf("done\n");
}

void forth_vm_schedule_word(void** code) {
    // printf("pushed current rs to nestingstack\n");
    word_immediatebuf[1] = (void*)code;
    current_ip = word_immediatebuf;
}

void breakpoint() {
    printf("this is a breakpoint\n");
    return;
}

void forth_vm_dbg_print_ds(void) {
    printf("<ds> ");
    for(cell* p = current_d0 - 1; p >= current_ds; p--)
        printf("%ld ", (long)*p);
    printf("\n");
}

void forth_vm_dbg_print_rs(void) {
    // printf("<rs> ");
    // for(xt* p = current_r0 - 1; p >= current_rs; p--) {
    //     const char* name = forth_dictionary_get_name_by_cfa(*p);
    //     printf(name ? "%s " : "%p ", name ? (void*)name : (void*)*p);
    // }
    // printf("\n");
}

/* execution engine -- todo: rename to run? */
int forth_vm_run() {
    register cell temp; /* this is an actual thing in figforth -- a register called temp. */

    /* todo: remove as globals???
        no cleaner way to do this if we want to name these as globals 
        (necessary in order to relocate "interpret" into the interpreter module,
         which may not be necessary)
    */
    builtin_immediatebuf[0] = NULL;
    builtin_immediatebuf[1] = CODE(IRETURN);
       word_immediatebuf[0] = CODE(CALL);
       word_immediatebuf[1] = NULL;
       word_immediatebuf[2] = CODE(IRETURN);

    // forth_io_read_string("this is a test");

    // return 0;

    if(!forth_initialized) {
        printf("initializing forth...\n");

        /* core -- inner interpreter */
        forth_dictionary_defcode("ireturn", CODE(IRETURN), 0);
        forth_dictionary_defcode("branch",  CODE(BRANCH),  0);
        forth_dictionary_defcode("call",    CODE(CALL),    0);
        forth_dictionary_defcode("lit",     CODE(LIT),     0);
        forth_dictionary_defcode("exit",    CODE(EXIT),    0);
        forth_dictionary_defcode("eow",     CODE(EOW),     0);
        /* interpreter */
        forth_dictionary_defconst("f_builtin",   FLAG_BUILTIN);
        forth_dictionary_defconst("f_hasarg",    FLAG_HASARG);
        forth_dictionary_defconst("f_immediate", FLAG_IMMEDIATE);
        forth_dictionary_defconst("f_hidden",    FLAG_HIDDEN);
        forth_dictionary_defconst("f_inline",    FLAG_INLINE);
        forth_dictionary_defconst("state",      (cell)&state);
        forth_dictionary_defconst("cellsize",   (cell)sizeof(cell));
        forth_dictionary_defconst("floatsize",  (cell)sizeof(float));
        forth_dictionary_defcode("bye",     CODE(BYE), 0);
        forth_dictionary_defcode("[", CODE(LEFT_BRACKET),   FLAG_IMMEDIATE );
        forth_dictionary_defcode("]", CODE(RIGHT_BRACKET),  FLAG_IMMEDIATE );
        forth_dictionary_defcode(":", CODE(COLON),          0);
        forth_dictionary_defcode(";", CODE(SEMICOLON),      FLAG_IMMEDIATE );
        /* vm */
        forth_dictionary_defconst("s0", (cell)&current_d0); /* todo: change to current_s0? */
        forth_dictionary_defconst("r0", (cell)&current_r0);
        forth_dictionary_defconst("f0", (cell)&current_f0);
        forth_dictionary_defconst("t0", (cell)&current_t0);
        forth_dictionary_defcode("die",     CODE(DIE),          0);
        forth_dictionary_defcode("0branch", CODE(ZERO_BRANCH),  FLAG_HASARG  ); /* todo: these are  definitely interpreter opcodes */
        forth_dictionary_defcode("1branch", CODE(IF_BRANCH),    FLAG_HASARG  );
        forth_dictionary_defcode("jump",    CODE(JUMP),         FLAG_HASARG  );
        forth_dictionary_defcode("+",       CODE(ADD),          0);
        forth_dictionary_defcode("dup",     CODE(DUP),          0);
        forth_dictionary_defcode("swap",    CODE(SWAP),         0);
        forth_dictionary_defcode("xor",     CODE(XOR),          0);
        forth_dictionary_defcode("and",     CODE(AND),          0);
        forth_dictionary_defcode("1-",      CODE(SUB1),         0);
        forth_dictionary_defcode("1+",      CODE(ADD1),         0);
        forth_dictionary_defcode("invert",  CODE(INVERT),       0);
        /* dictionary */
        forth_dictionary_defconst("here",    (cell)&dictionary_pointer);
        forth_dictionary_defcode("latest",    CODE(LATEST),       0);
        forth_dictionary_defcode("create",    CODE(CREATE),       0);
        forth_dictionary_defcode("word",      CODE(WORD),         0);
        forth_dictionary_defcode("find",      CODE(FIND),         0);
        forth_dictionary_defcode(",",         CODE(COMMA),        0);
        forth_dictionary_defcode("'",         CODE(TICK),         FLAG_IMMEDIATE);
        forth_dictionary_defcode("immediate", CODE(IMMEDIATE),    FLAG_IMMEDIATE);
        forth_dictionary_defcode("hidden",    CODE(HIDDEN),       0);
        /* io */
        forth_dictionary_defcode("emit",    CODE(EMIT),     0);
        forth_dictionary_defcode("tell",    CODE(TELL),     0);
        forth_dictionary_defcode(".",       CODE(DOT),      0);
        /* other */
        forth_dictionary_defcode("@",       CODE(FETCH),    0);
        forth_dictionary_defcode("!",       CODE(STORE),    0);
        forth_dictionary_defcode("c!",      CODE(CSTORE),   0);
        forth_dictionary_defcode("+!",      CODE(MEMADD),   0);
        /* end defcodes */

        /* convenience codes -- kind of a hack tbh */
        // call_code = forth_dictionary_get_xt_by_name("call");
        // lit_code  = forth_dictionary_get_xt_by_name("lit");

        printf("initialized.\n");
        forth_initialized = 1;
    }

    void* quitcode[] = { 
        CODE(INTERPRET), 
        CODE(BRANCH), 
        OFFSET(-2),
        CODE(EOW)
    };
    current_ip = quitcode;

    printf("starting forth...\n");

    NEXT();

    /* we return here to avoid fallthrough */
    fprintf(stderr, "Err: forth_vm_run function somehow fell through...\n");
    return 1;

    /* labels */
    OP(DIE): DIE();
    OP(BYE): BYE();

    /* forth core ops */
    OP(INTERPRET): {
        INTERPRET();
        NEXT();
    }

    // OP(INTERPRET): {
    //     forth_interpreter_interpret();
    //     NEXT();
    // }

    OP(BRANCH): {
        BRANCH();
        NEXT();
    }   

    OP(IRETURN): {
        IRETURN();
        NEXT();
    }

    OP(CALL): {
        CALL();
        NEXT();
    }

    OP(LIT): {
        LIT();
        NEXT();
    }

    OP(EOW): { /* end of word marker -- do nothing */
        EOW();
        NEXT(); 
    }

    /* forth interpreter words */
    OP(LEFT_BRACKET): {
        LEFT_BRACKET();
        NEXT();
    }

    OP(RIGHT_BRACKET): {
        RIGHT_BRACKET();
        NEXT();
    }

    OP(COLON): {
        COLON();
        NEXT();
    }

    OP(SEMICOLON): {
        forth_dictionary_compile((cell)CODE(EXIT));
        latest->flags &= ~FLAG_HIDDEN;
        state = STATE_IMMEDIATE;
        NEXT();
    }

    /* forth vm words */
    OP(NOOP): {
        NEXT();
    }

    OP(EXIT): {
        EXIT();
        NEXT();
    }

    OP(ZERO_BRANCH): { /* todo: FLAG_HASARG */
        ZERO_BRANCH();
        NEXT();
    }

    OP(IF_BRANCH): { /* todo: FLAG_HASARG */
        IF_BRANCH(); /* todo: rename to BRANCH_IF_TRUE??? */
        NEXT();
    }

    OP(JUMP): {
        JUMP();
        NEXT();
    }

    /* forth dictionary ops */
    OP(CREATE): {
        CREATE();
        NEXT();
    }

    OP(WORD): { /* todo: check ans definitions of word and create... */
        WORD();
        NEXT();
    }

    OP(FIND): {
        FIND();
        NEXT();
    }

    OP(HIDDEN): {
        HIDDEN();
        NEXT();
    }

    OP(TICK): {
        TICK();
        NEXT();
    }

    OP(COMMA): {
        COMMA();
        NEXT();
    }

    OP(FETCH): { /* todo: a little confused about the pointer semantics here, apparently */
        FETCH();   
        NEXT();
    }

    OP(STORE): {
        STORE();
        NEXT();
    }

    OP(CSTORE): {
        CSTORE();
        NEXT();
    }

    OP(LATEST): {
        LATEST();
        NEXT();
    }

    OP(IMMEDIATE): {
        IMMEDIATE();
        NEXT();
    }

    OP(ADD): {
        ADD();
        NEXT();
    }

    OP(SUB1): {
        SUB1();
        NEXT();
    }

    OP(ADD1): {
        ADD1();
        NEXT();
    }

    OP(MEMADD): {
        MEMADD();
        NEXT();
    }
    
    OP(INVERT): {
        INVERT();
        NEXT();
    }

    OP(SWAP): {
        SWAP();
        NEXT();
    }

    OP(AND): {
        AND();
        NEXT();
    }

    /* forth io ops */
    OP(EMIT): {
        EMIT();
        NEXT();
    }

    OP(TELL): {
        TELL();
        NEXT();
    }

    OP(DOT): {
        DOT();
        NEXT();
    }

    OP(DUP): {
        DUP();
        NEXT();
    }

    OP(XOR): {
        XOR();
        NEXT();
    }
}

void forth_vm_test(void) {
    printf("Forth VM module compiled successfully\n");

    return;
}

void forth_vm_dbg_print_stack() {

}
