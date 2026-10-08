#include "forth.h"

#define DEFAULT_INPUT_STACK_SIZE 32 /* aka MAX_DEPTH */
#define DEFAULT_WORD_BUFFER_SIZE 128 /* todo: rename to config_word_Buffer_size */ /* ( todo: remove "default" from name) */
#define DEFAULT_INPUT_BUFFER_SIZE 2048

typedef struct input_t { /* todo: separate if we bootstrap from forth instead? */
    FILE* stream;        /* ---   though this would mean exposing the globals */
    char* buffer;
    char* position;
    /* buffer size? */
} input_t;

static char          wordbuf[DEFAULT_WORD_BUFFER_SIZE]; /* todo: rename to long-form words */
static input_t       input_stack[DEFAULT_INPUT_STACK_SIZE];
static char          input_buffers[DEFAULT_INPUT_STACK_SIZE][DEFAULT_INPUT_BUFFER_SIZE];
static input_t*      input_stack_pointer = input_stack; /* todo: should this grow up or down? */
static char* current_input_buffer; /* todo: rename to input_buffer*/
static char* current_input_buffer_position;
static int   current_input_buffer_size; /*todo: do we need? */
static FILE* current_input_stream;
static FILE* current_output_stream;

void forth_io_define_constants(void) {
    forth_dictionary_defconst("<stdin>",               (cell)stdin); /* todo: rm brackets? */
    forth_dictionary_defconst("<stdout>",              (cell)stdout);
    forth_dictionary_defconst("<stderr>",              (cell)stderr);

    forth_dictionary_defconst("input-stream",          (cell)&current_input_stream);
    forth_dictionary_defconst("output-stream",         (cell)&current_output_stream);

    forth_dictionary_defconst("input-stack",           (cell)input_stack); /* whats the..? */
    forth_dictionary_defconst("input-buffers",         (cell)input_buffers);
    forth_dictionary_defconst("input-stack-max-depth", (cell)DEFAULT_INPUT_STACK_SIZE);
    forth_dictionary_defconst("input-stack-pointer",   (cell)&input_stack_pointer);
    forth_dictionary_defconst("input-buffer",          (cell)&current_input_buffer); 
    forth_dictionary_defconst("input-buffer-pos",      (cell)&current_input_buffer_position);
    forth_dictionary_defconst("input-buffer-size",     (cell)DEFAULT_INPUT_BUFFER_SIZE); /* todo: macro or var */
}

void forth_io_set_input_stream(FILE* input_stream) {
    /* todo: err if input_stream isnt file? */
    current_input_stream = input_stream;
}

void forth_io_set_output_stream(FILE* output_stream) {
    current_output_stream = output_stream;
}

/* todo: i dont think we need this.. */
void forth_io_set_input_buffers(char* linebuf, int size) {
    current_input_buffer          = linebuf;
    current_input_buffer_position = current_input_buffer;
    current_input_buffer_size     = size;
}

/* todo: do these need to be static? */
FILE* forth_io_open_file(const char* filename, const char* mode) {
    FILE* fp = fopen(filename, mode);
    if(!fp) {
        fprintf(stderr, "Unable to open file: %s\n", filename);
        return NULL;
    }
    return fp;
}


// static void prim_read_file(void) {
//     FILE*  fp   = (FILE*)forth_vm_pop_ds();
//     cell   u    = forth_vm_pop_ds();
//     char*  addr = (char*)forth_vm_pop_ds();
//     size_t n    = fread(addr, 1, (size_t)u, fp);
//     forth_vm_push_ds((cell)n);
// }

/* todo: cant include from a string.. */
void forth_io_set_input_string(const char* input) {
    if(strlen(input) >= current_input_buffer_size) { /* todo: > or >= */
        printf("Error: Input string must be shorter than linebuf.\n");
        return;
    }

    const char* position = input;
    int i;

    for (i = 0; i < current_input_buffer_size - 1 && *position; i++) {
        current_input_buffer[i] = *position++;
    }
    current_input_buffer[i] = '\0';
    current_input_buffer_position = current_input_buffer;
}


int forth_io_is_eof(void) {
    return (*current_input_buffer_position == '\0') && feof(current_input_stream);
}

int forth_io_is_eol(void) {
    while(*current_input_buffer_position && isspace(*current_input_buffer_position))
        current_input_buffer_position++;
    return *current_input_buffer_position == '\0';
}

/* todo: rename without namespace to indicate local helper not included in api? 
    read the next line of the current stream into the current buffer.
    returns NULL at EOF (buffer left empty). nesting is handled in forth. */
char* forth_io_get_next_line(void) {
    char* line = fgets(current_input_buffer, current_input_buffer_size, current_input_stream);
    if(!line) current_input_buffer[0] = '\0';
    current_input_buffer_position = current_input_buffer;
    return line;
}

