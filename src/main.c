#include "screen.h"

#include <SDL2/SDL.h>

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define GROUND_Y 23
#define UI_Y 24
#define PIVOT_X 4
#define PIVOT_Y 14

typedef enum {
    STATE_READY = 0,
    STATE_ATTACHED,
    STATE_FLYING,
    STATE_LANDED
} GameMode;

typedef struct {
    GameMode mode;
    bool accelerating;
    bool title_screen;
    bool start_blink;
    Uint32 blink_started_at;
    double theta;
    double omega;
    double radius;
    double x;
    double y;
    double vx;
    double vy;
    double landed_distance;
    char message[80];
} GameState;

static double clamp_double(double value, double min_value, double max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static void set_message(GameState *game, const char *text) {
    snprintf(game->message, sizeof(game->message), "%s", text);
}

static void update_attached_position(GameState *game) {
    game->x = PIVOT_X + sin(game->theta) * game->radius;
    game->y = PIVOT_Y + cos(game->theta) * game->radius;
}

static void reset_game(GameState *game) {
    memset(game, 0, sizeof(*game));
    game->mode = STATE_READY;
    game->title_screen = false;
    game->start_blink = false;
    game->blink_started_at = 0;
    game->theta = 0.0;
    game->omega = 0.0;
    game->radius = 2.0;
    update_attached_position(game);
    set_message(game, "SPACE spin/release. R reset. Q title.");
}

static void reset_to_title(GameState *game) {
    reset_game(game);
    game->title_screen = true;
    set_message(game, "Push space bar,then game start");
}

static void release_flyer(GameState *game) {
    game->mode = STATE_FLYING;
    game->accelerating = false;
    game->vx = game->radius * game->omega * cos(game->theta);
    game->vy = -game->radius * game->omega * sin(game->theta);
    set_message(game, "FLY!");
}

static void draw_ground(Screen *screen) {
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        screen_put_char(screen, x, GROUND_Y, '#', COLOR_GREEN, COLOR_BLACK, 0);
    }
}

static void draw_title(Screen *screen) {
    int box_w = 46;
    int box_h = 11;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int box_y = 5;

    screen_clear(screen, ' ', COLOR_LIGHTGRAY, COLOR_BLACK, 0);
    screen_draw_box(screen, box_x, box_y, box_w, box_h, COLOR_YELLOW, COLOR_BLUE, 0);
    screen_put_string_center(screen, box_y + 1, "LEGACY PC HIGHBAR", COLOR_WHITE, COLOR_BLUE, 0);
    screen_put_string(screen, box_x + 2, box_y + 3, "RULE:", COLOR_LIGHTCYAN, COLOR_BLUE, 0);
    screen_put_string(screen, box_x + 8, box_y + 3, "Spin up and release for distance.", COLOR_WHITE, COLOR_BLUE, 0);
    screen_put_string(screen, box_x + 2, box_y + 5, "CONTROL:", COLOR_LIGHTCYAN, COLOR_BLUE, 0);
    screen_put_string(screen, box_x + 11, box_y + 5, "SPACE start/release  R reset", COLOR_WHITE, COLOR_BLUE, 0);
    screen_put_string(screen, box_x + 11, box_y + 6, "Q title  ESC quit", COLOR_WHITE, COLOR_BLUE, 0);
    screen_put_string_center(screen, box_y + 9, "Push space bar,then game start", COLOR_LIGHTGREEN, COLOR_BLUE, 0);
}

static void draw_start_blink(Screen *screen, Uint32 started_at) {
    Uint32 elapsed = SDL_GetTicks() - started_at;
    Uint32 phase = elapsed / 500;

    screen_clear(screen, ' ', COLOR_LIGHTGRAY, COLOR_BLACK, 0);
    if ((phase % 2) == 0) {
        int y = GROUND_Y / 2;
        screen_put_string_center(screen, y, "Let's spin!!", COLOR_YELLOW, COLOR_BLACK, 0);
    }
}

static void draw_highbar(Screen *screen) {
    screen_put_char(screen, PIVOT_X, PIVOT_Y, 'O', COLOR_YELLOW, COLOR_BLACK, 0);
}

