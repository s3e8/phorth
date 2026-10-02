/* sys_tty_win32.c */
#if defined(_WIN32)

#include "sys_tty.h"
#include <stdio.h>
#include <windows.h>

/* older mingw headers may lack these (windows 10+ console flags) */
#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT      0x0200
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static DWORD original_input_mode;
static DWORD original_output_mode;
static int   saved = 0;

static HANDLE  input_handle(void) { return GetStdHandle(STD_INPUT_HANDLE);  }
static HANDLE output_handle(void) { return GetStdHandle(STD_OUTPUT_HANDLE); }

static void save_modes(void) {
    if (saved) return;
    GetConsoleMode(input_handle(),  &original_input_mode);
    GetConsoleMode(output_handle(), &original_output_mode);
    saved = 1;
}

void sys_hello(void) {
    printf("hello from sys_tty_win32...\n");
}

void sys_tty_enable_raw_mode(void) {
    save_modes();
    DWORD input_mode = original_input_mode;
    input_mode &= ~(
        ENABLE_ECHO_INPUT      |   /* terminal echo */
        ENABLE_LINE_INPUT      |   /* line buffering (like ICANON) */
        ENABLE_PROCESSED_INPUT     /* ctrl-c etc handled by the system (like ISIG) */
    );
    input_mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;   /* arrow keys arrive as ESC [ A..D, same as unix */
    SetConsoleMode(input_handle(), input_mode);
    SetConsoleMode(output_handle(), original_output_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void sys_tty_disable_raw_mode(void) {
    if (!saved) return;
    SetConsoleMode(input_handle(),  original_input_mode);
    SetConsoleMode(output_handle(), original_output_mode);
}

int sys_tty_read_byte(void) {
    char  c;
    DWORD count;
    if (!ReadFile(input_handle(), &c, 1, &count, NULL) || count != 1) return -1;
    return (unsigned char)c;
}

void sys_tty_get_terminal_window_size(int* rows, int* cols) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(output_handle(), &info);
    *rows = info.srWindow.Bottom - info.srWindow.Top  + 1;
    *cols = info.srWindow.Right  - info.srWindow.Left + 1;
}

void sys_tty_write(const char* buf, int len) {
    DWORD written;
    WriteFile(output_handle(), buf, (DWORD)len, &written, NULL);
}

void sys_tty_clear_screen(void) {
    save_modes();   /* make sure ansi escapes are understood even outside raw mode */
    SetConsoleMode(output_handle(), original_output_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    sys_tty_write("\x1b[2J", 4);   /* clear entire screen */
    sys_tty_write("\x1b[H",  3);   /* move cursor to row 1, col 1 */
}

#endif