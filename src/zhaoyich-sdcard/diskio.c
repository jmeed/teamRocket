/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
#include <stdlib.h>
#include "diskio.h"
#include "sdcard.h"
#include "uart0_os.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/*-----------------------------------------------------------------------*/

#define ATA		0
#define MMC		1
#define USB		2



/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	int result;
	if ((result = SDCardStartup()) != 0) {
		uart0_write_string("SDCard Initialization failed with error ");
		uart0_write_hex32(result);
		uart0_write_string("\r\n");
		return 1;
	}
	uart0_write_string("SDCard successfully initialized\r\n");
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	int result;
	int retries = 0;
	while (count > 0) {
		retry:
		result = SDCardSendCommand(17, sector * 512, 0xff, buff, 512);
		if (result != 0 && retries < 10) {
			if (result == SDCARD_ERROR_CRC_FAILED) {
				uart0_write_string("CRC checksum failed. Retrying.\r\n");
			} else {
				uart0_write_string("SDCard unknown failure. Retrying.\r\n");
			}
			retries ++;
			SDCardWaitIdle();
			goto retry;
		}
		count -= 1;
		sector += 1;
		buff += 512;
		if (result != 0) {
			uart0_write_string("A disk read failed miserably with error code ");
			uart0_write_hex32(result);
			uart0_write_string(" at ");
			uart0_write_hex32(sector * 512);
			uart0_write_string("\r\n");
			exit(90);
			return RES_ERROR;
		}
	}
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/* The FatFs module will issue multiple sector transfer request
/  (count > 1) to the disk I/O layer. The disk function should process
/  the multiple sector transfer properly Do. not translate it into
/  multiple single sector transfers to the media, or the data read/write
/  performance may be drasticaly decreased. */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
    int i;

    for (i =1; i<=count; i++)
    {
    	spi_flash_control_hw(SPI_FLASH_4KBLOCK_ERASE,(sector*i*4096),NULL);  
    }
	return (spi_flash_write(sector*4096, (uint8_t *) buff, count*4096));
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	UINT *result = (UINT *)buff;
	switch (ctrl) {
    case CTRL_SYNC:
    	break;
    case CTRL_POWER:
    	break;
    case CTRL_LOCK:
    	break;
    case CTRL_EJECT:
    	break;
    case GET_SECTOR_COUNT:
    	*result = 262144;
    	break;
    case GET_SECTOR_SIZE:
    	*result = 512;
    	break;
    case GET_BLOCK_SIZE:
    	*result = 1;/*in no.of Sectors */
    	break;
    default:
    	break;
    }
	return 0;

}

