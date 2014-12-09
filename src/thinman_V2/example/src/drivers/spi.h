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

// Resources needed to control a system SPI device
typedef struct {
	LPC_SSP_T* ssp_device;
	xSemaphoreHandle mutex;
	xSemaphoreHandle sem_ready;
	Chip_SSP_DATA_SETUP_T xf_setup;
	bool ready;
} spi_device_t;

// The number of SPI devices available on the system
#define SPI_DEVICE_COUNT 2
// The global array holding all configured SPI device resources
extern spi_device_t spi_devices[SPI_DEVICE_COUNT];

#define SPI_DEVICE_0 (&spi_devices[0])
#define SPI_DEVICE_1 (&spi_devices[1])

// Initialize global SPI device resources (mutexes, semaphores)
void spi_init(void);

// Set up an SPI device with the frame width, format, clock mode, and whether it should operate as master
void spi_setup_device(spi_device_t* device, uint32_t bits, uint32_t frameFormat, uint32_t clockMode, bool master);
// Enable an SPI device
void spi_enable(spi_device_t* device);
// Disable an SPI device
void spi_disable(spi_device_t* device);
// Set the bit rate of the SPI device
void spi_set_bit_rate(spi_device_t* device, uint32_t bit_rate);
// Send and receive simultaneously data through the SPI device.  The received data will overwrite the provided buffer
void spi_transceive(spi_device_t* device, uint8_t* buffer, size_t size);
// Send and receive a single byte
uint8_t spi_transceive_byte(spi_device_t* device, uint8_t b);
// Read size bytes from the SPI device.  The data line will be held high during the reception (effectively sending 0xff)
void spi_receive(spi_device_t* device, uint8_t* buffer, size_t size);
// Send data to the SPI device, and discard any received data (buffer will not be overwritten)
void spi_send(spi_device_t* device, const uint8_t* buffer, size_t size);


#endif /* SPI_H_ */
