#include "spi.h"

void spi_pin_mux() {
	Chip_IOCON_PinMuxSet(LPC_IOCON, SPI_SCK_PORT, SPI_SCK_PIN,
		(IOCON_FUNC2 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, SPI_MOSI_PORT, SPI_MOSI_PIN,
		(IOCON_FUNC2 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, SPI_MISO_PORT, SPI_MISO_PIN,
		(IOCON_FUNC3 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);
}

void spi_init(LPC_SSP_T* id_in) {
	// Set up global state
	spi_id = id_in;

	// Set up pins
	spi_pin_mux();

	// Setup format
	ssp_format.bits = SSP_BITS_8;
	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.clockMode = SSP_CLOCK_MODE0;

	// Set up buffers
	xf_setup.rx_data = spi_rx_buf;
	xf_setup.tx_data = spi_tx_buf;

	// Chip library calls
	Chip_SSP_Init(spi_id);
	Chip_SSP_SetMaster(spi_id, 1);
	Chip_SSP_SetFormat(spi_id, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
	Chip_SSP_Enable(spi_id);
	Chip_SSP_SetBitRate(spi_id, SPI_SPEED_LOW);
}

void spi_transceive(uint8_t tx_len, uint8_t rx_len) {
	// Setup params for sending
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;
	xf_setup.length = tx_len + rx_len;

	// Send
	Chip_SSP_RWFrames_Blocking(spi_id, &xf_setup);
}
