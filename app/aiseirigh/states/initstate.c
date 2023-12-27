#include "mountmsstate.h"
#include "menustate.h"

#include <statemachine.h>
#include <primaryscreen.h>
#include <appstate.h>
#include <workbuffer.h>

#include <assert.h>

#define WORK_BUFFER_SIZE    (12 * 1024 * 1024)

static const char *get_updater_path(const PspModelIdentity *identity)
{
    if (identity->model == PSP_MODEL_05G) {
        return "ms:/661GO.PBP";
    } else {
        return "ms:/661.PBP";
    }
}

static void on_enter(StateMachine *machine)
{
    printf("setting identity\n");
    machine->app_state->identity = model_get_identity();

    printf("setting updater path\n");
    machine->app_state->updater_path = get_updater_path(machine->app_state->identity);

    printf("allocating work buffers\n");
    machine->app_state->wb1 = work_buffer_create(WORK_BUFFER_SIZE, 64);
    machine->app_state->wb2 = work_buffer_create(WORK_BUFFER_SIZE, 64);
    assert(machine->app_state->wb1.ptr != NULL);
    assert(machine->app_state->wb2.ptr != NULL);
    statemachine_transition(machine, &g_menu_state);
}

State g_init_state = {
    .on_enter = on_enter
};
