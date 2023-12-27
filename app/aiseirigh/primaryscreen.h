#pragma once

#include "screen.h"
#include "colour.h"

void primary_set_screen(Screen *screen);
void primary_puts(Colour colour, const char *msg);
void primary_printf(Colour colour, const char *fmt, ...);
void primary_clear(void);
