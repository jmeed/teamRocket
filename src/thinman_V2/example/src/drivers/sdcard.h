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
#define SDCARD_RESET_ATTEMPTS 20
#define SDCARD_ERROR_OK 0
#define SDCARD_ERROR_GENERIC -1
#define SDCARD_ERROR_TRANSMIT_INTERRUPTED -2
#define SDCARD_ERROR_IDLE_WAIT_TIMEOUT -10
#define SDCARD_ERROR_VOLTAGE_NOT_SUPPORTED -11
#define SDCARD_ERROR_CRC_FAILED -12
#define SDCARD_ERROR_INVALID_SDCARD -13
#define SDCARD_WRITE_CRC_FAIL_RETRY 3
#define SDCARD_IDLE_PRE_WAIT_ATTEMPTS 50

#define SDCARD_IDLE_WAIT_BYTES 10

#define SDCARD_R1_ILLEGAL_CMD 4
#define SDCARD_R1_IDLE 1
#define SDCARD_R1_ERROR_MASK 0b01111100

#define SDCARD_ERROR_LOGGING
#define SDCARD_ERROR_LOG_SIZE 0x20

int SDCardSendCommand(uint8_t command, uint32_t param, uint8_t crc, void* buffer, size_t recvSize);
int SDCardSendACommand(uint8_t acommand, uint32_t param, uint8_t crc, void* buffer, size_t recvSize);
void SDCardInit();
int SDCardStartup();
void SDCardWaitIdle();
bool SDCardInitialized();

int SDCardReadSector(uint8_t* buffer, uint32_t sector);
int SDCardDiskRead(uint8_t* buffer, uint32_t sector, size_t count);
int SDCardWriteSector(const uint8_t* buffer, uint32_t sector);
int SDCardDiskWrite(const uint8_t* buffer, uint32_t sector, size_t count);

#ifdef SDCARD_ERROR_LOGGING
void SDCardDumpLogs(void);
#endif

#endif /* SDCARD_H_ */
