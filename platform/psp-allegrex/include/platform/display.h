#pragma once

#include <stddef.h>

void init_display_framebuffer(void);
void get_all_framebuffers(struct display_framebuffer *fbs, size_t *num_fbs);
void set_active_framebuffer(unsigned char *fb);
