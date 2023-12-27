/*
 * Copyright (c) 2015 Travis Geiselbrecht
 * Copyright (c) 2023 Davee Morgan
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <lk/reg.h>
#include <sys/types.h>
#include <kernel/thread.h>
#include <platform.h>
#include <platform/interrupts.h>
#include <platform/debug.h>
#include <platform/timer.h>
#include <platform/psp-allegrex.h>
#include <arch/allegrex.h>

#include <syscon.h>
#include <gpio.h>
#include <sysreg.h>
#include <uart.h>
#include <cpu.h>
#include <emcsm.h>
#include <interrupt.h>
#include <exception.h>
#include <utils.h>
#include <model.h>
#include <led.h>
#include <emcddr.h>
#include <dmacplus.h>
#include <pwm.h>
#include <lcdc.h>
#include <display.h>
#include <kirk.h>
#include <lflash.h>

extern void platform_init_interrupts(void);
extern void platform_init_uart(void);
extern void dbg_uart_init(void);
extern void init_display_framebuffer(void);

void platform_early_init(void)
{
    sysreg_busclk_enable(BUSCLK_APB);
	gpio_init();
	gpio_set_port_mode(GPIO_PORT_LCD_RESET, GPIO_MODE_OUTPUT);
	gpio_clear(GPIO_PORT_LCD_RESET);

    // syscon is already initialised, but there is some state we need to gather
	syscon_init();

    allegrex_init_timer(222 * 1000 * 1000);
    allegrex_enable_irq(2);
}

void platform_init(void)
{
	led_init();
    dbg_uart_init();

	const PspModelIdentity *identity = model_get_identity();

	if (!identity) {
		panic("could not identify PSP model!");
	}

	dprintf(INFO, "running on a %s, %s generation %s (codename %s)\n", identity->model_str, identity->motherboard, identity->gen_str, identity->codename);

	dprintf(INFO, "initialising dmacplus\n");
	dmacplus_init();

	dprintf(INFO, "initialising pwm\n");
	pwm_init();

	dprintf(INFO, "initialising lcdc\n");
	lcdc_init();

	dprintf(INFO, "initialising led\n");
	led_init();

	dprintf(INFO, "initialising display\n");
	display_init();

    dprintf(INFO, "initialising kirk\n");
    kirk_hwreset();
	kirkF((void *)0xBFC00C00);

	dprintf(INFO, "initialising emcsm\n");
   	emcsm_init();

	dprintf(INFO, "initialising lflash\n");
	lflash_init();

    dprintf(INFO, "initialising framebuffer\n");
    init_display_framebuffer();
}
