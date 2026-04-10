#ifndef ANSI80_SCREEN_H
#define ANSI80_SCREEN_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

typedef struct {
    unsigned char ch;
    unsigned char fg;
    unsigned char bg;
    unsigned short attr;
} Cell;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    int cell_width;
    int cell_height;
    Cell cells[SCREEN_HEIGHT][SCREEN_WIDTH];
} Screen;

enum {
    COLOR_BLACK = 0,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_BROWN,
    COLOR_LIGHTGRAY,
    COLOR_DARKGRAY,
    COLOR_LIGHTBLUE,
    COLOR_LIGHTGREEN,
    COLOR_LIGHTCYAN,
    COLOR_LIGHTRED,
    COLOR_LIGHTMAGENTA,
    COLOR_YELLOW,
    COLOR_WHITE
};

int screen_init(Screen *screen, const char *title, const char *font_path, int font_ptsize);
void screen_shutdown(Screen *screen);
void screen_clear(Screen *screen, unsigned char ch, unsigned char fg, unsigned char bg, unsigned short attr);
void screen_put_char(Screen *screen, int x, int y, unsigned char ch, unsigned char fg, unsigned char bg, unsigned short attr);
void screen_put_string(Screen *screen, int x, int y, const char *text, unsigned char fg, unsigned char bg, unsigned short attr);
void screen_draw_box(Screen *screen, int x, int y, int w, int h, unsigned char fg, unsigned char bg, unsigned short attr);
void screen_present(Screen *screen);

#endif
