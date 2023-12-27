#include "flashfilesstate.h"
#include "finishstate.h"

#include <statemachine.h>
#include <primaryscreen.h>
#include <appstate.h>
#include <fferr.h>
#include <psar.h>
#include <personalisation.h>
#include <ipl.h>

#include <emcsm.h>

#include <stdlib.h>
#include <string.h>

static int flash_file(const char *path, unsigned char *data, size_t len, const FileProperties *prop)
{
	FIL fd;
	FRESULT res = f_open(&fd, path, FA_WRITE | FA_CREATE_ALWAYS);

	if (res != FR_OK) {
		printf("%s: error %i opening %s for writing\n", __FUNCTION__, res, path);
		return 1;
	}

	if (prop->personalisation == PSAR_FILE_REQ_PERSONALISED) {
		PersonalisationResult pres = personalise(data);

		if (pres != PERSONALISATION_RESULT_OK) {
			printf("%s: error %i personalisating %s for writing\n", __FUNCTION__, pres, path);
			return 1;
		}
	}

	size_t bytes_written = 0;
	if ((res = f_write(&fd, data, len, &bytes_written)) != FR_OK) {
		printf("%s: error %i writing %i bytes to %s. written %i\n", __FUNCTION__, res, len, path, bytes_written);
		return 1;
	}

	if ((res = f_close(&fd))) {
		printf("%s: error %i closing file: %s\n", __FUNCTION__, res, path);
		return 1;
	}

	printf("flashed %s\n", path);
	return 0;
}

static int on_read_psar_file(const char *path, unsigned char *data, size_t len, const FileProperties *prop)
{
    if (prop->file_type == PSAR_FILE_TYPE_IPL) {
        emcsm_set_scramble(0);
        clear_ipl();
        int res = write_ipl(data, len);
        return 0;
    } else {
        return flash_file(path, data, len, prop);
    }
}

static void on_enter(StateMachine *machine)
{
    primary_puts(FG_COLOUR, "flashing files... ");

    PsarResult res = psar_read_each_file(&machine->app_state->ph, on_read_psar_file);

    if (res != PSAR_RESULT_OK) {
        statemachine_transition_error(  machine,
                                        "error %i flashing files.",
                                        res);
        return;
    }

    primary_puts(FG_COLOUR, "ok\n");
    statemachine_transition(machine, &g_finish_state);
}

State g_flash_files_state = {
    .on_enter = on_enter
};
