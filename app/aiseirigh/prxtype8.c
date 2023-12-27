#include "prxtype8.h"
#include "crypt.h"
#include "log.h"
#include <kirk.h>

#include <stdint.h>
#include <string.h>

PrxDecryptResult prx8_decrypt(unsigned char *image, const PrxMetaType *metatype)
{
    unsigned char reorganised_header[0x150];
    memcpy(reorganised_header + 0x00, (char *)image + 0xD0, 0x80);
    memcpy(reorganised_header + 0x80, (char *)image + 0x80, 0x50);
    memcpy(reorganised_header + 0xD0, (char *)image + 0x00, 0x80);

    unsigned char xorbuf[0x90];
    if (kirk7_decrypt(xorbuf, metatype->seed, sizeof(xorbuf), metatype->keyid) < 0) {
        return PRX_DECRYPT_ERR_KIRK7;
    }

    unsigned sha_buf[0x150];
    memcpy(sha_buf + 0x000, xorbuf, 0x14);
    memcpy(sha_buf + 0x014, reorganised_header + 0x18, 0x28);
    memcpy(sha_buf + 0x03C, reorganised_header + 0x40, 0x70);
    memcpy(sha_buf + 0x0AC, reorganised_header + 0xB0, 0x20);
    memcpy(sha_buf + 0x0CC, reorganised_header + 0xD0, 0x80);
    size_t sha_len = 0x14C;

    // if (kirkB(sha_buf, sha_buf, sha_len) < 0) {
    //     return -3;
    // }

    unsigned char kirk_block[0x70];

    for (size_t i = 0; i < sizeof(kirk_block); ++i) {
        kirk_block[i] = reorganised_header[0x40 + i] ^ xorbuf[0x14 + i];
    }

    if (kirk7_decrypt(kirk_block, kirk_block, sizeof(kirk_block), metatype->keyid) < 0) {
        return PRX_DECRYPT_ERR_KIRK7;
    }

    for (size_t i = 0; i < sizeof(kirk_block); ++i) {
        kirk_block[i] ^= xorbuf[0x20 + i];
    }

    memcpy((char *)image + 0x40 + 0x00, kirk_block, sizeof(kirk_block));
    memcpy((char *)image + 0x40 + 0x70, reorganised_header + 0xB0, 0x20);
    memcpy((char *)image + 0x40 + 0x90, reorganised_header + 0xD0, 0x80);

    int res = kirk1((void *)image, (void *)image + 0x40);
    
    if (res != 0) {
        return PRX_DECRYPT_ERR_KIRK1;
    }

    return PRX_DECRYPT_RESULT_OK;
}
