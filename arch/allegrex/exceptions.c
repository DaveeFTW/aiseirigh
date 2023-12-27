/*
 * Copyright (c) 2015 Travis Geiselbrecht
 * Copyright (c) 2023 Davee Morgan
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <lk/trace.h>
#include <lk/debug.h>
#include <assert.h>
#include <stdint.h>
#include <lk/bits.h>
#include <kernel/thread.h>
#include <kernel/debug.h>
#include <arch/allegrex.h>

#define LOCAL_TRACE 0

enum ExceptionCode
{
    EXCEPTION_IRQ = 0,
    EXCEPTION_SYSCALL = 8
};

void allegrex_gen_exception(struct allegrex_iframe *iframe) 
{
    enum ExceptionCode excode = BITS_SHIFT(iframe->cause, 6, 2);

    switch (excode) {
        case EXCEPTION_SYSCALL:
            LTRACEF("SYSCALL, EPC 0x%x\n", iframe->epc);
            iframe->epc += 4;
            break;

        default:
            printf("status 0x%x\n", iframe->status);
            printf("cause 0x%x\n", iframe->cause);
            printf("\texcode 0x%x\n", excode);
            printf("epc 0x%x\n", iframe->epc);
            for (;;);
    }
}
