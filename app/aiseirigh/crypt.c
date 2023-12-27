#include "crypt.h"
#include "log.h"

#include <cpu.h>
#include <kirk.h>
#include <string.h>

typedef struct
{
    volatile unsigned int signature;
    volatile unsigned int version;
    volatile unsigned int error;
    volatile unsigned int proc_phase;
    volatile unsigned int command;
    volatile unsigned int result;
    volatile unsigned int unk_18;
    volatile unsigned int status;
    volatile unsigned int status_async;
    volatile unsigned int status_async_end;
    volatile unsigned int status_end;
    volatile unsigned int src_addr;
    volatile unsigned int dst_addr;
} PspKirkRegs;

#define MAKE_PHYS_ADDR(_addr)    (((unsigned int)_addr) & 0x1FFFFFFF)
#define SYNC()                   __asm ("sync")
#define KIRK_HW_REGISTER_ADDR    ((PspKirkRegs *)0xBDE00000)

int kirk111(void *dst, const void *src)
{
    cpu_dcache_wb_inv_all();
    PspKirkRegs *const kirk = KIRK_HW_REGISTER_ADDR;
 
    kirk->command = 1; // decrypt operation
    kirk->src_addr = MAKE_PHYS_ADDR(src);
    kirk->dst_addr = MAKE_PHYS_ADDR(dst);
 
    // begin processing
    kirk->proc_phase = 1;
    SYNC();

    // wait until we advance from the initial phase
    while ((kirk->proc_phase & 1) != 0);

    // wait until status is set
    while (kirk->status == 0);

    kirk->status_end = kirk->status & kirk->status_async;
    SYNC();
    return kirk->result;
}


int kirk1_decrypt(unsigned char *dst, const unsigned char *src, size_t len)
{
    if (len > (0x400 - 0x14)) {
        return -1;
    }

    memcpy((void *)0xBFC00C00, src, len);

    if (kirk111((void *)0xBFC00C00, (void *)0xBFC00C00) < 0) {
        return -2;
    }

    memcpy(dst, (void *)0xBFC00C00, len);
    return 0;
}

int kirk5_encrypt(unsigned char *dst, const unsigned char *src, size_t len)
{
    if (len > (0x400 - 0x14)) {
        return -1;
    }

    volatile Kirk58Header *hdr = (volatile Kirk58Header *)0xBFC00C00;
    memcpy((void *)(0xBFC00C00 + sizeof(Kirk58Header)), src, len);
    
    hdr->mode = 4;
    hdr->unk_4 = 0;
    hdr->unk_8 = 0;
    hdr->keyslot = 0x100;
    hdr->size = len;

    int res = kirk5((void *)0xBFC00C00, (void *)0xBFC00C00);

    if (res != 0) {
        LOG("%s: kirk5 %i", __FUNCTION__, res);
        return -2;
    }

    memcpy(dst, (void *)0xBFC00C00 + sizeof(Kirk58Header), len);
    return 0;
}

int kirk7_decrypt(unsigned char *dst, const unsigned char *src, size_t len, int keyid)
{
    if (len > (0x400 - 0x14)) {
        return -1;
    }

    volatile Kirk47Header *hdr = (volatile Kirk47Header *)0xBFC00C00;
    memcpy((void *)(0xBFC00C00 + sizeof(Kirk47Header)), src, len);
    
    hdr->mode = 5;
    hdr->unk_4 = 0;
    hdr->unk_8 = 0;
    hdr->keyslot = keyid;
    hdr->size = len;

    int res = kirk7((void *)0xBFC00C00, (void *)0xBFC00C00);

    if (res != 0) {
        return -2;
    }

    memcpy(dst, (void *)0xBFC00C00, len);
    return 0;
}