/*
 * Copyright (c) 2015 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <sys/types.h>

struct allegrex_context_switch_frame {
    /* callee saved */
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t ra;
    uint32_t sp;
};

struct arch_thread {
    struct allegrex_context_switch_frame cs_frame;
};

void allegrex_context_switch(struct allegrex_context_switch_frame *oldcs, struct allegrex_context_switch_frame *newcs);

