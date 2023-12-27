#include "prxtype2.h"
#include "crypt.h"
#include "log.h"
#include <kirk.h>

#include <stdint.h>
#include <string.h>

typedef struct
{
    unsigned char personalisation[0x5C];
    unsigned char btcnf_id[0x10];
    unsigned char sha1_hash[0x14];
    unsigned char kirk_aes_key[0x10];
    unsigned char kirk_cmac_key[0x10];
    unsigned char kirk_header_hash[0x10];
    unsigned char kirk_data_hash[0x10];
    unsigned char kirk_metadata[0x10];
    unsigned char elf_info[0x80];
} PrxType2;

static PrxDecryptResult expand_seed(unsigned char *expanded_seed, const PrxMetaType *metatype)
{
    // TODO: remove constants?
    for (size_t i = 0; i < 0x90; i += 0x10) {
        memcpy(expanded_seed + i, metatype->seed, 0x10);
        expanded_seed[i] = i / 0x10;
    }

    if (kirk7_decrypt(expanded_seed, expanded_seed, 0x90, metatype->keyid) < 0) {
        return PRX_DECRYPT_ERR_KIRK7;
    }

    return PRX_DECRYPT_RESULT_OK;
}

PrxDecryptResult prx2_decrypt(unsigned char *image, const PrxMetaType *metatype)
{
    unsigned char xorbuf[0x90];
    PrxDecryptResult res = expand_seed(xorbuf, metatype);

    if (res != PRX_DECRYPT_RESULT_OK) {
        return res;
    }

    PrxType2 header;
    memcpy(header.personalisation, (unsigned char *)image + 0xD0, sizeof(header.personalisation));
    memcpy(header.btcnf_id, (unsigned char *)image + 0x140, sizeof(header.btcnf_id));
    memcpy(header.sha1_hash, (unsigned char *)image + 0x12C, sizeof(header.sha1_hash));
    memcpy(header.kirk_aes_key, (unsigned char *)image + 0x80, sizeof(header.kirk_aes_key));
    memcpy(header.kirk_cmac_key, (unsigned char *)image + 0x90, sizeof(header.kirk_cmac_key));
    memcpy(header.kirk_header_hash, (unsigned char *)image + 0xA0, sizeof(header.kirk_header_hash));
    memcpy(header.kirk_data_hash, (unsigned char *)image + 0xC0, sizeof(header.kirk_data_hash));
    memcpy(header.kirk_metadata, (unsigned char *)image + 0xB0, sizeof(header.kirk_metadata));
    memcpy(header.elf_info, (unsigned char *)image + 0x00, sizeof(header.elf_info));

    // decrypt the header
    int k7_res = kirk7_decrypt(header.btcnf_id, header.btcnf_id, 0x60, metatype->keyid);

    if (k7_res < 0) {
        return PRX_DECRYPT_ERR_KIRK7;
    }

    // TODO: sha1?

    unsigned char kirk_block[0x40];
    memcpy(kirk_block + 0x00, header.kirk_aes_key, sizeof(header.kirk_aes_key));
    memcpy(kirk_block + 0x10, header.kirk_cmac_key, sizeof(header.kirk_cmac_key));
    memcpy(kirk_block + 0x20, header.kirk_header_hash, sizeof(header.kirk_header_hash));
    memcpy(kirk_block + 0x30, header.kirk_data_hash, sizeof(header.kirk_data_hash));

    for (size_t i = 0; i < sizeof(kirk_block); ++i) {
        kirk_block[i] ^= xorbuf[0x10 + i];
    }

    if (kirk7_decrypt(kirk_block, kirk_block, sizeof(kirk_block), metatype->keyid) < 0) {
        return PRX_DECRYPT_ERR_KIRK7;
    }

    for (size_t i = 0; i < sizeof(kirk_block); ++i) {
        kirk_block[i] ^= xorbuf[0x50 + i];
    }

    memcpy((char *)image + 0x40 + 0x00, kirk_block, sizeof(kirk_block));
    memset((char *)image + 0x40 + 0x40, 0, 0x30);
    memcpy((char *)image + 0x40 + 0x70, header.kirk_metadata, sizeof(header.kirk_metadata));
    memset((char *)image + 0x40 + 0x80, 0, 0x10);
    memcpy((char *)image + 0x40 + 0x90, header.elf_info, sizeof(header.elf_info));

    uint32_t kirk_cmd = 1;
    memcpy((char *)image + 0x40 + 0x60, &kirk_cmd, sizeof(kirk_cmd));

    res = kirk1((void *)image, (void *)image + 0x40);
    
    if (res != 0) {
        return PRX_DECRYPT_ERR_KIRK1;
    }

    return PRX_DECRYPT_RESULT_OK;
}
