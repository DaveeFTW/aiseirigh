#include "mountmsstate.h"
#include "checkupdaterstate.h"
#include "processpsarstate.h"

#include <statemachine.h>
#include <primaryscreen.h>
#include <appstate.h>
#include <fferr.h>

#include <ff.h>
#include <mbedtls/sha1.h>

#include <string.h>

typedef enum {
    FILE_SHA_RES_OK,
    FILE_SHA_ERR_OPEN,
    FILE_SHA_ERR_READ,
    FILE_SHA_ERR_CLOSE,
} FileHashResult;

#define SHA1_DIGEST_SIZE (20)

static FileHashResult file_sha1_calc(const char *path, const WorkBuffer *buffer, unsigned char *digest)
{
    mbedtls_sha1_context ctx;
    mbedtls_sha1_init(&ctx);
    mbedtls_sha1_starts(&ctx);


    FIL fd;
	FRESULT res = f_open(&fd, path, FA_READ);

	if (res) {
        printf("%s: error %s opening file %s", __FUNCTION__, ff_result_to_str(res), path);
        return FILE_SHA_ERR_OPEN;
	}

	size_t remaining = f_size(&fd);

	while (remaining > 0) {
		size_t copylen = remaining > buffer->len ? buffer->len : remaining;
		size_t bytes_read = 0;
		
		if ((res = f_read(&fd, buffer->ptr, copylen, &bytes_read)) != FR_OK) {
			printf("%s: error %s reading %i bytes from %s. read %i", __FUNCTION__, ff_result_to_str(res), copylen, path, bytes_read);
            return FILE_SHA_ERR_READ;
		}

        mbedtls_sha1_update(&ctx, buffer->ptr, copylen);
		remaining -= copylen;
	}

	if ((res = f_close(&fd))) {
		printf("%s: error %s closing file: %s", __FUNCTION__, ff_result_to_str(res), path);
            return FILE_SHA_ERR_CLOSE;
	}

    mbedtls_sha1_finish(&ctx, digest);
    mbedtls_sha1_free(&ctx);
    return FILE_SHA_RES_OK;
}

static const unsigned char *get_updater_sha1(const PspModelIdentity *identity)
{
	static const unsigned char s_psp_661_update[SHA1_DIGEST_SIZE] = { 0xBC, 0x10, 0xAD, 0x21, 0x14, 0xE1, 0xC3, 0x49, 0xF2, 0x3E, 0xE2, 0xB1, 0x30, 0xD8, 0x2F, 0x7E, 0x86, 0x34, 0x60, 0xAC };
	static const unsigned char s_psp_go_661_update[SHA1_DIGEST_SIZE] = { 0x91, 0xF7, 0x8E, 0xCE, 0xC0, 0x25, 0x70, 0x5F, 0x38, 0xC3, 0x68, 0x80, 0x59, 0xCB, 0x14, 0xFF, 0x42, 0x57, 0xAB, 0x8A };

	if (identity->model != PSP_MODEL_05G) {
		return s_psp_661_update;
	} else {
		return s_psp_go_661_update;
	}
}

static void on_enter(StateMachine *machine)
{
    primary_puts(FG_COLOUR, "checking 6.61 updater... ");
    const PspModelIdentity *identity = machine->app_state->identity;

    unsigned char digest[SHA1_DIGEST_SIZE];
	FileHashResult hash_res = file_sha1_calc(machine->app_state->updater_path,
                                             &machine->app_state->wb1,
                                             digest);

	if (hash_res != FILE_SHA_RES_OK || memcmp(digest, get_updater_sha1(identity), SHA1_DIGEST_SIZE) != 0) {
        statemachine_transition_error(  machine,
                                        "check you have the correct updater in %s\n",
                                        machine->app_state->updater_path);
        return;
	}
 
    primary_puts(FG_COLOUR, "ok\n");
    statemachine_transition(machine, &g_process_psar_state);
}

State g_check_updater_state = {
    .on_enter = on_enter
};
