#include "processpsarstate.h"
#include "partitionflashstate.h"

#include <statemachine.h>
#include <primaryscreen.h>
#include <appstate.h>
#include <fferr.h>
#include <psar.h>

#include <kernel/thread.h>

#include <stdlib.h>
#include <string.h>

struct Directories {
    char **list;
    size_t num;
};

static int read_directory(const char *path, void *arg)
{
    struct Directories *dirs = (struct Directories *)arg;

    dirs->list = realloc(dirs->list, (dirs->num + 1) * sizeof(char *));
    dirs->list[dirs->num++] = strdup(path);

    return 0;
}

static int strcmp_qsort_wrapper(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

static void on_enter(StateMachine *machine)
{
    primary_puts(FG_COLOUR, "reading 6.61 psar... ");

    struct AppState *app_state = machine->app_state;
    PsarResult psar_res = psar_init(&app_state->ph, &app_state->wb1, &app_state->wb2);

    if (psar_res != PSAR_RESULT_OK) {
        statemachine_transition_error(  machine,
                                        "error %i initialising psar handle.",
                                        psar_res);
        return;
    }

    psar_res = psar_open(&app_state->ph, app_state->updater_path);

    if (psar_res != PSAR_RESULT_OK) {
        statemachine_transition_error(  machine,
                                        "error %i opening 6.61 psar.",
                                        psar_res);
        return;
    }

    // there doesnt seem to be a sensible way to parse the PSAR and get the 
    // directories in a constructable order. instead, read all the directories
    // into a buffer and then sort them
    struct Directories directories = {
        .num = 0,
        .list = NULL
    };

    psar_read_each_directory(&app_state->ph, read_directory, &directories);
    qsort(directories.list, directories.num, sizeof(char *), strcmp_qsort_wrapper);

    app_state->directories = directories.list;
    app_state->num_directories = directories.num;

    primary_puts(FG_COLOUR, "ok\n");
    statemachine_transition(machine, &g_partition_flash_state);
}

State g_process_psar_state = {
    .on_enter = on_enter
};
