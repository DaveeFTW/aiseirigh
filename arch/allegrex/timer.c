/*
 * Copyright (c) 2015 Travis Geiselbrecht
 * Copyright (c) 2023 Davee Morgan
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <arch/allegrex.h>
#include <lk/err.h>
#include <lk/trace.h>
#include <lk/debug.h>
#include <assert.h>
#include <stdint.h>
#include <lk/bits.h>
#include <arch/ops.h>
#include <platform.h>
#include <platform/timer.h>

#include <cpu.h>
#include <interrupt.h>

#define LOCAL_TRACE 0

uint64_t g_accumulated_count_us;
static volatile uint32_t last_compare_set;

static uint32_t tick_rate;
static uint32_t tick_rate_mhz;

static lk_time_t tick_interval_ms;
static lk_bigtime_t tick_interval_us;
static uint32_t tick_interval;

static platform_timer_callback cb;
static void *cb_args;

static enum IrqHandleStatus allegrex_timer_irq(void)
{
    uint32_t count = allegrex_read_c0_count();
    g_accumulated_count_us += count / tick_rate_mhz;

    allegrex_write_c0_count(0);
    allegrex_write_c0_compare(tick_interval);

    enum handler_return ret = INT_NO_RESCHEDULE;

    if (cb) {
        lk_time_t now = current_time();
        ret = cb(cb_args, now);
    }

    return ret == INT_NO_RESCHEDULE ? IRQ_HANDLE_NO_RESCHEDULE : IRQ_HANDLE_RESCHEDULE;
}

status_t platform_set_periodic_timer(platform_timer_callback callback, void *arg, lk_time_t interval)
{
    TRACEF("callback %p, arg %p, interval %u\n", callback, arg, interval);

    DEBUG_ASSERT(interval > 0);
    DEBUG_ASSERT(tick_rate != 0 && tick_rate_mhz != 0);

    cb = callback;
    cb_args = arg;

    tick_interval_ms = interval;
    tick_interval_us = interval * 1000;
    tick_interval = interval * (tick_rate / 1000);
    g_accumulated_count_us = 0;

    allegrex_write_c0_count(0);
    allegrex_write_c0_compare(tick_interval);
    return NO_ERROR;
}

lk_time_t current_time(void)
{
    uint32_t mask = cpu_suspend_interrupts();
    uint64_t us = g_accumulated_count_us + allegrex_read_c0_count()/tick_rate_mhz;
    cpu_resume_interrupts(mask);

    // convert to milliseconds
    return us / 1000;
}

lk_bigtime_t current_time_hires(void)
{
    uint32_t mask = cpu_suspend_interrupts();
    uint64_t us = g_accumulated_count_us + allegrex_read_c0_count()/tick_rate_mhz;
    cpu_resume_interrupts(mask);

    // convert to milliseconds
    return us;
}

void allegrex_init_timer(uint32_t freq)
{
    tick_rate = freq;
    tick_rate_mhz = freq / 1000000;
    interrupt_set_handler(IRQ_COUNT, allegrex_timer_irq);
    allegrex_enable_irq(7);
}
