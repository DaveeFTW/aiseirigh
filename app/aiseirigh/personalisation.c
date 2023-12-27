#include "personalisation.h"
#include "crypt.h"
#include "log.h"

#include <stddef.h>
#include <string.h>

static const unsigned char g_personalisation_xor_before[0x10] =
{
	0x71, 0xF6, 0xA8, 0x31, 0x1E, 0xE0, 0xFF, 0x1E,
	0x50, 0xBA, 0x6C, 0xD2, 0x98, 0x2D, 0xD6, 0x2D
};

static const unsigned char g_personalisation_xor_after[0x10] =
{
	0xAA, 0x85, 0x4D, 0xB0, 0xFF, 0xCA, 0x47, 0xEB,
	0x38, 0x7F, 0xD7, 0xE4, 0x3D, 0x62, 0xB0, 0x10
};

PersonalisationResult personalise(unsigned char *data)
{
	unsigned char tmp[0xD0];

	memcpy(tmp, data + 0x110, 0x40);
	memcpy(tmp + 0x40, data + 0x80, 0x90);
	
	for (size_t i = 0; i < sizeof(tmp); ++i)
	{
		tmp[i] ^= g_personalisation_xor_before[i % sizeof(g_personalisation_xor_before)];
	}

	int res = kirk5_encrypt(tmp, tmp, sizeof(tmp));

	if (res != 0)
	{
		LOG("%s: error %i applying personalisation", __FUNCTION__, res);
		return PERSONALISATION_ERR_ENCRYPT;
	}

	for (size_t i = 0; i < sizeof(tmp); ++i)
	{
		tmp[i] ^= g_personalisation_xor_after[i % sizeof(g_personalisation_xor_after)];
	}

	memcpy(data + 0x80, tmp, sizeof(tmp));
	return PERSONALISATION_RESULT_OK;
}
