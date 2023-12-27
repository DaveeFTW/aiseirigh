#include "formatflashstate.h"
#include "createdirectoriesstate.h"

#include <statemachine.h>
#include <primaryscreen.h>
#include <appstate.h>
#include <fferr.h>
#include <psar.h>

#include <stdlib.h>
#include <string.h>

static void on_enter(StateMachine *machine)
{
    primary_puts(FG_COLOUR, "mounting flash0... ");

    FRESULT res = f_mount(&machine->app_state->f0, "flash0:", 1);

    if (res != FR_OK) {
        statemachine_transition_error(  machine,
                                        "error %s mounting flash0.",
                                        ff_result_to_str(res));
        return;
    }

    primary_puts(FG_COLOUR, "ok\n");
    primary_puts(FG_COLOUR, "mounting flash1... ");

    res = f_mount(&machine->app_state->f1, "flash1:", 1);

    if (res != FR_OK) {
        statemachine_transition_error(  machine,
                                        "error %s mounting flash1.",
                                        ff_result_to_str(res));
        return;
    }

    primary_puts(FG_COLOUR, "ok\n");
    statemachine_transition(machine, &g_create_directories_state);
}

State g_mount_flash_state = {
    .on_enter = on_enter
};
