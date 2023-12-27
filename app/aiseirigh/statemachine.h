#pragma once

#include "states/state.h"

#include <lib/evflag.h>
#include <kernel/mutex.h>

struct AppState;

typedef struct StateMachine {
    State *state;
    State *next_state;
    evflag_t evflag;
    mutex_t mutex;
    unsigned int on_press_keys;
    struct AppState *app_state;
} StateMachine;

void statemachine_init(StateMachine *machine, struct AppState *state);
void statemachine_start(StateMachine *machine, State *state);
void statemachine_transition(StateMachine *machine, State *state);
void statemachine_transition_error(StateMachine *machine, const char *why, ...);
