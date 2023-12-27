#include "gamepad.h"

#include <lk/init.h>
#include <lk/err.h>
#include <kernel/thread.h>
#include <kernel/mutex.h>
#include <kernel/wait.h>

#include <vsync.h>

#include <syscon.h>
#include <interrupt.h>

#define KEY_MASK    (SYSCON_CTRL_UP | SYSCON_CTRL_RIGHT | SYSCON_CTRL_DOWN \
                    | SYSCON_CTRL_LEFT | SYSCON_CTRL_TRIANGLE \
                    | SYSCON_CTRL_CIRCLE | SYSCON_CTRL_CROSS \
                    | SYSCON_CTRL_SQUARE | SYSCON_CTRL_SELECT \
                    | SYSCON_CTRL_LTRIGGER | SYSCON_CTRL_RTRIGGER \
                    | SYSCON_CTRL_START | SYSCON_CTRL_HOME \
                    | SYSCON_CTRL_HOLD | SYSCON_CTRL_WLAN \
                    | SYSCON_CTRL_VOL_UP | SYSCON_CTRL_VOL_DOWN \
                    | SYSCON_CTRL_LCD | SYSCON_CTRL_NOTE )

#define TO_KEYS(x)  ((~(x)) & KEY_MASK)

typedef struct {
    unsigned int keys;
    unsigned int prev_keys;
    mutex_t gamepad_mutex;
    unsigned int on_press_keys, on_release_keys, on_hold_keys;
    wait_queue_t wait_on_press, wait_on_release, wait_on_hold;
} GamepadContext;

static GamepadContext g_gamepad_ctx;

static status_t wait_queue(wait_queue_t *wait)
{
    THREAD_LOCK(state);
    status_t res = wait_queue_block(wait, INFINITE_TIME);
    THREAD_UNLOCK(state);
    return res;
}

status_t gamepad_read(GamepadTrigger trigger, unsigned int *keys)
{
    status_t status = NO_ERROR;

    switch (trigger) {
        case GAMEPAD_TRIGGER_ON_PRESS:
            status = wait_queue(&g_gamepad_ctx.wait_on_press);

            if (status != NO_ERROR) {
                return status;
            }

            mutex_acquire(&g_gamepad_ctx.gamepad_mutex);
            *keys = g_gamepad_ctx.on_press_keys;
            mutex_release(&g_gamepad_ctx.gamepad_mutex);
            break;
        case GAMEPAD_TRIGGER_ON_RELEASE:
            status = wait_queue(&g_gamepad_ctx.wait_on_release);

            if (status != NO_ERROR) {
                return status;
            }

            mutex_acquire(&g_gamepad_ctx.gamepad_mutex);
            *keys = g_gamepad_ctx.on_release_keys;
            mutex_release(&g_gamepad_ctx.gamepad_mutex);
            break;
        case GAMEPAD_TRIGGER_ON_HOLD:
            status = wait_queue(&g_gamepad_ctx.wait_on_hold);

            if (status != NO_ERROR) {
                return status;
            }

            mutex_acquire(&g_gamepad_ctx.gamepad_mutex);
            *keys = g_gamepad_ctx.on_hold_keys;
            mutex_release(&g_gamepad_ctx.gamepad_mutex);
            break;
        default:
            *keys = 0;
    }

    return status;
}

static int gamepad_poll(void *arg)
{
    mutex_init(&g_gamepad_ctx.gamepad_mutex);
    wait_queue_init(&g_gamepad_ctx.wait_on_press);
    wait_queue_init(&g_gamepad_ctx.wait_on_release);
    wait_queue_init(&g_gamepad_ctx.wait_on_hold);
    g_gamepad_ctx.prev_keys = g_gamepad_ctx.keys = ~0;

    while (1) {
        unsigned int keys = ~0;
        if (syscon_get_digital_key(&keys) >= 0) {
            mutex_acquire(&g_gamepad_ctx.gamepad_mutex);

            g_gamepad_ctx.keys = keys;
            g_gamepad_ctx.on_press_keys = TO_KEYS(keys | ~g_gamepad_ctx.prev_keys);
            g_gamepad_ctx.on_release_keys = TO_KEYS(~keys | g_gamepad_ctx.prev_keys);
            g_gamepad_ctx.on_hold_keys = TO_KEYS(keys | g_gamepad_ctx.prev_keys);
            g_gamepad_ctx.prev_keys = keys;

            if (g_gamepad_ctx.on_press_keys) {
                THREAD_LOCK(state);
                wait_queue_wake_all(&g_gamepad_ctx.wait_on_press, false, NO_ERROR);
                THREAD_UNLOCK(state);
            }

            if (g_gamepad_ctx.on_release_keys) {
                THREAD_LOCK(state);
                wait_queue_wake_all(&g_gamepad_ctx.wait_on_release, false, NO_ERROR);
                THREAD_UNLOCK(state);
            }

            if (g_gamepad_ctx.on_hold_keys) {
                THREAD_LOCK(state);
                wait_queue_wake_all(&g_gamepad_ctx.wait_on_hold, false, NO_ERROR);
                THREAD_UNLOCK(state);
            }

            mutex_release(&g_gamepad_ctx.gamepad_mutex);
        }

        vsync_wait();
    }

    return 0;
}

static void gamepad_init_hook(uint level)
{
    thread_detach_and_resume(thread_create("gamepad_poll", &gamepad_poll, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE));
}

LK_INIT_HOOK(gamepad, &gamepad_init_hook, LK_INIT_LEVEL_TARGET);
