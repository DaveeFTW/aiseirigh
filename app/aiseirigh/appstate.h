#pragma once

#include <model.h>

#include <ff.h>
#include <workbuffer.h>
#include <psar.h>

struct AppState
{
    FATFS ms;
    FATFS f0;
    FATFS f1;
    WorkBuffer wb1;
    WorkBuffer wb2;
    const PspModelIdentity *identity;
    const char *updater_path;
    char **directories;
    size_t num_directories;
    PsarHandle ph;
};

extern struct AppState g_app_state;