/* Parse next word from a string, updating a position pointer */
char* forth_io_get_next_word() {
    char*  tmp       = wordbuf;
    char*  position  = current_input_buffer_position;
    int    size      = sizeof(wordbuf);
    size_t count     = 0;

    // printf("getting next word...\n");

    /* Skip whitespace */
    skip_whitespace:
        while(*position && isspace(*position)) position++;

    /* If line is empty, check for new line */
    if(*position == '\0') {
        if(!forth_io_get_next_line()) return NULL;
        position = current_input_buffer_position;
        goto skip_whitespace;
    }

    /* Copy word */
    while(*position && !isspace(*position) && count < size - 1) {
        *tmp++ = *position++;
        count++;
    }
    *tmp = '\0';

    if(*position) position++; 
    current_input_buffer_position = position;

    // printf("word retrieved.\n");

    return wordbuf;
}

int forth_io_get_char() {
    return fgetc(current_input_stream);
}

int forth_io_refill(void) { 
    return forth_io_get_next_line() != NULL; 
}
int forth_io_input_is_stdin(void) { 
    return current_input_stream == stdin; 
}

void forth_io_read_string(const char* str) {
    forth_io_set_input_string(str);

    while(*current_input_buffer_position) {
        forth_io_get_next_word();
        printf("Got word: '%s'\n", wordbuf);
    }
}

/* ops */
void forth_io_emit(int ch) {
    fputc(ch, current_output_stream);
}

void forth_io_tell(const char* str) {
    fputs(str, current_output_stream);
}

void forth_io_dot(cell value) {
    printf("%ld ", (long)value);
}

/* todo: without the (unsigned char) cast, a byte >=127 sign-extends to a negative int 
   and could collide with the -1 EOF sentinel.. fine for ASCII bootstrap text? 
   revisit if non-ASCII input matters later 
*/
int forth_io_get_next_char(void) { /* todo: read_key vs get_char? */
    if(*current_input_buffer_position == '\0') {
        if(!forth_io_get_next_line()) return -1;
    }
    return *current_input_buffer_position++;
}

/* other ops */
void forth_io_skip_line(void) {
    while(*current_input_buffer_position) current_input_buffer_position++;
}

void forth_io_skip_parens(void) {
    char* word;
    while ((word = forth_io_get_next_word())) {
        size_t len = strlen(word);
        if (len > 0 && word[len - 1] == ')') return;
    }
    fprintf(stderr, "Error: unterminated comment\n");
}

/* todo: does this belong here? */
const char* forth_io_format(const char* format_string) {
    static char outbuf[256];
    char        conversion_spec[16];                 /* one "%...X" piece, e.g. "%5d" or "%.2f" */
    size_t      outlen = 0;
    const char* cursor        = format_string;

    while(*cursor && outlen < sizeof(outbuf) - 1) {
        /* plain character: copy through */
        if(*cursor != '%') {
            outbuf[outlen++] = *cursor++;
            continue;
        }

        /* find the end of this conversion spec (skip flags/width/precision) */
        const char* spec_start = cursor++;
        while(*cursor && !strchr("dsef%", *cursor)) cursor++;
        if(!*cursor) break;                          /* unterminated spec */

        size_t spec_length = cursor - spec_start + 1;
        if(spec_length >= sizeof(conversion_spec)) break;
        memcpy(conversion_spec, spec_start, spec_length);
        conversion_spec[spec_length] = '\0';

        /* format one argument, popped from the matching stack */
        char   conversion_type = *cursor++;
        char*  write_pos       = outbuf + outlen;
        size_t space_left      = sizeof(outbuf) - outlen;

        switch(conversion_type) {
            case '%': outbuf[outlen++] = '%'; break;
            case 'd': outlen += snprintf(write_pos, space_left, conversion_spec, (int)forth_vm_pop_ds());    break;
            case 's': outlen += snprintf(write_pos, space_left, conversion_spec, (char*)forth_vm_pop_ds());  break;
            case 'e':
            case 'f': outlen += snprintf(write_pos, space_left, conversion_spec, (double)forth_vm_pop_fs()); break;
        }

        /* snprintf returns the untruncated length; clamp */
        if(outlen >= sizeof(outbuf)) outlen = sizeof(outbuf) - 1;
    }

    outbuf[outlen] = '\0';
    return outbuf;
}

void forth_io_init_defaults(void) {
    setvbuf(stdout, NULL, _IONBF, 0); /* todo: should I still be doing this? */
    setvbuf(stderr, NULL, _IONBF, 0);
    input_stack_pointer = input_stack;
    input_buffers[0][0] = '\0';
    forth_io_set_input_stream(stdin);
    forth_io_set_output_stream(stdout);
    // forth_io_set_wordbuf(wordbuf, wordbuf_size);
    forth_io_set_input_buffers(input_buffers[0], DEFAULT_INPUT_BUFFER_SIZE); /* todo: use var or macro */
}

void forth_io_print_current_word(void) {
    printf("word buffer: %s\n", wordbuf);
}

void forth_io_print_state(void) {
    printf("          word buffer: %s\n", wordbuf);
    printf("  current line buffer: %s\n", current_input_buffer);
    printf("current line position: %s\n", current_input_buffer_position);
}