#include "screen.h"
#include "colour.h"
#include "cursor.h"

#include <kernel/mutex.h>

#include <platform/display.h>

#include <display.h>
#include <dmacplus.h>

#include <stdint.h>
#include <string.h>

void screen_init(Screen *screen, gfx_surface *surface)
{
    memset(screen, 0, sizeof(*screen));
    screen->surface = surface;
    mutex_init(&screen->mutex);
}

void screen_fill(Screen *screen, Colour colour)
{
    gfx_clear(screen->surface, colour);
    screen->cursor.x = 0;
    screen->cursor.y = 0;
}

size_t screen_width(Screen *screen)
{
    return screen->surface->width;
}

size_t screen_height(Screen *screen)
{
    return screen->surface->height;
}

void screen_set_pixel(Screen *screen, unsigned int x, unsigned int y, Colour colour)
{
    gfx_putpixel(screen->surface, x, y, colour);
}

void screen_display(Screen *screen)
{
    set_active_framebuffer(screen->surface->ptr);
}
