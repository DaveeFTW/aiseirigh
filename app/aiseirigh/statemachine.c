#include "statemachine.h"
#include "errextension.h"

#include "states/errorstate.h"

#include <kernel/thread.h>
#include <gamepad.h>
#include <lk/err.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define SM_TRANSITION   (1 << 0)
#define SM_ON_PRESS     (1 << 1)

static int on_press(void *arg)
{
    StateMachine *machine = (StateMachine *)arg;

    while (1) {
        unsigned int keys;
        status_t status = gamepad_read(GAMEPAD_TRIGGER_ON_PRESS, &keys);

        if (status != NO_ERROR) {
            break;
        }

        mutex_acquire(&machine->mutex);
        machine->on_press_keys = keys;
        mutex_release(&machine->mutex);
        evflag_raise(&machine->evflag, SM_ON_PRESS, false);
    }

    return 0;
}

static void perform_state_transition(StateMachine *machine)
{
    mutex_acquire(&machine->mutex);
    assert(machine->next_state != NULL);
    State *next_state = machine->next_state;
    State *current_state = machine->state;
    mutex_release(&machine->mutex);

    // if there is a current state and that state has an exit notification
    // execute it prior to transitioning out to the next state
    if (current_state && current_state->on_exit) {
        current_state->on_exit(machine);
    }

    mutex_acquire(&machine->mutex);
    machine->state = next_state;
    machine->next_state = NULL;
    mutex_release(&machine->mutex);

    if (next_state->on_enter) {
        next_state->on_enter(machine);
    }
}

void statemachine_init(StateMachine *machine, struct AppState *app_state)
{
    evflag_init(&machine->evflag, 0);
    mutex_init(&machine->mutex);
    machine->on_press_keys = 0;
    machine->state = machine->next_state = NULL;
    machine->app_state = app_state;
    thread_detach_and_resume(thread_create("on_press", &on_press, machine, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE));
}

void statemachine_start(StateMachine *machine, State *state)
{
    statemachine_transition(machine, state);

    while (1) {
        uint bits = 0;
        status_t res = evflag_wait(&machine->evflag, 
                                   SM_TRANSITION | SM_ON_PRESS,
                                   EVENT_FLAG_WAIT_MODE_OR | EVENT_FLAG_AUTOCLEAR,
                                   &bits);

        if (res != NO_ERROR) {
            continue;
        }

        if (bits & SM_TRANSITION) {
            perform_state_transition(machine);
            mutex_acquire(&machine->mutex);
            machine->on_press_keys = 0;
            mutex_release(&machine->mutex);
        } else if (bits & SM_ON_PRESS) {
            if (machine->state && machine->state->on_key_press) {
                machine->state->on_key_press(machine, machine->on_press_keys);
            }
            mutex_acquire(&machine->mutex);
            machine->on_press_keys = 0;
            mutex_release(&machine->mutex);
        }
    }
}

void statemachine_transition(StateMachine *machine, State *state)
{
    mutex_acquire(&machine->mutex);
    assert(machine->next_state == NULL);
    machine->next_state = state;
    mutex_release(&machine->mutex);
    evflag_raise(&machine->evflag, SM_TRANSITION, false);
}

void statemachine_transition_error(StateMachine *machine, const char *why, ...)
{
    char out[1];
    va_list ap;

    va_start(ap, why);
    size_t len = vsnprintf(out, 0, why, ap);
    va_end(ap);

    char *msg = malloc(len + 1);
    assert(msg != NULL);

    va_start(ap, why);
    vsnprintf(msg, len + 1, why, ap);
    va_end(ap);

    error_state_set_message(msg);
    free(msg);

    statemachine_transition(machine, &g_error_state);
}
