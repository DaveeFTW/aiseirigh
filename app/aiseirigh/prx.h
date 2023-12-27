#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum {
    PRX_TYPE_2,
    PRX_TYPE_8
} PrxType;

typedef struct {
    unsigned int tag;
    int keyid;
    PrxType type;
    const uint8_t *seed;
} PrxMetaType;

typedef enum {
    PRX_DECRYPT_RESULT_OK,
    PRX_DECRYPT_ERR_UNSUPPORTED,
    PRX_DECRYPT_ERR_KIRK1,
    PRX_DECRYPT_ERR_KIRK7,
} PrxDecryptResult;

PrxDecryptResult prx_decrypt(void *data, size_t *out_length);
