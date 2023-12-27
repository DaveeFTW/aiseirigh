/*
 * Copyright (c) 2015 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <lk/trace.h>
#include <lk/debug.h>
#include <stdint.h>
#include <lk/bits.h>
#include <arch.h>
#include <platform.h>
#include <kernel/thread.h>

#include <sysreg.h>
#include <gpio.h>
#include <exception.h>
#include <interrupt.h>

#include <string.h>

#define LOCAL_TRACE 0

extern EXCEPTION_HANDLER exception_vectors;

void arch_early_init(void)
{
    LTRACE;

    sysreg_busclk_enable(BUSCLK_APB);
	gpio_init();
	gpio_set_port_mode(GPIO_PORT_LCD_RESET, GPIO_MODE_OUTPUT);
	gpio_clear(GPIO_PORT_LCD_RESET);

    // initialize the exception handlers to their defaults
    exception_init();

    // by default we capture all exceptions using this handler
    exception_register_default_handler(&exception_vectors);


    // initialize the interrupt service
    interrupt_init();

    // register our rescheduler hook
    interrupt_set_reschedule_hook(thread_preempt);

    // external interrupts are cascaded upon IRQ2
    allegrex_enable_irq(2);

    // LK expects interrupts to be disabled at this stage
    arch_disable_ints();

    LTRACE_EXIT;
}

void arch_init(void)
{
    LTRACE;

    printf("MIPS registers:\n");
    printf("\tPRId    0x%x\n", allegrex_read_c0_prid());
    printf("\tconfig  0x%x\n", allegrex_read_c0_config());
    printf("\tstatus  0x%x\n", allegrex_read_c0_status());
    printf("\tebase   0x%x\n", allegrex_read_c0_ebase());
    printf("\tcount   0x%x\n", allegrex_read_c0_count());
    printf("\tcompare 0x%x\n", allegrex_read_c0_compare());

    __asm__ volatile("syscall");

    LTRACE_EXIT;
}

void arch_idle(void)
{
    // halt processor
	asm volatile (
		"mfc0 $v0, $25\n"
		"cache 0xA, 0($v0)\n" // ensure that ebase is in the instruction cache
		".word 0x70000000\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"sync\n"
		"nop\n" ::: "v0"
	);
}

void arch_chain_load(void *entry, ulong arg0, ulong arg1, ulong arg2, ulong arg3)
{
    PANIC_UNIMPLEMENTED;
}

void allegrex_enable_irq(uint num)
{
    uint32_t mask = cpu_suspend_interrupts();
    uint32_t temp = allegrex_read_c0_status();

    // IRQ number should be in the range of 0 to 7 inclusive
    temp |= (1 << (num + 8));

    allegrex_write_c0_status(temp);
    cpu_resume_interrupts(mask);
}

void allegrex_disable_irq(uint num)
{
    uint32_t mask = cpu_suspend_interrupts();
    uint32_t temp = allegrex_read_c0_status();

    // IRQ number should be in the range of 0 to 7 inclusive
    temp &= ~(1 << (num + 8));

    allegrex_write_c0_status(temp);
    cpu_resume_interrupts(mask);
}

void arch_disable_cache(uint flags)
{ 
    PANIC_UNIMPLEMENTED;
}

void arch_enable_cache(uint flags)
{
    PANIC_UNIMPLEMENTED;
}

void arch_clean_cache_range(addr_t start, size_t len)
{
    // TODO: we currently affect the whole cache
	cpu_dcache_wb_all();
}

void arch_clean_invalidate_cache_range(addr_t start, size_t len)
{
    // TODO: we currently affect the whole cache
	cpu_dcache_wb_inv_all();
}

void arch_invalidate_cache_range(addr_t start, size_t len)
{
    // TODO: we currently affect the whole cache
    // cpu_dcache_inv_all();
}

void arch_sync_cache_range(addr_t start, size_t len)
{
    // TODO: we currently affect the whole cache
    arch_clean_cache_range(start, len);
	cpu_icache_inv_all();
}
