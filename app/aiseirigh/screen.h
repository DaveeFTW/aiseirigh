#pragma once

#include "cursor.h"
#include "colour.h"

#include <kernel/mutex.h>
#include <lib/gfx.h>

#include <stddef.h>
#include <stdint.h>

typedef struct {
    gfx_surface *surface;
    mutex_t mutex;
    Cursor cursor;
} Screen;


void screen_init(Screen *screen, gfx_surface *surface);
size_t screen_width(Screen *screen);
size_t screen_height(Screen *screen);
void screen_fill(Screen *screen, Colour colour);
void screen_set_pixel(Screen *screen, unsigned int x, unsigned int y, Colour colour);
void screen_display(Screen *screen);
