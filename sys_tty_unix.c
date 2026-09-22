/* sys_tty_unix.c */
#if defined(__unix__) || defined(__APPLE__) /* todo: make into simplified macro? */

#include "sys_tty.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

static struct termios original_termios;
static int saved = 0;

void sys_hello(void) {
    printf("hello from sys...\n");
}

void sys_tty_enable_raw_mode(void) {
    if (!saved) {
        tcgetattr(STDIN_FILENO, &original_termios);
        saved = 1;
    }
    struct termios raw = original_termios;

    /* local, "line-discipline" flags */
    raw.c_lflag &= ~(
        ECHO   |    /* terminal echo: printing back out what you type */
        ICANON |    /* canonical mode: line-buffer, line-editing, etc */
        ISIG   |    /* signals: quit commands etc that default to os signal interrupts */
        IEXTEN      /* extended: additional, miscellanious (I think) input processing */
    );
    /* input-processing flags */
    raw.c_iflag &= ~(
        IXON   |    /* old-school software flow control stuff, apparently */
        ICRNL  |    /* translates /r (carriage return) to /n, etc */
        BRKINT |    /* some "break condition" signal interrupt thing */
        INPCK  |    /* parity checking   - legacy serial communication concern */
        ISTRIP      /* 8th bit stripping - legacy serial communication concern*/
    );
    /* output-processing flags */
    raw.c_oflag &= ~(
        OPOST       /* output post-processing flag: e.g. /n -> /r/n, etc */
    );
    raw.c_cflag |= CS8; /* sets character size to 8-bits per byte */
    raw.c_cc[VMIN]  = 1; /* block until at least one byte is available */
    raw.c_cc[VTIME] = 0; /* no timeout */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void sys_tty_disable_raw_mode(void) {
    if (saved) tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

int sys_tty_read_byte(void) {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;
    return (unsigned char)c;
}

void sys_tty_get_terminal_window_size(int* rows, int* cols) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    /* ws_xpixel */
    /* ws_ypixel */
}

void sys_tty_write(const char* buf, int len) {
    write(STDOUT_FILENO, buf, len);
}

void sys_tty_clear_screen(void) {
    sys_tty_write("\x1b[2J", 4);   /* clear entire screen */
    sys_tty_write("\x1b[H",  3);   /* move cursor to row 1, col 1 */
}

#endif