/*
 * spi.h
 *
 *  Created on: Oct 22, 2014
 *      Author: Max Zhao
 */

#ifndef SPI_H_
#define SPI_H_
#include <FreeRTOS.h>
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>
#include "chip.h"

typedef struct {
	LPC_SSP_T* ssp_device;
	xSemaphoreHandle mutex;
	xSemaphoreHandle sem_ready;
	Chip_SSP_DATA_SETUP_T xf_setup;
} spi_device_t;

#define SPI_DEVICE_COUNT 2
extern spi_device_t spi_devices[SPI_DEVICE_COUNT];

#define SPI_DEVICE_0 (&spi_devices[0])
#define SPI_DEVICE_1 (&spi_devices[1])

void spi_init(void);
void spi_setup_device(spi_device_t* device, uint32_t bits, uint32_t frameFormat, uint32_t clockMode, bool master);
void spi_enable(spi_device_t* device);
void spi_disable(spi_device_t* device);
void spi_set_bit_rate(spi_device_t* device, uint32_t bit_rate);
void spi_transceive(spi_device_t* device, uint8_t* buffer, size_t size);
uint8_t spi_transceive_byte(spi_device_t* device, uint8_t b);
void spi_receive(spi_device_t* device, uint8_t* buffer, size_t size);


#endif /* SPI_H_ */
