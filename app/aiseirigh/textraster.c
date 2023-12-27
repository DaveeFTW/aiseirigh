#include "screen.h"
#include "colour.h"

#include <kernel/mutex.h>
#include <lib/font.h>

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define FONT_PIXEL_WIDTH    (FONT_X)
#define FONT_PIXEL_HEIGHT   (FONT_Y)

static int needs_x_wrap(Screen *screen, unsigned int x)
{
    return (x + FONT_PIXEL_WIDTH) > screen_width(screen);
}

static int needs_y_wrap(Screen *screen, unsigned int y)
{
    return (y + FONT_PIXEL_HEIGHT) > screen_height(screen);
}

static void blit_character(Screen *screen, char ch, unsigned int x, unsigned int y, Colour fg_colour, Colour bg_colour)
{
    font_draw_char(screen->surface, ch, x, y, fg_colour);
    // const uint8_t *font_data = &msx[(uint32_t)ch * 8];

    // for (size_t y_offset = 0; y_offset < FONT_PIXEL_HEIGHT; ++y_offset) {
    //     uint8_t bitmap = font_data[y_offset];

    //     for (size_t x_offset = 0; x_offset < FONT_PIXEL_WIDTH; ++x_offset) {
    //         Colour colour = bitmap & (1 << (8 - x_offset - 1)) ? fg_colour : bg_colour;
    //         screen_set_pixel(screen, x + x_offset, y + y_offset, colour);
    //     }
    // }
}

static void clear_line(Screen *screen, Colour colour)
{
    for (size_t y = 0; y < FONT_PIXEL_HEIGHT; ++y) {
        for (size_t x = 0; x < screen_width(screen); ++x) {
            screen_set_pixel(screen, screen->cursor.x + x, screen->cursor.y + y, colour);
        }
    }
}

void text_rasterise_string(Screen *screen, Colour fg_colour, Colour bg_colour, const char *str, size_t len)
{
    mutex_acquire(&screen->mutex);

    for (size_t i = 0; str[i] && i < len; ++i) {
        if (needs_x_wrap(screen, screen->cursor.x) || str[i] == '\n') {
            screen->cursor.x = 0;
            screen->cursor.y += FONT_PIXEL_HEIGHT;

            if (needs_y_wrap(screen, screen->cursor.y)) {
                screen->cursor.y = 0;
            }

            clear_line(screen, bg_colour);

            if (str[i] == '\n') {
                continue;
            }
        }

        blit_character(screen, str[i], screen->cursor.x, screen->cursor.y, fg_colour, bg_colour);
        screen->cursor.x += FONT_PIXEL_WIDTH;
    }

    mutex_release(&screen->mutex);
}

void text_rasterise_stringf(Screen *screen, Colour fg_colour, Colour bg_colour, const char *fmt, ...)
{
    char fullstr[256];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(fullstr, sizeof(fullstr), fmt, ap);
    va_end(ap);

    text_rasterise_string(screen, fg_colour, bg_colour, fullstr, strlen(fullstr));
}
