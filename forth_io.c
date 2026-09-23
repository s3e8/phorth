#include "forth.h"

#define DEFAULT_INPUT_STACK_SIZE 32
#define DEFAULT_WORD_BUFFER_SIZE 128
#define DEFAULT_LINE_BUFFER_SIZE 2048

typedef struct input_t {
    FILE* stream;
    char* buffer;
    int   buffer_position;
    /* todo: buffer size? */
} input_t;

static input_t* input_stack_pointer;
static input_t* input_stack_base;
static int     input_stack_size;
static char*  wordbuf; /* todo: rename to long-form words */
static int    wordbuf_size;

static FILE* current_input_stream;
static FILE* current_output_stream;
static char* current_line_buffer;
static char* current_line_buffer_position;
static int   current_line_buffer_size;

static input_t default_input_stack[DEFAULT_INPUT_STACK_SIZE];
static char    default_line_buffer[DEFAULT_LINE_BUFFER_SIZE];
static char    default_word_buffer[DEFAULT_WORD_BUFFER_SIZE];

void forth_vm_push_input_stack(FILE* stream) {
    *input_stack++ = current_input;
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

void forth_io_set_wordbuf(char* wordbuf, int size) {
    wordbuf      = wordbuf;
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

void forth_io_init_defaults(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    forth_io_set_input_stream(stdin);
    forth_io_set_output_stream(stdout);
    forth_io_set_wordbuf(default_word_buffer, sizeof(default_word_buffer));
    forth_io_set_linebuf(default_line_buffer, sizeof(default_line_buffer));
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
char* forth_io_get_next_line() {
    if(!current_line_buffer) {
        fprintf(stderr, "Error: current_line_buffer not set\n");
        return NULL;
    }
    if(current_input_stream == stdin) printf("outer> ");

    char* tmp = fgets(current_line_buffer, current_line_buffer_size, current_input_stream);
    if (!tmp && feof(current_input_stream)) {
        printf("End of file reached, switching to stdin...\n");
        current_input_stream = stdin;
        printf("outer> ");
        tmp = fgets(current_line_buffer, current_line_buffer_size, current_input_stream);
    }
    if(!tmp) return NULL;

    current_line_buffer_position = tmp;

    return tmp;
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



void forth_io_test_all(void) {
    forth_io_read_string("this is a test");
}
