#include "mountmsstate.h"
#include "checkupdaterstate.h"

#include <statemachine.h>
#include <primaryscreen.h>
#include <appstate.h>
#include <fferr.h>

#include <ff.h>

static void on_enter(StateMachine *machine)
{
    primary_puts(FG_COLOUR, "mounting memory card... ");

	FRESULT res = f_mount(&machine->app_state->ms, "ms:", 1);

	if (res != FR_OK) {
        statemachine_transition_error(  machine,
                                        "error %s mounting memory card.",
                                        ff_result_to_str(res));
        return;
	}

    primary_puts(FG_COLOUR, "ok\n");
    statemachine_transition(machine, &g_check_updater_state);
}

State g_mount_ms_state = {
    .on_enter = on_enter
};
