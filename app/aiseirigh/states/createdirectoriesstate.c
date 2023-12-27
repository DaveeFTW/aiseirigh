#include "createdirectoriesstate.h"
#include "flashfilesstate.h"

#include <statemachine.h>
#include <primaryscreen.h>
#include <appstate.h>
#include <fferr.h>
#include <psar.h>

#include <stdlib.h>
#include <string.h>

#define MAX_RETRY	(5)

static void on_enter(StateMachine *machine)
{
    primary_puts(FG_COLOUR, "creating directories... ");

	FRESULT result = FR_OK;

	for (size_t i = 0; i < machine->app_state->num_directories; ++i) {
		FRESULT res = FR_OK;
		printf("creating dir %s\n", machine->app_state->directories[i]);

		for (size_t retry = 0; retry < MAX_RETRY; ++retry) {
			res = f_mkdir(machine->app_state->directories[i]);

			if (res == FR_OK) {
				break;
			}
		}

		if (res != FR_OK) {
			result = res;
		}
	}

    if (result != FR_OK) {
        statemachine_transition_error(  machine,
                                        "error %s creating directories.",
                                        ff_result_to_str(result));
        return;
    }

    primary_puts(FG_COLOUR, "ok\n");
    statemachine_transition(machine, &g_flash_files_state);
}

State g_create_directories_state = {
    .on_enter = on_enter
};
