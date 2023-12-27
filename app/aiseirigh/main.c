/*
 * Copyright (c) 2023 Davee Morgan
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include "screen.h"
#include "textraster.h"
#include "primaryscreen.h"
#include "statemachine.h"
#include "states/initstate.h"
#include "appstate.h"

#include <app.h>
#include <lk/debug.h>
#include <lk/err.h>
#include <kernel/thread.h>

#include <lib/gfx.h>
#include <lib/io.h>
#include <gamepad.h>

#include <dev/display.h>
#include <platform/display.h>

#include <mbedtls/des.h>

#include <ff.h>

#include <syscon.h>
#include <model.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void get_display_surfaces(gfx_surface ***surfaces, size_t *num_surfaces)
{
    assert(surfaces != NULL);
    assert(num_surfaces != NULL);

    size_t num_fbs = 0;
    get_all_framebuffers(NULL, &num_fbs);
    assert(num_fbs == 2);
    
    struct display_framebuffer *fbs = malloc(num_fbs * sizeof(struct display_framebuffer));
    assert(fbs != NULL);

    *num_surfaces = num_fbs;
    *surfaces = malloc(num_fbs * sizeof(gfx_surface *));
    assert(*surfaces != NULL);

    get_all_framebuffers(fbs, &num_fbs);
    assert(num_fbs == 2);

    for (size_t i = 0; i < num_fbs; ++i) {
        (*surfaces)[i] = gfx_create_surface_from_display(&fbs[i]);
    }
}

static int des_decrypt_cbc(void *dst, const void *src, size_t len, const void *key, const void *iv)
{
    mbedtls_des_context ctx;
    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_dec(&ctx, key);
    unsigned char iv_buf[8];
    memcpy(iv_buf, iv, sizeof(iv_buf));
    return mbedtls_des_crypt_cbc(&ctx, MBEDTLS_DES_DECRYPT, len, iv_buf, src, dst);
}

static void setup_screens(Screen **screen1, Screen **screen2)
{
    assert(screen1 != NULL);
    assert(screen2 != NULL);

    gfx_surface **surfaces = NULL;
    size_t num_surfaces = 0;
    get_display_surfaces(&surfaces, &num_surfaces);
    
    // clear all surfaces so they are marked as black
    for (size_t i = 0; i < num_surfaces; ++i) {
        gfx_clear(surfaces[i], BG_COLOUR);
    }

    Screen *primary_screen = malloc(sizeof(Screen));
    Screen *debug_screen = malloc(sizeof(Screen));
    assert(primary_screen != NULL);
    assert(debug_screen != NULL);

    screen_init(primary_screen, surfaces[0]);
    screen_init(debug_screen, surfaces[1]);

    *screen1 = primary_screen;
    *screen2 = debug_screen;
}

static void debug_surface_print(print_callback_t *cb, const char *str, size_t len)
{
    text_rasterise_string((Screen *)cb->context, FG_COLOUR_DBG, BG_COLOUR, str, len);
}

static int display_selection_thread(void *arg)
{
    Screen *screens[2];
    memcpy(screens, arg, sizeof(screens));

    int cur_screen = 0;
    screen_display(screens[cur_screen]);

    while (1) {
        unsigned int keys;
        status_t status = gamepad_read(GAMEPAD_TRIGGER_ON_PRESS, &keys);

        if (status != NO_ERROR) {
            continue;
        }

        if (IS_KEY_PRESSED(keys, SYSCON_CTRL_LTRIGGER)) {
            cur_screen = (cur_screen + 1) % 2;
            screen_display(screens[cur_screen]);
        }
    }

    return 0;
}

static FRESULT mount_memory_card(FATFS *ms)
{
	return f_mount(ms, "ms:", 1);
}



static void aiseirigh_entry(const struct app_descriptor *app, void *args)
{
    // we expect two surfaces, one our primary display, the second a debug
    // output stream. we will be rendering text to these displays so create two
    // screens
    Screen *primary_screen, *debug_screen;
    setup_screens(&primary_screen, &debug_screen);

    // construct a print callback that will receive each byte written to stdout
    // we will capture this and output the text to the debug screen
    print_callback_t cb = {
        .entry = { 0 },
        .print = debug_surface_print,
        .context = (void *)debug_screen
    };

    register_print_callback(&cb);

    Screen *screens[2] = {
        primary_screen,
        debug_screen
    };

    thread_detach_and_resume(thread_create("display_select", &display_selection_thread, screens, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE));

    primary_set_screen(primary_screen);
    printf("hello world from aiseirigh!\n");

    // program start!p

    StateMachine sm;
    statemachine_init(&sm, &g_app_state);
    statemachine_start(&sm, &g_init_state);

    // const PspModelIdentity *identity = model_get_identity();
    // primary_puts(primary_screen, FG_COLOUR, "aiseirigh v0.1\nby davee\n\n");
	// primary_printf(primary_screen, FG_COLOUR, "running on a %s, %s generation %s\n\n", identity->model_str, identity->motherboard, identity->gen_str);
    // primary_puts(primary_screen, FG_COLOUR, "press X to flash\n");

    // while (1) {
    //     unsigned int keys = gamepad_read(GAMEPAD_TRIGGER_ON_PRESS);

    //     printf("primary keys %08X, %i\n", keys, IS_KEY_PRESSED(keys, SYSCON_CTRL_CROSS));

    //     if (IS_KEY_PRESSED(keys, SYSCON_CTRL_CROSS)) {
    //         break;
    //     }
    // }

    // screen_fill(&primary_screen, BG_COLOUR);
    // primary_puts(primary_screen, FG_COLOUR, "aiseirigh v0.1\nby davee\n\n");
	// primary_printf(primary_screen, FG_COLOUR, "running on a %s, %s generation %s\n\n", identity->model_str, identity->motherboard, identity->gen_str);
    // primary_puts(primary_screen, FG_COLOUR, "checking 6.61 updater... ");

    // printf("mounting memory card... ");

    // FATFS *ms = malloc(sizeof(FATFS));
    // assert(ms != NULL);

    // FRESULT res = mount_memory_card(ms);

    // switch (res) {
    //     case FR_OK:
    //         printf("ok\n", sizeof(ms));
    //         break;
    //     default:
    //         panic("failed (%s)\n", ff_result_to_str(res));
    // }

    // unsigned char digest[SHA1_DIGEST_SIZE];
    // FileHashResult hash_res = file_sha1_calc(get_updater_path(identity), &work_buffer1, digest);

    // if (hash_res != FILE_SHA_RES_OK || memcmp(digest, get_updater_sha1(identity), SHA1_DIGEST_SIZE) != 0) {
    //     printf("file_sha1_calc returned %i\n", hash_res);
    //     primary_puts(primary_screen, FG_COLOUR_ERR, "failed\n");
    //     primary_printf(primary_screen, FG_COLOUR, "check you have the correct updater in %s\n", get_updater_path(identity));
    //     enter_error_state();
    // }

    // free(ms);
}

APP_START(aiseirigh)
.entry = aiseirigh_entry,
.flags = APP_FLAG_CUSTOM_STACK_SIZE,
.stack_size = 1 * 1024 * 1024
APP_END

