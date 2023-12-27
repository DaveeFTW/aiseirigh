#pragma once

#include <stddef.h>

typedef enum {
    FDISK_RESULT_OK,
    FDISK_ERR_LBA_OUT_OF_CHS_RANGE,
    FDISK_ERR_WRITE
} FdiskResult;

FdiskResult fdisk(size_t f0_size, size_t f1_size, size_t f2_size, size_t f3_size);