static void draw_flyer(Screen *screen, const GameState *game) {
    if (game->mode == STATE_FLYING || game->mode == STATE_LANDED) {
        int body_x = (int)lround(game->x);
        int body_y1 = (int)lround(game->y);
        int body_y2 = body_y1 + 1;
        static const char *flip_frames[] = { "_|", "/\\", "|_", "\\/" };

        if (game->mode == STATE_FLYING && body_y1 < GROUND_Y - 4) {
            Uint32 frame = (SDL_GetTicks() / 80) % 4;
            const char *sprite = flip_frames[frame];
            int left_x = body_x - 1;
            int right_x = body_x;

            if (left_x >= 0 && left_x < SCREEN_WIDTH && body_y1 >= 0 && body_y1 < GROUND_Y) {
                screen_put_char(screen, left_x, body_y1, (unsigned char)sprite[0], COLOR_WHITE, COLOR_BLACK, 0);
            }
            if (right_x >= 0 && right_x < SCREEN_WIDTH && body_y1 >= 0 && body_y1 < GROUND_Y) {
                screen_put_char(screen, right_x, body_y1, (unsigned char)sprite[1], COLOR_WHITE, COLOR_BLACK, 0);
            }
            return;
        }

        if (body_x >= 0 && body_x < SCREEN_WIDTH && body_y1 >= 0 && body_y1 < GROUND_Y) {
            screen_put_char(screen, body_x, body_y1, '|', COLOR_WHITE, COLOR_BLACK, 0);
        }
        if (body_x >= 0 && body_x < SCREEN_WIDTH && body_y2 >= 0 && body_y2 < GROUND_Y) {
            screen_put_char(screen, body_x, body_y2, '|', COLOR_WHITE, COLOR_BLACK, 0);
        }
        return;
    }

    int dx = 0;
    int dy = 1;
    char ch = '|';
    double normalized = game->theta;

    while (normalized <= -M_PI) {
        normalized += 2.0 * M_PI;
    }
    while (normalized > M_PI) {
        normalized -= 2.0 * M_PI;
    }

    if (normalized > -M_PI / 8.0 && normalized <= M_PI / 8.0) {
        dx = 0;
        dy = 1;
        ch = '|';
    } else if (normalized > M_PI / 8.0 && normalized <= 3.0 * M_PI / 8.0) {
        dx = 1;
        dy = 1;
        ch = '\\';
    } else if (normalized > 3.0 * M_PI / 8.0 && normalized <= 5.0 * M_PI / 8.0) {
        dx = 1;
        dy = 0;
        ch = '-';
    } else if (normalized > 5.0 * M_PI / 8.0 && normalized <= 7.0 * M_PI / 8.0) {
        dx = 1;
        dy = -1;
        ch = '/';
    } else if (normalized <= -M_PI / 8.0 && normalized > -3.0 * M_PI / 8.0) {
        dx = -1;
        dy = 1;
        ch = '/';
    } else if (normalized <= -3.0 * M_PI / 8.0 && normalized > -5.0 * M_PI / 8.0) {
        dx = -1;
        dy = 0;
        ch = '-';
    } else if (normalized <= -5.0 * M_PI / 8.0 && normalized > -7.0 * M_PI / 8.0) {
        dx = -1;
        dy = -1;
        ch = '\\';
    } else {
        dx = 0;
        dy = -1;
        ch = '|';
    }

    for (int segment = 1; segment <= 2; ++segment) {
        int px = PIVOT_X + dx * segment;
        int py = PIVOT_Y + dy * segment;
        if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < GROUND_Y) {
            screen_put_char(screen, px, py, ch, COLOR_WHITE, COLOR_BLACK, 0);
        }
    }
}

