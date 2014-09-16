/*
 * sdcard.h
 *
 *  Created on: Apr 1, 2014
 *      Author: Max Zhao
 */

#ifndef SDCARD_H_
#define SDCARD_H_
#include <stddef.h>
#include <mss_spi.h>
#include <mss_gpio.h>
#include <stdint.h>
#include <stdbool.h>

/* Configuration */
#define SDCARD_SPI_DEVICE &g_mss_spi1
#define SDCARD_SPI_SLAVE MSS_SPI_SLAVE_0
#define SDCARD_SPI_SLAVE_PIN MSS_GPIO_0

#define SDCARD_SPI_FRAME_BITS 8
#define SDCARD_SPI_MAX_WAIT 16
#define SDCARD_RESET_ATTEMPTS 20
#define SDCARD_ERROR_OK 0
#define SDCARD_ERROR_GENERIC -1
#define SDCARD_ERROR_TRANSMIT_INTERRUPTED -2
#define SDCARD_ERROR_IDLE_WAIT_TIMEOUT -10
#define SDCARD_ERROR_VOLTAGE_NOT_SUPPORTED -11
#define SDCARD_ERROR_CRC_FAILED -12

#define SDCARD_IDLE_WAIT_BYTES 10

int SDCardSendCommand(uint8_t command, uint32_t param, uint8_t crc, void* buffer, size_t recvSize);
void SDCardInit();
int SDCardStartup();
void SDCardWaitIdle();

#endif /* SDCARD_H_ */
