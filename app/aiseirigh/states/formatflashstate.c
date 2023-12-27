#include "formatflashstate.h"
#include "mountflashstate.h"

#include <statemachine.h>
#include <primaryscreen.h>
#include <appstate.h>
#include <fferr.h>
#include <psar.h>

#include <kernel/thread.h>

#include <stdlib.h>
#include <string.h>

static FRESULT format_flash(const char *partition, const WorkBuffer *work_buffer)
{
    MKFS_PARM opt;
    opt.fmt = FM_FAT;
    opt.n_fat = 2;
    opt.align = 0;
    opt.au_size = 32 * 512;
    opt.n_root = 512;

    return f_mkfs(partition, &opt, work_buffer->ptr, work_buffer->len);
}

static void on_enter(StateMachine *machine)
{
    static const char *flashes[] = {
        "flash3",
        "flash2",
        "flash1",
        "flash0"
    };

    static const size_t num_flashes = sizeof(flashes)/sizeof(*flashes);

    for (size_t i = 0; i < num_flashes; ++i) {
        char flash[10];
        primary_printf(FG_COLOUR, "formatting %s... ", flashes[i]);

        snprintf(flash, sizeof(flash), "%s:", flashes[i]);
        FRESULT res = format_flash(flash, &machine->app_state->wb1);

        if (res != FR_OK) {
            statemachine_transition_error(  machine,
                                            "error %s formatting %s.",
                                            ff_result_to_str(res),
                                            flashes[i]);
            return;
        }

        primary_puts(FG_COLOUR, "ok\n");
    }

    statemachine_transition(machine, &g_mount_flash_state);
}

State g_format_flash_state = {
    .on_enter = on_enter
};
