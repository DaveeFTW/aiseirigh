#pragma once

#include "state.h"

extern State g_error_state;

void error_state_set_message(const char *why);
