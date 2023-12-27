/*
 * Copyright (c) 2008-2014 Travis Geiselbrecht
 * Copyright (c) 2023 Davee Morgan
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <kernel/thread.h>
#include <lk/compiler.h>
#include <stdbool.h>
#include <sys/types.h>

__BEGIN_CDECLS

#define EVENT_FLAG_MAGIC (0x6576666C)  // "evfl"


typedef struct evflag {
    int magic;
    uint bits;
    wait_queue_t wait;
} evflag_t;

#define EVENT_FLAG_WAIT_MODE_BIT    (1)

#define EVENT_FLAG_AUTOCLEAR        (1 << 0)
#define EVENT_FLAG_WAIT_MODE_OR     (0 << EVENT_FLAG_WAIT_MODE_BIT)
#define EVENT_FLAG_WAIT_MODE_AND    (1 << EVENT_FLAG_WAIT_MODE_BIT)


#define EVENT_FLAG_INITIAL_VALUE(e, initial) \
{ \
    .magic = EVENT_FLAG_MAGIC, \
    .bits = initial, \
    .wait = WAIT_QUEUE_INITIAL_VALUE((e).wait), \
}

/* Rules for Events:
 * - Events may be raised from interrupt context *but* the reschedule
 *   parameter must be false in that case.
 * - Events may not be waited upon from interrupt context.
*/

void evflag_init(evflag_t *, uint initial);
void evflag_destroy(evflag_t *);
status_t evflag_wait_timeout(evflag_t *e, uint pattern, uint flags, uint *out_bits, lk_time_t timeout);
status_t evflag_raise(evflag_t *e, uint bits, bool reschedule);

static inline bool evflag_initialized(evflag_t *e)
{
    return e->magic == EVENT_FLAG_MAGIC;
}

static inline status_t evflag_wait(evflag_t *e, uint pattern, uint flags, uint *out_bits)
{
    return evflag_wait_timeout(e, pattern, flags, out_bits, INFINITE_TIME);
}

__END_CDECLS
