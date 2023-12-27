/*
 * Copyright (c) 2023 Davee Morgan
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */

#include <lk/err.h>
#include <lk/debug.h>
#include <lk/trace.h>
#include <dev/display.h>

#include <platform/display.h>

#include <display.h>
#include <dmacplus.h>

#include <string.h>
#include <assert.h>

#define DISP_WIDTH          (480)
#define DISP_HEIGHT         (272)
#define DISP_STRIDE         (512)
#define FRAMEBUFFER_SIZE    (DISP_STRIDE * DISP_WIDTH * 4)

// TODO: this should be somewhere else
#define EDRAM_BASE  (0x04000000)

static unsigned char *g_framebuffer1 = (unsigned char *)EDRAM_BASE;
static unsigned char *g_framebuffer2 = (unsigned char *)(EDRAM_BASE + FRAMEBUFFER_SIZE);
static unsigned char *g_active_framebuffer;

static void get_framebuffer(struct display_framebuffer *fb, unsigned char *mem)
{
    fb->image.pixels = (void *)mem;
    fb->format = DISPLAY_FORMAT_ABGR_8888;
    fb->image.format = IMAGE_FORMAT_ABGR_8888;
    fb->image.rowbytes = DISP_WIDTH * 4;
    fb->image.width = DISP_WIDTH;
    fb->image.height = DISP_HEIGHT;
    fb->image.stride = DISP_STRIDE;
    fb->flush = NULL;
}

void init_display_framebuffer(void)
{
    memset(g_framebuffer1, 0, FRAMEBUFFER_SIZE);
    memset(g_framebuffer2, 0, FRAMEBUFFER_SIZE);
    g_active_framebuffer = g_framebuffer1;
    display_set_mode(0, DISP_WIDTH, DISP_HEIGHT, DISP_STRIDE, PIXEL_FORMAT_RGBA8888);
    display_set_framebuffer(g_active_framebuffer);
}

void set_active_framebuffer(unsigned char *fb)
{
    g_active_framebuffer = fb;
    display_set_framebuffer(g_active_framebuffer);
}

void get_all_framebuffers(struct display_framebuffer *fbs, size_t *num_fbs)
{
    if (!num_fbs) {
        return;
    }

    if (!fbs) {
        *num_fbs = 2;
        return;
    }

    switch (*num_fbs) {
        case 0:
            break;
        case 1:
            get_framebuffer(&fbs[0], g_framebuffer1);
            break;

        default:
            get_framebuffer(&fbs[0], g_framebuffer1);
            get_framebuffer(&fbs[1], g_framebuffer2);
            break;
    }
}

status_t display_get_framebuffer(struct display_framebuffer *fb)
{
    get_framebuffer(fb, g_active_framebuffer);
    return NO_ERROR;
}

status_t display_get_info(struct display_info *info)
{
    info->format = DISPLAY_FORMAT_ABGR_8888;
    info->width = DISP_WIDTH;
    info->height = DISP_HEIGHT;
    return NO_ERROR;
}

status_t display_present(struct display_image *image, uint starty, uint endy)
{
    TRACEF("display_present - not implemented");
    DEBUG_ASSERT(false);
    return NO_ERROR;
}
