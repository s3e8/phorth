#include "forth.h"

#define DEFAULt_WORDBUF_SIZE 128
#define DEFAULT_LINEBUF_SIZE 2048

static FILE* current_input_stream;
static FILE* current_output_stream;
static char* current_wordbuf;
static char* current_linebuf;
static char* current_linebuf_position;
static int   current_wordbuf_size;
static int   current_linebuf_size;

static char default_linebuf[DEFAULT_LINEBUF_SIZE];
static char default_wordbuf[DEFAULt_WORDBUF_SIZE];

void forth_io_set_input_stream(FILE* input_stream) {
    /* todo: err if input_stream isnt file */
    current_input_stream = input_stream;
}

void forth_io_set_output_stream(FILE* output_stream) {
    current_output_stream = output_stream;
}

FILE* forth_io_open_or_create_file(const char* filename, const char* mode)
{
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
    current_wordbuf      = wordbuf;
    current_wordbuf_size = size;
}

void forth_io_set_linebuf(char* linebuf, int size) {
    current_linebuf          = linebuf;
    current_linebuf_position = current_linebuf;
    current_linebuf_size     = size;
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
    forth_io_set_wordbuf(default_wordbuf, sizeof(default_wordbuf));
    forth_io_set_linebuf(default_linebuf, sizeof(default_linebuf));
}

/* io debug */
void forth_io_print_state() {
    printf("current_wordbuf: %s\n",     current_wordbuf);
    printf("current_linebuf: %s\n",     current_linebuf);
    printf("current_linebuf_pos: %s\n", current_linebuf_position);
}

/* input stuff */
char* forth_io_get_next_line() {
    if(!current_linebuf) {
        fprintf(stderr, "Error: current_linebuf not set\n");
        return NULL;
    }
    if(current_input_stream == stdin) printf("> ");

    char* tmp = fgets(current_linebuf, current_linebuf_size, current_input_stream);
    if (!tmp && feof(current_input_stream)) {
        printf("End of file reached, switching to stdin...\n");
        current_input_stream = stdin;
        printf("> ");
        tmp = fgets(current_linebuf, current_linebuf_size, current_input_stream);
    }
    if(!tmp) return NULL;

    current_linebuf_position = tmp;

    return tmp;
}

/* Parse next word from a string, updating a position pointer */
char* forth_io_get_next_word()
{
    char*  tmp       = current_wordbuf;
    char*  position  = current_linebuf_position;
    int    size      = current_wordbuf_size;
    size_t count     = 0;

    printf("getting next word...\n");

    /* Skip whitespace */
    skip_whitespace:
        while(*position && isspace(*position)) position++;

    /* if line is empty, check for new line */
    if(*position == '\0') {
        if(!forth_io_get_next_line()) return NULL;
        position = current_linebuf_position;
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

    current_linebuf_position = position;

    printf("word retrieved.\n");

    return current_wordbuf;
}

int forth_io_get_char() {
    return fgetc(current_input_stream);
}

void forth_io_read_string(const char* str) {
    const char* position = str;
    char* tempbuf = current_linebuf;

    int i;
    /* fill linebuf */
    /* todo: I dont think it needs to be - 1 */
    for (i = 0; i < current_linebuf_size - 1 && *position; i++) {
        *tempbuf++ = *position++;
    }
    *tempbuf = '\0'; 
    current_linebuf_position = current_linebuf;

    while(*current_linebuf_position) {
        forth_io_get_next_word();
        printf("Got word: '%s'\n", current_wordbuf);
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


/* other ops */
void forth_io_skip_line(void) {
    while(*current_linebuf_position) current_linebuf_position++;
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
