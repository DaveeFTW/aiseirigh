#pragma once

#include "screen.h"
#include "colour.h"

void text_rasterise_string(Screen *screen, Colour fg_colour, Colour bg_colour, const char *str, size_t len);
void text_rasterise_stringf(Screen *screen, Colour fg_colour, Colour bg_colour, const char *fmt, ...);
