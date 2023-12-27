#pragma once

#include <sys/types.h>

#define IS_KEY_PRESSED(keys, combo)     (((keys) & (combo)) == (combo))

typedef enum {
    GAMEPAD_TRIGGER_ON_PRESS,
    GAMEPAD_TRIGGER_ON_RELEASE,
    GAMEPAD_TRIGGER_ON_HOLD
} GamepadTrigger;

status_t gamepad_read(GamepadTrigger trigger, unsigned int *keys);
