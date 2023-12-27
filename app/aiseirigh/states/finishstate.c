#include "finishstate.h"

#include <statemachine.h>
#include <primaryscreen.h>
#include <appstate.h>
#include <fferr.h>
#include <psar.h>

#include <kernel/thread.h>

#include <syscon.h>

#include <gamepad.h>

#include <stdlib.h>
#include <string.h>

static void on_key_press(StateMachine *machine, unsigned int keys)
{
    if (IS_KEY_PRESSED(keys, SYSCON_CTRL_CROSS)) {
        syscon_reset_device(1, 1);
    }
}

static void on_enter(StateMachine *machine)
{
	thread_sleep(5*1000);

    primary_puts(FG_COLOUR_SUCCESS, "\nDone. Press X to reboot.\n");
}

State g_finish_state = {
    .on_enter = on_enter,
    .on_key_press = on_key_press
};
