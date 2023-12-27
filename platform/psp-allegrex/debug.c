/*
 * Copyright (c) 2009 Corey Tabaka
 * Copyright (c) 2015 Travis Geiselbrecht
 * Copyright (c) 2023 Davee Morgan
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <lk/reg.h>
#include <lk/trace.h>
#include <lk/err.h>
#include <stdio.h>
#include <kernel/thread.h>
#include <lib/cbuf.h>

#include <uart.h>
#include <syscon.h>

static const unsigned int g_uart_baud_rate = 115200;

void dbg_uart_init(void)
{
	syscon_ctrl_hr_power(1);
	uart_init(UART_HPREMOTE, g_uart_baud_rate);
}

void platform_dputc(char c)
{
    if (c == '\n') {
        platform_dputc('\r');
    }

    uart_putc(UART_HPREMOTE, c);
}

int platform_dgetc(char *c, bool wait)
{
    int ret = uart_getc(UART_HPREMOTE);

    if (ret >= 0) {
        *c = ret;
    }

    return ret;
}