static void draw_scene(Screen *screen, const GameState *game) {
    char ui[128];

    screen_clear(screen, ' ', COLOR_LIGHTGRAY, COLOR_BLACK, 0);
    draw_ground(screen);
    draw_highbar(screen);
    draw_flyer(screen, game);

    if (game->mode == STATE_LANDED) {
        char result[80];
        snprintf(result, sizeof(result), "DISTANCE: %.2fm  PRESS R TO RETRY", game->landed_distance / 10.0);
        screen_put_string(screen, 18, 2, result, COLOR_YELLOW, COLOR_BLACK, 0);
    }

    snprintf(
        ui,
        sizeof(ui),
        "SPACE spin/release  Q title  R reset  ANG:%6.1f  SPD:%5.2f  %s",
        game->theta * 180.0 / M_PI,
        fabs(game->omega),
        game->message
    );
    screen_put_string(screen, 0, UI_Y, ui, COLOR_LIGHTCYAN, COLOR_BLACK, 0);
}

static void update_game(GameState *game, double dt) {
    const double angular_accel = 8.0;
    const double angular_damping = 0.998;
    const double max_omega = 25.0;
    const double gravity = 16.0;

    if (game->mode == STATE_READY) {
        game->mode = STATE_ATTACHED;
    }

    if (game->mode == STATE_ATTACHED) {
        if (game->accelerating) {
            game->omega += angular_accel * dt;
            game->omega = clamp_double(game->omega, 0.0, max_omega);
            game->theta += game->omega * dt;
            if (game->theta > M_PI) {
                game->theta -= 2.0 * M_PI;
            } else if (game->theta <= -M_PI) {
                game->theta += 2.0 * M_PI;
            }
            set_message(game, "SPINNING...");
        }

        game->omega *= angular_damping;
        if (game->omega < 0.02) {
            game->omega = 0.0;
        }
        update_attached_position(game);
        return;
    }

    if (game->mode == STATE_FLYING) {
        game->x += game->vx * dt;
        game->y += game->vy * dt;
        game->vy += gravity * dt;

        if (game->y >= GROUND_Y - 2) {
            game->y = GROUND_Y - 2;
            game->mode = STATE_LANDED;
            game->landed_distance = game->x - PIVOT_X;
            set_message(game, "LANDED");
        }

        if (game->x < 0.0) {
            game->x = 0.0;
        } else if (game->x > (double)(SCREEN_WIDTH - 1)) {
            game->x = (double)(SCREEN_WIDTH - 1);
        }
    }
}

int main(void) {
    Screen screen;
    GameState game;
    SDL_Event event;
    bool running = true;
    Uint32 previous_ticks = 0;
    const char *font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";

    reset_to_title(&game);

    if (screen_init(&screen, "Legacy-PC-HighBar-PoC", font_path, 14) != 0) {
        return 1;
    }

    previous_ticks = SDL_GetTicks();

    while (running) {
        Uint32 current_ticks = SDL_GetTicks();
        double dt = (double)(current_ticks - previous_ticks) / 1000.0;
        previous_ticks = current_ticks;
        dt = clamp_double(dt, 0.0, 0.05);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) {
                    running = false;
                } else if (key == SDLK_q) {
                    reset_to_title(&game);
                } else if (key == SDLK_r && !game.title_screen) {
                    reset_game(&game);
                } else if (game.title_screen && key == SDLK_SPACE) {
                    reset_game(&game);
                    game.start_blink = true;
                    game.blink_started_at = SDL_GetTicks();
                } else if (key == SDLK_SPACE && (game.mode == STATE_READY || game.mode == STATE_ATTACHED)) {
                    if (!game.accelerating) {
                        game.accelerating = true;
                        game.mode = STATE_ATTACHED;
                        set_message(&game, "SPINNING...");
                    } else {
                        release_flyer(&game);
                    }
                }
            }
        }

        if (game.title_screen) {
            draw_title(&screen);
        } else if (game.start_blink) {
            if (SDL_GetTicks() - game.blink_started_at >= 4000) {
                game.start_blink = false;
                screen_clear(&screen, ' ', COLOR_LIGHTGRAY, COLOR_BLACK, 0);
            }
            draw_start_blink(&screen, game.blink_started_at);
        } else {
            update_game(&game, dt);
            draw_scene(&screen, &game);
        }
        screen_present(&screen);
        SDL_Delay(16);
    }

    screen_shutdown(&screen);
    return 0;
}
