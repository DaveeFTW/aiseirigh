#include "vsync.h"

#include <lk/init.h>
#include <lk/err.h>
#include <kernel/wait.h>
#include <kernel/thread.h>

#include <interrupt.h>

#include <stdio.h>

static wait_queue_t g_vsync_queue;

void vsync_wait(void)
{
    THREAD_LOCK(state);
    wait_queue_block(&g_vsync_queue, INFINITE_TIME);
    THREAD_UNLOCK(state);
}

static enum IrqHandleStatus vsync_interrupt(void)
{
    THREAD_LOCK(state);
    int num_waiting = wait_queue_wake_all(&g_vsync_queue, false, NO_ERROR);
    THREAD_UNLOCK(state);

    if (num_waiting) {
        return IRQ_HANDLE_RESCHEDULE;
    }

    return IRQ_HANDLE_NO_RESCHEDULE;
}

static void vsync_init_hook(uint level)
{
    wait_queue_init(&g_vsync_queue);
    interrupt_set_handler(IRQ_VSYNC, vsync_interrupt);
}

LK_INIT_HOOK(vsync, &vsync_init_hook, LK_INIT_LEVEL_PLATFORM);
