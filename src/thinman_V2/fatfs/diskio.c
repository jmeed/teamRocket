/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "drivers/sdcard.h"		/* Example: MMC/SDC control */
#include "drivers/S25FL.h"
#include "logging.h"

/* Definitions of physical drive number for each drive */

// Revert to default to SDCard
#define MMC 1
#define SPIFLASH 0



/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv) {
	case MMC :
		if (SDCardInitialized()) {
			return 0;
		}
		// translate the reslut code here

		return STA_NOINIT;

	case SPIFLASH :
		if (S25FL_initialized()) {
			return 0;
		}
		return STA_NOINIT;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case MMC :
		result = SDCardStartup();
		if (result != SDCARD_ERROR_OK) {
			LOG_ERROR("SDCard Initialization failed with exit code %d", result);
			stat = STA_NOINIT;
		} else {
			stat = 0;
		}

		// translate the reslut code here

		return stat;
	case SPIFLASH:
		if (S25FL_initialized()) {
			return 0;
		}
		return STA_NOINIT;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case MMC :
		// translate the arguments here

		result = SDCardDiskRead(buff, sector, count);
		res = result;
		// translate the reslut code here

		return res;
	case SPIFLASH:
		S25FL_read_sectors(buff, sector, count);
//		LOG_DEBUG("read sect %d + %d", sector, count);
		return 0;
	}

	LOG_ERROR("Read fall through for pdrv %d", pdrv);
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case MMC :
		// translate the arguments here

		result = SDCardDiskWrite(buff, sector, count);
//		if (result != 0) {
//			exit_error(result + 40);
//		}
		res = result;

		// translate the reslut code here

		return res;
	case SPIFLASH:
		S25FL_write_sectors(buff, sector, count);
//		LOG_DEBUG("wrote sect %d + %d", sector, count);
		return 0;
	}

	LOG_ERROR("Write fall through with pdrv %d", pdrv);

	return RES_PARERR;
}
#endif

DWORD get_fattime(void) {
	return 0;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;
	int result;
	(void) result;

	switch (pdrv) {
	case MMC :

		// Process of the command for the MMC/SD card
		if (cmd == CTRL_SYNC) {
			res = 0;
		}
		return res;

	case SPIFLASH :

		// Process of the command for the MMC/SD card
		if (cmd == CTRL_SYNC) {
			res = 0;
		} else if (cmd == GET_SECTOR_COUNT) {
			*((DWORD*)buff) = S25FL_SECTOR_COUNT;
			res = 0;
		} else if (cmd == GET_SECTOR_SIZE) {
			*((DWORD*)buff) = S25FL_SECTOR_SIZE;
			res = 0;
		} else if (cmd == GET_BLOCK_SIZE) {
			*((DWORD*) buff) = S25FL_BLOCK_SIZE / S25FL_SECTOR_SIZE;
			res = 0;
		}

		return res;
	}

	return RES_PARERR;
}
#endif
