/*
 * Copyright (c) 2015 Travis Geiselbrecht
 * Copyright (c) 2023 Davee Morgan
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#pragma once

#ifndef ASSEMBLY
#include <lk/compiler.h>
#include <stdint.h>
#include <sys/types.h>

#define GEN_CP_REG_FUNCS(regname, regnum, sel) \
static inline __ALWAYS_INLINE uint32_t allegrex_read_##regname(void) { \
    uint32_t val; \
    __asm__ volatile("mfc0 %0, $" #regnum: "=r" (val)); \
    return val; \
} \
\
static inline __ALWAYS_INLINE uint32_t allegrex_read_##regname##_relaxed(void) { \
    uint32_t val; \
    __asm__("mfc0 %0, $" #regnum : "=r" (val)); \
    return val; \
} \
\
static inline __ALWAYS_INLINE void allegrex_write_##regname(uint32_t val) { \
    __asm__ volatile("mtc0 %0, $" #regnum :: "r" (val)); \
} \
\
static inline __ALWAYS_INLINE void allegrex_write_##regname##_relaxed(uint32_t val) { \
    __asm__ volatile("mtc0 %0, $" #regnum :: "r" (val)); \
}

GEN_CP_REG_FUNCS(c0_count, 9, 0)
GEN_CP_REG_FUNCS(c0_compare, 11, 0)
GEN_CP_REG_FUNCS(c0_status, 12, 0)
GEN_CP_REG_FUNCS(c0_cause, 13, 0)
GEN_CP_REG_FUNCS(c0_epc, 14, 0)
GEN_CP_REG_FUNCS(c0_prid, 15, 0)
GEN_CP_REG_FUNCS(c0_config, 16, 0)
GEN_CP_REG_FUNCS(c0_ebase, 25, 0)

struct allegrex_iframe {
    uint32_t at;
    uint32_t v0;
    uint32_t v1;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t t7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t t8;
    uint32_t t9;
    uint32_t k0;
    uint32_t k1;
    uint32_t gp;
    uint32_t sp;
    uint32_t fp;
    uint32_t ra;
    uint32_t lo;
    uint32_t hi;
    uint32_t status;
    uint32_t cause;
    uint32_t epc;
};
STATIC_ASSERT(sizeof(struct allegrex_iframe) == 0x90);

void allegrex_init_timer(uint32_t freq);
void allegrex_enable_irq(uint num);
void allegrex_disable_irq(uint num);

#endif // !ASSEMBLY
