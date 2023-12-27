#pragma once

struct StateMachine;

typedef struct State {
    void (* on_enter)(struct StateMachine *machine);
    void (* on_exit)(struct StateMachine *machine);
    void (* on_key_press)(struct StateMachine *machine, unsigned int keys);
    void (* on_key_release)(struct StateMachine *machine, unsigned int keys);
} State;
