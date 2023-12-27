#include "partitionflashstate.h"
#include "formatflashstate.h"

#include <statemachine.h>
#include <primaryscreen.h>
#include <appstate.h>
#include <fferr.h>

#include <lib/fdisk.h>

#include <emcsm.h>

#include <string.h>

static FdiskResult partition_flash(const PspModelIdentity *identity)
{
	emcsm_set_write_protect(EMCSM_DISABLE_WRITE_PROTECT);

	// TODO: maybe we do this based on flash size rather than model?
	// ultimately, doesn't matter on retail. maybe would affect someone
	// with some custom hw mod
	if (identity->model == PSP_MODEL_01G) {
		return fdisk(0xC000, 0x2000, 0x0800, 0x780);
	} else {
		// on PSP go i saw a 5th partition of size 0x0680. ignoring...
		return fdisk(0x14800, 0x2800, 0x2000, 0x4900);
	}
}

static void on_enter(StateMachine *machine)
{
    primary_puts(FG_COLOUR, "partitioning flash... ");
    FdiskResult fdisk_res = partition_flash(machine->app_state->identity);

    if (fdisk_res != FDISK_RESULT_OK) {
        statemachine_transition_error(  machine,
                                        "error %i partitioning flash\n",
                                        fdisk_res);
        return;
	}
 
    primary_puts(FG_COLOUR, "ok\n");
    statemachine_transition(machine, &g_format_flash_state);
}

State g_partition_flash_state = {
    .on_enter = on_enter
};
