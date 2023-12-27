#include "workbuffer.h"

#include <stdlib.h>

WorkBuffer work_buffer_create(size_t len, size_t align)
{
    WorkBuffer work_buffer = {
        .ptr = memalign(align, len),
        .len = len
    };

    return work_buffer;
}
