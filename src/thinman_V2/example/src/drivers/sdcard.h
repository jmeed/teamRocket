/*
 * sdcard.h
 *
 *  Created on: Apr 1, 2014
 *      Author: Max Zhao
 */

#ifndef SDCARD_H_
#define SDCARD_H_
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "spi.h"

/* Configuration */
#define SDCARD_SPI_DEVICE SPI_DEVICE_1
#define SDCARD_SPI_SLAVE_PORT 1
#define SDCARD_SPI_SLAVE_PIN 23
#define SDCARD_SPI_FRAME_BITS 8
#define SDCARD_SPI_MAX_WAIT 16
#define SDCARD_WRITE_CRC_FAIL_RETRY 3
#define SDCARD_IDLE_PRE_WAIT_ATTEMPTS 50
#define SDCARD_RESET_ATTEMPTS 20
#define SDCARD_IDLE_WAIT_BYTES 10

/* Error codes */
#define SDCARD_ERROR_OK 0
#define SDCARD_ERROR_GENERIC -1
#define SDCARD_ERROR_TRANSMIT_INTERRUPTED -2
#define SDCARD_ERROR_IDLE_WAIT_TIMEOUT -10
#define SDCARD_ERROR_VOLTAGE_NOT_SUPPORTED -11
#define SDCARD_ERROR_CRC_FAILED -12
#define SDCARD_ERROR_INVALID_SDCARD -13


#define SDCARD_R1_ILLEGAL_CMD 4
#define SDCARD_R1_IDLE 1
#define SDCARD_R1_ERROR_MASK 0b01111100

// Enable recording and batch logging SD card access errors
#define SDCARD_ERROR_LOGGING
// Number of buffered entries in the SD card error log
#define SDCARD_ERROR_LOG_SIZE 0x20

// Initialize SD card resources (mutexes, semaphores)
void SDCardInit();
// Startup SD card. Returns 0 if successful, an error code otherwise
int SDCardStartup();
// Send a normal command to the SD card.  command is the CMD\d+ number (zero for CMD0)
int SDCardSendCommand(uint8_t command, uint32_t param, uint8_t crc, void* buffer, size_t recvSize);
// Send an ACMD to the SD card (prefixed by CMD55)
int SDCardSendACommand(uint8_t acommand, uint32_t param, uint8_t crc, void* buffer, size_t recvSize);
// Blocks until the SD card is ready to accept another command
void SDCardWaitIdle();
// Returns whether the last SDCardStartup was successful
bool SDCardInitialized();

// Read one 512-byte sector from the SD card, given the LBA (sector number)
int SDCardReadSector(uint8_t* buffer, uint32_t sector);
// Read a series of 512-byte sectors from the SD card, given the first LBA and the sector count.
int SDCardDiskRead(uint8_t* buffer, uint32_t sector, size_t count);
// Write a single 512-byte sector to the SD card, given the LBA
int SDCardWriteSector(const uint8_t* buffer, uint32_t sector);
// Write a series of 512-byte sectors to the SD card, given the first LBA and sector count.
int SDCardDiskWrite(const uint8_t* buffer, uint32_t sector, size_t count);

#ifdef SDCARD_ERROR_LOGGING
// Dump all previously buffered SD card access errors to the system log (through logging.h)
void SDCardDumpLogs(void);
#endif

#endif /* SDCARD_H_ */
