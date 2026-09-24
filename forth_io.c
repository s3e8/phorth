#include "forth.h"

#define DEFAULT_INPUT_STACK_SIZE 32
#define DEFAULT_WORD_BUFFER_SIZE 128
#define DEFAULT_LINE_BUFFER_SIZE 2048

typedef struct input_t { /* todo: separate if we bootstrap from forth instead? */
    FILE* stream;        /* ---   though this would mean exposing the globals */
    char* buffer;
    char* position;
    /* buffer size? */
} input_t;

static input_t* input_stack_pointer;
static input_t* input_stack_base;
static int      input_stack_size;
static char*  wordbuf; /* todo: rename to long-form words */
static int    wordbuf_size;

static FILE* current_input_stream;
static FILE* current_output_stream;
static char* current_line_buffer;
static char* current_line_buffer_position;
static int   current_line_buffer_size;

static input_t default_input_stack[DEFAULT_INPUT_STACK_SIZE];
static char    default_word_buffer[DEFAULT_WORD_BUFFER_SIZE];
static char    default_line_buffers[DEFAULT_INPUT_STACK_SIZE][DEFAULT_LINE_BUFFER_SIZE];

void forth_io_push_input_stack(FILE* stream) {
    if(input_stack_pointer - input_stack_base >= input_stack_size - 1) {
        fprintf(stderr, "Input stack overflow\n");
        fclose(stream);
        return;
    }
    *input_stack_pointer++ = (input_t){ current_input_stream, current_line_buffer, current_line_buffer_position };

    current_input_stream         = stream;
    current_line_buffer          = default_line_buffers[input_stack_pointer - input_stack_base];
    current_line_buffer[0]       = '\0';
    current_line_buffer_position = current_line_buffer;
}

int forth_io_pop_input_stack(void) {
    if(input_stack_pointer <= input_stack_base) return 0;
    fclose(current_input_stream);

    input_t input = *--input_stack_pointer;
    current_input_stream         = input.stream;
    current_line_buffer          = input.buffer;
    current_line_buffer_position = input.position;
    return 1;
}

void forth_io_set_input_stream(FILE* input_stream) {
    /* todo: err if input_stream isnt file? */
    current_input_stream = input_stream;
}

void forth_io_set_output_stream(FILE* output_stream) {
    current_output_stream = output_stream;
}

FILE* forth_io_open_or_create_file(const char* filename, const char* mode) {
    FILE* fp = fopen(filename, mode);
    if(!fp) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }
    setvbuf(fp, NULL, _IONBF, 0);  // disable input buffering, we have our own

    return fp;
}

void forth_io_set_input_file(const char* filename) {
    FILE* fp = forth_io_open_or_create_file(filename, "r");
    if(!fp) return; /* todo */

    current_input_stream = fp;
}

void forth_io_set_wordbuf(char* buf, int size) {
    wordbuf      = buf;
    wordbuf_size = size;
}

void forth_io_set_linebuf(char* linebuf, int size) {
    current_line_buffer          = linebuf;
    current_line_buffer_position = current_line_buffer;
    current_line_buffer_size     = size;
}

/* todo: don't need both of these.. can set i/o in forth */
void forth_io_open_input_file(const char* filename, const char* mode) {
    FILE* f = fopen(filename, mode);
    if(!f) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return;
    }
    current_input_stream = f;
}

void forth_io_open_output_file(const char* filename, const char* mode) {
    FILE* f = fopen(filename, mode);
    if(!f) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return;
    }
    current_output_stream = f;
}

void forth_io_close_input() {
    if(current_input_stream && current_input_stream != stdin) {
        fclose(current_input_stream);
        current_input_stream = NULL;
    }
}

void forth_io_close_all(void) {}

int forth_io_include_file(const char* filename) {
    FILE* fp = forth_io_open_or_create_file(filename, "r");
    if(!fp) return 0;
    forth_io_push_input_stack(fp);
    return 1;
}

void forth_io_init_defaults(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    input_stack_size    = DEFAULT_INPUT_STACK_SIZE;
    input_stack_base    = default_input_stack;
    input_stack_pointer = input_stack_base;
    default_line_buffers[0][0] = '\0';
    forth_io_set_input_stream(stdin);
    forth_io_set_output_stream(stdout);
    forth_io_set_wordbuf(default_word_buffer, sizeof(default_word_buffer));
    forth_io_set_linebuf(default_line_buffers[0], DEFAULT_LINE_BUFFER_SIZE); /* todo: use var or macro */
}

