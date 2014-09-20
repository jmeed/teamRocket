/*
 * spi1_os.h
 *
 *  Created on: Apr 10, 2014
 *      Author: Max Zhao
 *
 * Interrupt based SPI1 handling for FreeRTOS (only!)
 * WARNING: this module does not handle slave select and SPI mode setup.
 * Make sure these configurations are properly setup before calling transmit
 * Also, this function is not mutex protected to reduce overhead, since SPI1 is
 * ONLY used for the SDCard.
 *
 * USES PDMA_CHANNEL_1 for SPI write, and simultaneously
 * PDMA_CHANNEL_2 for SPI read.
 */

#ifndef SPI1_OS_H_
#define SPI1_OS_H_
#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <semphr.h>

extern SemaphoreHandle_t xSemSPI1TXComplete;
extern SemaphoreHandle_t xSemSPI1RXComplete;
extern SemaphoreHandle_t xMutexSPI1;

void spi1_init();
void spi1_transmit_internal(void* buffer, int length, bool read);
#define spi1_transmit(buffer, length) spi1_transmit_internal(buffer, length, true)
#define spi1_send(buffer, length) spi1_transmit_internal(buffer, length, false)
uint8_t spi1_transmit_byte(uint8_t data);
void spi1_send_byte(uint8_t data);

#endif /* SPI1_OS_H_ */
