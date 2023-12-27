/*
 * Copyright (c) 2008-2014 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */

/**
 * @file
 * @brief  Event wait and signal functions for threads.
 * @defgroup event Events
 *
 * An event is a subclass of a wait queue.
 *
 * Threads wait for events, with optional timeouts.
 *
 * Events are "signaled", releasing waiting threads to continue.
 * Signals may be one-shot signals (EVENT_FLAG_AUTOUNSIGNAL), in which
 * case one signal releases only one thread, at which point it is
 * automatically cleared. Otherwise, signals release all waiting threads
 * to continue immediately until the signal is manually cleared with
 * event_unsignal().
 *
 * @{
 */

#include <lib/evflag.h>

#include <assert.h>
#include <kernel/thread.h>
#include <lk/debug.h>
#include <lk/err.h>

#include <stdlib.h>

#define EVENT_FLAG_NUM_PREALLOC_WAITERS 1

static int check_pattern(uint bits, uint pattern, uint flags)
{
    switch (flags & (1 << EVENT_FLAG_WAIT_MODE_BIT)) {
        case EVENT_FLAG_WAIT_MODE_AND:
            return (bits & pattern) == bits;

        case EVENT_FLAG_WAIT_MODE_OR:
            return (bits & pattern) != 0;
    }

    return 0;
}

static uint clear_pattern(uint bits, uint pattern, uint flags)
{
    // for both modes AND/OR we expect the same outcome: all pattern bits are
    // removed from the event flag pattern
    return bits & ~pattern;
}

/**
 * @brief  Initialize an event object
 *
 * @param e        Event object to initialize
 * @param initial  Initial value for "signaled" state
 * @param flags    0 or EVENT_FLAG_AUTOUNSIGNAL
 */
void evflag_init(evflag_t *e, uint initial) {
    *e = (evflag_t)EVENT_FLAG_INITIAL_VALUE(*e, initial);
}

/**
 * @brief  Destroy an event object.
 *
 * Event's resources are freed and it may no longer be
 * used until event_init() is called again.  Any threads
 * still waiting on the event will be resumed.
 *
 * @param e        Event object to initialize
 */
void evflag_destroy(evflag_t *e) {
    DEBUG_ASSERT(e->magic == EVENT_FLAG_MAGIC);

    THREAD_LOCK(state);

    e->magic = 0;
    e->bits = 0;
    wait_queue_destroy(&e->wait, true);

    THREAD_UNLOCK(state);
}

/**
 * @brief  Wait for event to be signaled
 *
 * If the event has already been signaled, this function
 * returns immediately.  Otherwise, the current thread
 * goes to sleep until the event object is signaled,
 * the timeout is reached, or the event object is destroyed
 * by another thread.
 *
 * @param e        Event object
 * @param timeout  Timeout value, in ms
 *
 * @return  0 on success, ERR_TIMED_OUT on timeout,
 *         other values on other errors.
 */
status_t evflag_wait_timeout(evflag_t *e, uint pattern, uint flags, uint *out_bits, lk_time_t timeout)
{
    status_t ret = NO_ERROR;

    DEBUG_ASSERT(e->magic == EVENT_FLAG_MAGIC);

    THREAD_LOCK(state);

    while (ret == NO_ERROR) {
        // check if we have a match already and exit immediately without waiting
        if (check_pattern(e->bits, pattern, flags)) {
            if (out_bits) {
                *out_bits = e->bits;
            }

            if (flags & EVENT_FLAG_AUTOCLEAR) {
                e->bits = clear_pattern(e->bits, pattern, flags);
            }

            break;
        } else {
            // wait here until we get a match
            ret = wait_queue_block(&e->wait, timeout);
        }
    }

    THREAD_UNLOCK(state);

    return ret;
}

/**
 * @brief  Signal an event
 *
 * Signals an event.  If EVENT_FLAG_AUTOUNSIGNAL is set in the event
 * object's flags, only one waiting thread is allowed to proceed.  Otherwise,
 * all waiting threads are allowed to proceed until such time as
 * event_unsignal() is called.
 *
 * @param e           Event object
 * @param reschedule  If true, waiting thread(s) are executed immediately,
 *                    and the current thread resumes only after the
 *                    waiting threads have been satisfied. If false,
 *                    waiting threads are placed at the end of the run
 *                    queue.
 *
 * @return  Returns NO_ERROR on success.
 */
status_t evflag_raise(evflag_t *e, uint bits, bool reschedule)
{
    DEBUG_ASSERT(e->magic == EVENT_FLAG_MAGIC);

    THREAD_LOCK(state);

    e->bits |= bits;
    wait_queue_wake_all(&e->wait, reschedule, NO_ERROR);

    THREAD_UNLOCK(state);

    return NO_ERROR;
}
