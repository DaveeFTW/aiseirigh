/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include <lflash.h>
#include <mspro.h>
#include <led.h>

#include <string.h>

#define PSPIO_PHYSICAL_DEV_FLASH	(0)
#define PSPIO_PHYSICAL_DEV_MSPRO	(1)

#define PSPIO_PARTITION_FLASH0		(1)
#define PSPIO_PARTITION_FLASH1		(2)
#define PSPIO_PARTITION_FLASH2		(3)
#define PSPIO_PARTITION_FLASH3		(4)

PARTITION VolToPart[FF_VOLUMES] = {
	{ PSPIO_PHYSICAL_DEV_FLASH, PSPIO_PARTITION_FLASH0 },
	{ PSPIO_PHYSICAL_DEV_FLASH, PSPIO_PARTITION_FLASH1 },
	{ PSPIO_PHYSICAL_DEV_FLASH, PSPIO_PARTITION_FLASH2 },
	{ PSPIO_PHYSICAL_DEV_FLASH, PSPIO_PARTITION_FLASH3 },
	{ PSPIO_PHYSICAL_DEV_MSPRO, 0 },
};	/* Volume - Partition mapping table */

#define SECTOR_SIZE					(512)

static LedConfig g_blink_on = {
	.on_time = 2,
	.off_time = 3,
	.blink_time = UINT32_MAX
};

static LedConfig g_blink_off = {
	.on_time = 2,
	.off_time = 3,
	.blink_time = 60
};

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
		case PSPIO_PHYSICAL_DEV_FLASH:
			return 0;
		case PSPIO_PHYSICAL_DEV_MSPRO:
			return 0;
		default:
			// TODO: handle? this shouldn't be possible
	}

	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	static int g_lflash_inited = 0;

	switch (pdrv) {
		case PSPIO_PHYSICAL_DEV_FLASH:
			if (!g_lflash_inited) {
				if (lflash_init() != LFLASH_ERR_NONE) {
					return STA_NOINIT;
				}

				g_lflash_inited = 1;
			}

			return 0;

		case PSPIO_PHYSICAL_DEV_MSPRO:
			if (mspro_init() < 0) {
				return STA_NOINIT;
			}

			return 0;

		default:
			// TODO: handle? this shouldn't be possible
	}

	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	switch (pdrv) {
		case PSPIO_PHYSICAL_DEV_FLASH:
			for (unsigned int i = 0; i < count; ++i) {
				led_set_mode(LED_WLAN, LED_MODE_BLINK, &g_blink_on);
				enum LflashError result = lflash_read_sector(sector + i, buff + (i * SECTOR_SIZE));
				led_set_mode(LED_WLAN, LED_MODE_BLINK, &g_blink_off);

				if (result != LFLASH_ERR_NONE) {
					return RES_ERROR;
				}
			}

			return RES_OK;

		case PSPIO_PHYSICAL_DEV_MSPRO:
			for (unsigned int i = 0; i < count; ++i) {
				led_set_mode(LED_MEMORY_STICK, LED_MODE_BLINK, &g_blink_on);
				int result = mspro_read_sector(sector + i, buff + (i * SECTOR_SIZE));
				led_set_mode(LED_MEMORY_STICK, LED_MODE_BLINK, &g_blink_off);

				if (result < 0) {
					return RES_ERROR;
				}
			}

			return RES_OK;

		default:
			// TODO: handle? this shouldn't be possible
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	switch (pdrv) {
		case PSPIO_PHYSICAL_DEV_FLASH:
			while (count > 0) {
				size_t num_written = 0;
				led_set_mode(LED_WLAN, LED_MODE_BLINK, &g_blink_on);
				enum LflashError result = lflash_write_sectors(sector, buff, count, &num_written);
				led_set_mode(LED_WLAN, LED_MODE_BLINK, &g_blink_off);

 				if (result != LFLASH_ERR_NONE) {
					return RES_ERROR;
				}

				sector += num_written;
				buff += num_written * SECTOR_SIZE;
				count -= num_written;
			}

			return RES_OK;

		case PSPIO_PHYSICAL_DEV_MSPRO:
			for (size_t sector_offset = 0; sector_offset < count; ++sector_offset) {
				led_set_mode(LED_MEMORY_STICK, LED_MODE_BLINK, &g_blink_on);
				int result = mspro_write_sector(sector + sector_offset, buff + sector_offset * SECTOR_SIZE);
				led_set_mode(LED_MEMORY_STICK, LED_MODE_BLINK, &g_blink_off);

				if (result < 0) {
					return RES_ERROR;
				}
			}

			return RES_OK;

		default:
			// TODO: handle? this shouldn't be possible
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (pdrv) {
		case PSPIO_PHYSICAL_DEV_FLASH:
			switch (cmd) {
				case CTRL_SYNC:
					led_set_mode(LED_WLAN, LED_MODE_BLINK, &g_blink_on);
					enum LflashError result = lflash_sync();
					led_set_mode(LED_WLAN, LED_MODE_BLINK, &g_blink_off);
					
					if (result != LFLASH_ERR_NONE) {
						return RES_ERROR;
					}

					return RES_OK;
				
				case GET_SECTOR_COUNT:
					*(LBA_t *)buff = lflash_get_size();
					return RES_OK;

				case GET_SECTOR_SIZE:
					*(WORD *)buff = SECTOR_SIZE;
					return RES_OK;

				case GET_BLOCK_SIZE:
					*(DWORD *)buff = lflash_get_block_size();
					return RES_OK;
			}

			break;

		case PSPIO_PHYSICAL_DEV_MSPRO:
			switch (cmd) {
				case CTRL_SYNC:
					// nothing to do here
					return RES_OK;
				
				// TODO?
				case GET_SECTOR_COUNT:
					return RES_PARERR;

				case GET_SECTOR_SIZE:
					*(WORD *)buff = SECTOR_SIZE;
					return RES_OK;

				// TODO?
				case GET_BLOCK_SIZE:
					return RES_PARERR;
			}

			break;

		default:
			// TODO: handle? this shouldn't be possible
	}

	return RES_PARERR;
}
