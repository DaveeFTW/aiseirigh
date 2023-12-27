#include <mbedtls/des.h>

#include <string.h>
#include <stdint.h>

int des_decrypt_cbc(void *dst, const void *src, size_t len, const void *key, const void *iv)
{
    mbedtls_des_context ctx;
    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_dec(&ctx, key);
    unsigned char iv_buf[8];
    memcpy(iv_buf, iv, sizeof(iv_buf));
    return mbedtls_des_crypt_cbc(&ctx, MBEDTLS_DES_DECRYPT, len, iv_buf, src, dst);
}
