#include "mountmsstate.h"
#include <statemachine.h>
#include <primaryscreen.h>

#include <string.h>

static const char *g_why = NULL;

void error_state_set_message(const char *why)
{
    g_why = strdup(why);
}

static void on_enter(StateMachine *machine)
{
    primary_puts(FG_COLOUR_ERR, "failed\n");

    if (g_why) {
        primary_puts(FG_COLOUR_ERR, g_why);
    }
}

State g_error_state = {
    .on_enter = on_enter
};
