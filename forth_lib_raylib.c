/* forth_lib_raylib.c */
#include "forth.h"

#ifdef INCLUDE_LIB_RAYLIB

#include <raylib.h>

static Color pop_color(void) { /* color is one cell: 0xRRGGBBAA */
    cell c = forth_vm_pop_ds();
    return (Color){ (c >> 24) & 255, (c >> 16) & 255, (c >> 8) & 255, c & 255 };
}

static void forth_lib_raylib_init_window(void) { /* ( w h title -- ) */
    const char* title = (const char*)forth_vm_pop_ds();
    int h = (int)forth_vm_pop_ds();
    int w = (int)forth_vm_pop_ds();
    InitWindow(w, h, title);
}

static void forth_lib_raylib_close_window(void)        { CloseWindow(); }                                       /* ( -- ) */
static void forth_lib_raylib_window_should_close(void) { forth_vm_push_ds(WindowShouldClose()); }               /* ( -- flag ) */
static void forth_lib_raylib_set_target_fps(void)      { SetTargetFPS((int)forth_vm_pop_ds()); }                /* ( fps -- ) */
static void forth_lib_raylib_begin_drawing(void)       { BeginDrawing(); }                                      /* ( -- ) */
static void forth_lib_raylib_end_drawing(void)         { EndDrawing(); }                                        /* ( -- ) */
static void forth_lib_raylib_clear_background(void)    { ClearBackground(pop_color()); }                        /* ( color -- ) */
static void forth_lib_raylib_is_key_down(void)         { forth_vm_push_ds(IsKeyDown((int)forth_vm_pop_ds())); } /* ( key -- flag ) */

static void forth_lib_raylib_draw_text(void) {
    Color color = pop_color();
    int font_size = (int)forth_vm_pop_ds(); /* todo: is this font size? */ 
    int h = (int)forth_vm_pop_ds();
    int w = (int)forth_vm_pop_ds();
    int y = (int)forth_vm_pop_ds();
    const char* text = (const char*)forth_vm_pop_ds();
    DrawText(text, w, h, font_size, color);
}

static void forth_lib_raylib_draw_rectangle(void) { /* ( x y w h color -- ) */
    Color color = pop_color();
    int h = (int)forth_vm_pop_ds();
    int w = (int)forth_vm_pop_ds();
    int y = (int)forth_vm_pop_ds();
    int x = (int)forth_vm_pop_ds();
    DrawRectangle(x, y, w, h, color);
}

void forth_lib_raylib_draw_circle(void) {
    Color color = pop_color();
    int radius  = forth_vm_pop_ds();
    int y       = forth_vm_pop_ds();
    int x       = forth_vm_pop_ds();
    DrawCircle(x, y, radius, color);
}

void forth_include_lib_raylib(void) {
    /* init game loop stuff */ /* todo: rename to rl-func */
    forth_dictionary_defextern("init-window",         forth_lib_raylib_init_window, 0);
    forth_dictionary_defextern("close-window",        forth_lib_raylib_close_window, 0);
    forth_dictionary_defextern("window-should-close", forth_lib_raylib_window_should_close, 0);
    forth_dictionary_defextern("set-target-fps",      forth_lib_raylib_set_target_fps, 0);
    forth_dictionary_defextern("begin-drawing",       forth_lib_raylib_begin_drawing, 0);
    forth_dictionary_defextern("end-drawing",         forth_lib_raylib_end_drawing, 0);
    forth_dictionary_defextern("clear-background",    forth_lib_raylib_clear_background, 0);
    /* controls */
    forth_dictionary_defextern("is-key-down",         forth_lib_raylib_is_key_down, 0);
    /* render */
    forth_dictionary_defextern("draw-text",           forth_lib_raylib_draw_text, 0);
    forth_dictionary_defextern("draw-rectangle",      forth_lib_raylib_draw_rectangle, 0);
    forth_dictionary_defextern("draw-circle",         forth_lib_raylib_draw_circle, 0);
}

#endif