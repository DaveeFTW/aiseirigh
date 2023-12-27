#pragma once

#include <stddef.h>

int des_decrypt_cbc(void *dst, const void *src, size_t len, const void *key, const void *iv);
