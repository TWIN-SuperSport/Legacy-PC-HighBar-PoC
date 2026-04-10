#include "screen.h"

#include <stdio.h>
#include <string.h>

static const SDL_Color ANSI_COLORS[16] = {
    {0x00, 0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xAA, 0xFF},
    {0x00, 0xAA, 0x00, 0xFF},
    {0x00, 0xAA, 0xAA, 0xFF},
    {0xAA, 0x00, 0x00, 0xFF},
    {0xAA, 0x00, 0xAA, 0xFF},
    {0xAA, 0x55, 0x00, 0xFF},
    {0xAA, 0xAA, 0xAA, 0xFF},
    {0x55, 0x55, 0x55, 0xFF},
    {0x55, 0x55, 0xFF, 0xFF},
    {0x55, 0xFF, 0x55, 0xFF},
    {0x55, 0xFF, 0xFF, 0xFF},
    {0xFF, 0x55, 0x55, 0xFF},
    {0xFF, 0x55, 0xFF, 0xFF},
    {0xFF, 0xFF, 0x55, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF},
};

static void destroy_screen(Screen *screen) {
    if (screen->font != NULL) {
        TTF_CloseFont(screen->font);
        screen->font = NULL;
    }
    if (screen->renderer != NULL) {
        SDL_DestroyRenderer(screen->renderer);
        screen->renderer = NULL;
    }
    if (screen->window != NULL) {
        SDL_DestroyWindow(screen->window);
        screen->window = NULL;
    }
}

int screen_init(Screen *screen, const char *title, const char *font_path, int font_ptsize) {
    memset(screen, 0, sizeof(*screen));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    screen->font = TTF_OpenFont(font_path, font_ptsize);
    if (screen->font == NULL) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    if (TTF_SizeText(screen->font, "W", &screen->cell_width, &screen->cell_height) != 0) {
        fprintf(stderr, "TTF_SizeText failed: %s\n", TTF_GetError());
        destroy_screen(screen);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    screen->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screen->cell_width * SCREEN_WIDTH,
        screen->cell_height * SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (screen->window == NULL) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        destroy_screen(screen);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    screen->renderer = SDL_CreateRenderer(screen->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (screen->renderer == NULL) {
        screen->renderer = SDL_CreateRenderer(screen->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (screen->renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        destroy_screen(screen);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    screen_clear(screen, ' ', COLOR_LIGHTGRAY, COLOR_BLACK, 0);
    return 0;
}

void screen_shutdown(Screen *screen) {
    destroy_screen(screen);
    TTF_Quit();
    SDL_Quit();
}

void screen_clear(Screen *screen, unsigned char ch, unsigned char fg, unsigned char bg, unsigned short attr) {
    int y;
    int x;

    for (y = 0; y < SCREEN_HEIGHT; ++y) {
        for (x = 0; x < SCREEN_WIDTH; ++x) {
            screen->cells[y][x].ch = ch;
            screen->cells[y][x].fg = fg;
            screen->cells[y][x].bg = bg;
            screen->cells[y][x].attr = attr;
        }
    }
}

void screen_put_char(Screen *screen, int x, int y, unsigned char ch, unsigned char fg, unsigned char bg, unsigned short attr) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;
    }

    screen->cells[y][x].ch = ch;
    screen->cells[y][x].fg = fg;
    screen->cells[y][x].bg = bg;
    screen->cells[y][x].attr = attr;
}

void screen_put_string(Screen *screen, int x, int y, const char *text, unsigned char fg, unsigned char bg, unsigned short attr) {
    int i;

    for (i = 0; text[i] != '\0'; ++i) {
        screen_put_char(screen, x + i, y, (unsigned char)text[i], fg, bg, attr);
    }
}

void screen_put_string_center(Screen *screen, int y, const char *text, unsigned char fg, unsigned char bg, unsigned short attr) {
    int text_width = (int)strlen(text);
    int x = (SCREEN_WIDTH - text_width) / 2;
    screen_put_string(screen, x, y, text, fg, bg, attr);
}

void screen_put_string_right(Screen *screen, int right_x, int y, const char *text, unsigned char fg, unsigned char bg, unsigned short attr) {
    int text_width = (int)strlen(text);
    int x = right_x - text_width + 1;
    screen_put_string(screen, x, y, text, fg, bg, attr);
}

void screen_draw_box(Screen *screen, int x, int y, int w, int h, unsigned char fg, unsigned char bg, unsigned short attr) {
    int i;

    if (w < 2 || h < 2) {
        return;
    }

    screen_put_char(screen, x, y, '+', fg, bg, attr);
    screen_put_char(screen, x + w - 1, y, '+', fg, bg, attr);
    screen_put_char(screen, x, y + h - 1, '+', fg, bg, attr);
    screen_put_char(screen, x + w - 1, y + h - 1, '+', fg, bg, attr);

    for (i = 1; i < w - 1; ++i) {
        screen_put_char(screen, x + i, y, '-', fg, bg, attr);
        screen_put_char(screen, x + i, y + h - 1, '-', fg, bg, attr);
    }

    for (i = 1; i < h - 1; ++i) {
        screen_put_char(screen, x, y + i, '|', fg, bg, attr);
        screen_put_char(screen, x + w - 1, y + i, '|', fg, bg, attr);
    }
}

void screen_present(Screen *screen) {
    int y;
    int x;

    SDL_SetRenderDrawColor(screen->renderer, 0, 0, 0, 255);
    SDL_RenderClear(screen->renderer);

    for (y = 0; y < SCREEN_HEIGHT; ++y) {
        for (x = 0; x < SCREEN_WIDTH; ++x) {
            Cell cell = screen->cells[y][x];
            SDL_Rect rect = {
                x * screen->cell_width,
                y * screen->cell_height,
                screen->cell_width,
                screen->cell_height
            };
            SDL_Surface *surface;
            SDL_Texture *texture;
            char glyph[2];

            SDL_Color bg = ANSI_COLORS[cell.bg & 0x0F];
            SDL_Color fg = ANSI_COLORS[cell.fg & 0x0F];

            SDL_SetRenderDrawColor(screen->renderer, bg.r, bg.g, bg.b, 255);
            SDL_RenderFillRect(screen->renderer, &rect);

            if (cell.ch == ' ') {
                continue;
            }

            glyph[0] = (char)cell.ch;
            glyph[1] = '\0';

            surface = TTF_RenderText_Solid(screen->font, glyph, fg);
            if (surface == NULL) {
                continue;
            }

            texture = SDL_CreateTextureFromSurface(screen->renderer, surface);
            if (texture != NULL) {
                SDL_Rect dst = {
                    rect.x,
                    rect.y,
                    surface->w,
                    surface->h
                };
                SDL_RenderCopy(screen->renderer, texture, NULL, &dst);
                SDL_DestroyTexture(texture);
            }

            SDL_FreeSurface(surface);
        }
    }

    SDL_RenderPresent(screen->renderer);
}