/* io debug */
void forth_io_print_current_word(void) {
    printf("wordbuf: %s\n", wordbuf);
}

char* forth_io_get_current_wordbuf(void) {
    return wordbuf;
}

void forth_io_print_state(void) {
    forth_io_print_current_word();
    printf("current_linebuf: %s\n",     current_line_buffer);
    printf("current_linebuf_pos: %s\n", current_line_buffer_position);
}

int forth_io_is_eof(void) {
    return (*current_line_buffer_position == '\0') && feof(current_input_stream);
}

/* input stuff */
// char* forth_io_get_next_line() {
//     if(!current_line_buffer) {
//         fprintf(stderr, "Error: current_line_buffer not set\n");
//         return NULL;
//     }
//     if(current_input_stream == stdin) printf("outer> ");

//     char* tmp = fgets(current_line_buffer, current_line_buffer_size, current_input_stream);
//     if (!tmp && feof(current_input_stream)) {
//         printf("End of file reached, switching to stdin...\n");
//         current_input_stream = stdin;
//         printf("outer> ");
//         tmp = fgets(current_line_buffer, current_line_buffer_size, current_input_stream);
//     }
//     if(!tmp) return NULL;

//     current_line_buffer_position = tmp;

//     return tmp;
// }

char* forth_io_get_next_line(void) {
    if(!current_line_buffer) {
        fprintf(stderr, "Error: current_line_buffer not set\n");
        return NULL;
    }
    for(;;) {
        if(current_input_stream == stdin) printf("outer> ");
        char* tmp = fgets(current_line_buffer, current_line_buffer_size, current_input_stream);
        if(tmp) {
            current_line_buffer_position = tmp;
            return tmp;
        }
        if(!forth_io_pop_input_stack()) return NULL;   /* EOF at level 0 */
        if(*current_line_buffer_position)              /* parent still has words on its line */
            return current_line_buffer_position;
    }
}

/* Parse next word from a string, updating a position pointer */
char* forth_io_get_next_word()
{
    char*  tmp       = wordbuf;
    char*  position  = current_line_buffer_position;
    int    size      = wordbuf_size;
    size_t count     = 0;

    // printf("getting next word...\n");

    /* Skip whitespace */
    skip_whitespace:
        while(*position && isspace(*position)) position++;

    /* if line is empty, check for new line */
    if(*position == '\0') {
        if(!forth_io_get_next_line()) return NULL;
        position = current_line_buffer_position;
        // printf("tmp: %s\n", tmp);
        // printf("pos: %s\n", pos);
        goto skip_whitespace;
    }

    /* Copy word */
    while(*position && !isspace(*position) && count < size - 1) {
        *tmp++ = *position++;
        count++;
    }
    *tmp = '\0';

    if(*position) position++; 
    current_line_buffer_position = position;

    // printf("word retrieved.\n");

    return wordbuf;
}

int forth_io_get_char() {
    return fgetc(current_input_stream);
}

void forth_io_set_string_input(const char* input) {
        if(strlen(input) >= current_line_buffer_size) { /* todo: > or >= */
        printf("Error: Input string must be shorter than linebuf.\n");
        return;
    }

    const char* position = input;
    int i;

    for (i = 0; i < current_line_buffer_size - 1 && *position; i++) {
        current_line_buffer[i] = *position++;
    }
    current_line_buffer[i] = '\0';
    current_line_buffer_position = current_line_buffer;
}

void forth_io_read_string(const char* str) {
    forth_io_set_string_input(str);

    while(*current_line_buffer_position) {
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
   and could collide with the -1 EOF sentinel — fine for ASCII bootstrap text, 
   revisit if non-ASCII input matters later 
*/
int forth_io_get_next_char(void) { /* todo: read_key vs get_char? */
    if(*current_line_buffer_position == '\0') {
        if(!forth_io_get_next_line()) return -1;
    }
    return *current_line_buffer_position++;
}


/* other ops */
void forth_io_skip_line(void) {
    while(*current_line_buffer_position) current_line_buffer_position++;
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

void forth_io_test_all(void) {
    forth_io_read_string("this is a test");
}
