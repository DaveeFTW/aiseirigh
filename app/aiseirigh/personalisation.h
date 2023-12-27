#pragma once

typedef enum {
    PERSONALISATION_RESULT_OK,
    PERSONALISATION_ERR_ENCRYPT
} PersonalisationResult;

PersonalisationResult personalise(unsigned char *data);
