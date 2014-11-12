#ifndef spi_H
#define spi_H

#include <stdint.h>
#include "chip.h"

#define SPI_SCK_PORT	1
#define SPI_SCK_PIN		20
#define SPI_MOSI_PORT	0
#define SPI_MOSI_PIN	21
#define SPI_MISO_PORT	0
#define SPI_MISO_PIN	22

#define SPI_SPEED_LOW	1000000
#define SPI_SPEED_HIGH	10000000

#define SPI_BUFFER_SIZE		1024	// how much will we read/write at once?
uint8_t spi_rx_buf[SPI_BUFFER_SIZE];
uint8_t spi_tx_buf[SPI_BUFFER_SIZE];

LPC_SSP_T* spi_id;
SSP_ConfigFormat ssp_format;
Chip_SSP_DATA_SETUP_T xf_setup;

// Set up board pins
// Called by spi_init
void spi_pin_mux();

// Calls pin mux and chip library functions to initialize SPI
void spi_init(LPC_SSP_T* id_in);

// SPI transmit and receive:
//	tx_len -> number of command/address bytes
//	rx_len -> number of expected received bytes
void spi_transceive(uint8_t tx_len, uint8_t rx_len);

#endif /* spi_H */
