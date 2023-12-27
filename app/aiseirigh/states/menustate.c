#include "menustate.h"
#include "mountmsstate.h"

#include <statemachine.h>
#include <primaryscreen.h>

#include <gamepad.h>

#include <syscon.h>
#include <model.h>

static void on_key_press(StateMachine *machine, unsigned int keys)
{
    if (IS_KEY_PRESSED(keys, SYSCON_CTRL_CROSS)) {
        statemachine_transition(machine, &g_mount_ms_state);
    }
}

static void on_enter(StateMachine *machine)
{
    primary_clear();

    const PspModelIdentity *identity = model_get_identity();

    primary_puts(FG_COLOUR, "aiseirigh v0.1\nby davee\n\n");
	primary_printf(FG_COLOUR, "running on a %s, %s generation %s\n\n", identity->model_str, identity->motherboard, identity->gen_str);
    primary_puts(FG_COLOUR, "press X to flash\n");
}

State g_menu_state = {
    .on_enter = on_enter,
    .on_key_press = on_key_press
};
