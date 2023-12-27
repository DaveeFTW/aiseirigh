#pragma once

#include <stdint.h>

typedef uint32_t Colour;

#define COLOUR_RGB(r, g, b)     ((Colour)(((Colour)b & 0xFF) | (((Colour)g & 0xFF) << 8) | (((Colour)r & 0xFF) << 16)))

#define FG_COLOUR               COLOUR_RGB(241, 143, 1)
#define FG_COLOUR_ERR           COLOUR_RGB(255, 0, 0)
#define FG_COLOUR_SUCCESS       COLOUR_RGB(0, 255, 0)
#define FG_COLOUR_DBG           COLOUR_RGB(0, 230, 0)
#define BG_COLOUR               COLOUR_RGB(0, 0, 0)
