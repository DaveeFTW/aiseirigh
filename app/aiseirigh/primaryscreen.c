#include "primaryscreen.h"
#include "textraster.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

static Screen *g_screen = NULL;

void primary_set_screen(Screen *screen)
{
    g_screen = screen;
}

void primary_puts(Colour colour, const char *msg)
{
    assert(g_screen != NULL);

    text_rasterise_string(g_screen, colour, BG_COLOUR, msg, strlen(msg));
    puts(msg);
}

void primary_printf(Colour colour, const char *fmt, ...)
{
    assert(g_screen != NULL);

    char out[1];
    va_list ap;

    va_start(ap, fmt);
    size_t len = vsnprintf(out, 0, fmt, ap);
    va_end(ap);

    char *msg = malloc(len + 1);
    assert(msg != NULL);

    va_start(ap, fmt);
    vsnprintf(msg, len + 1, fmt, ap);
    va_end(ap);

    text_rasterise_string(g_screen, colour, BG_COLOUR, msg, len);
    puts(msg);
    free(msg);
    msg = NULL;
}

void primary_clear(void)
{
    assert(g_screen != NULL);
    screen_fill(g_screen, BG_COLOUR);
}
