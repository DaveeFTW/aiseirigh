#pragma once

#include <stddef.h>

typedef struct {
    unsigned char *ptr;
    size_t len;
} WorkBuffer;

WorkBuffer work_buffer_create(size_t len, size_t align);
