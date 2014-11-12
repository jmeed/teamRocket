#include "S25FL.h"

void S25FL_init(LPC_SSP_T* id_in, enum Page_Size p_sz_in, enum Erase_Size e_sz_in) {
	// Store global state
	S25FL_ssp_id = id_in;
	S25FL_p_size = p_sz_in;
	S25FL_e_size = e_sz_in;

	// Set config if not defaults
	if (S25FL_p_size != S25FL_P_256)
		S25FL_set_page_size(S25FL_p_size);
	if (S25FL_e_size != S25FL_E_64)
		S25FL_set_erase_size(S25FL_e_size);

	// Set up SPI
	spi_init(id_in);
	S25FL_rx_buf = spi_rx_buf;
	S25FL_tx_buf = spi_tx_buf;

	// Set up slave select
	Chip_IOCON_PinMuxSet(LPC_IOCON, S25FL_SS_PORT, S25FL_SS_PIN, 
		(IOCON_FUNC0 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, S25FL_SS_PORT, S25FL_SS_PIN);
}

void S25FL_write_registers(uint8_t status_1, uint8_t config, uint8_t status_2) {
	// Setup params
	S25FL_tx_len = 4; // 1 command byte and 3 registers
	S25FL_rx_len = 0;

	// Copy data into buffer
	S25FL_tx_buf[0] = S25FL_WRR;
	S25FL_tx_buf[1] = status_1;
	S25FL_tx_buf[2] = config;
	S25FL_tx_buf[3] = status_2;

	// Send
	S25FL_write_enable();
	S25FL_ss_set();
	spi_transceive(S25FL_tx_len, S25FL_rx_len);
	S25FL_ss_clear();

	S25FL_write_wait();
}

uint8_t S25FL_read_register(uint8_t reg) {
	// Setup params
	S25FL_tx_len = 1; // add 1 command byte and 3 address bytes
	S25FL_rx_len = 1;

	// Copy data into buffer
	S25FL_tx_buf[0] = reg;

	// Send
	S25FL_ss_set();
	spi_transceive(S25FL_tx_len, S25FL_rx_len);
	S25FL_ss_clear();

	return S25FL_rx_buf[0];
}

void S25FL_set_page_size(enum Page_Size page_size) {
	uint8_t status_1 = S25FL_read_register(S25FL_RDSR1);
	uint8_t config = S25FL_read_register(S25FL_RDCR);
	uint8_t status_2 = S25FL_read_register(S25FL_RDSR2);
	status_2 &= 0xBF; // mask out page size bit
	status_2 |= (((uint8_t)page_size << 6) & 0x40); // set page size bit
	S25FL_write_registers(status_1, config, status_2);
}

void S25FL_set_erase_size(enum Erase_Size erase_size) {
	uint8_t status_1 = S25FL_read_register(S25FL_RDSR1);
	uint8_t config = S25FL_read_register(S25FL_RDCR);
	uint8_t status_2 = S25FL_read_register(S25FL_RDSR2);
	status_2 &= 0x7F; // mask out erase size bit
	status_2 |= (((uint8_t)erase_size << 7) & 0x80); // set page size bit
	S25FL_write_registers(status_1, config, status_2);
}

void S25FL_reset() {
	// Setup params
	S25FL_tx_len = 1; // 1 command byte
	S25FL_rx_len = 0;

	// Copy data into buffer
	S25FL_tx_buf[0] = S25FL_RESET;

	// Send
	S25FL_ss_set();
	spi_transceive(S25FL_tx_len, S25FL_rx_len);
	S25FL_ss_clear();
}

void S25FL_ss_set() {
	Chip_GPIO_SetPinState(LPC_GPIO, S25FL_SS_PORT, S25FL_SS_PIN, 0);
}

void S25FL_ss_clear() {
	Chip_GPIO_SetPinState(LPC_GPIO, S25FL_SS_PORT, S25FL_SS_PIN, 1);
}

void S25FL_write(uint32_t address, uint8_t* buffer, uint32_t length) {
	// Setup params
	S25FL_tx_len = length + 4; // add 1 command byte and 3 address bytes
	S25FL_rx_len = 0;

	// Copy data into buffer
	S25FL_tx_buf[0] = S25FL_PP;
	S25FL_tx_buf[1] = (address >> 16) & 0xFF;
	S25FL_tx_buf[2] = (address >> 8) & 0xFF;
	S25FL_tx_buf[3] = address & 0xFF;
	if (length > SPI_BUFFER_SIZE) {
		length = SPI_BUFFER_SIZE;
		printf("Warning: writing too much data, increase buffer size?\n");
	}
	int i;
	for (i = 0; i < length; ++i)
		S25FL_tx_buf[i + 4] = buffer[i];

	// Send
	S25FL_write_enable();
	S25FL_ss_set();
	spi_transceive(S25FL_tx_len, S25FL_rx_len);
	S25FL_ss_clear();

	S25FL_write_wait();
}

void S25FL_write_wait() {
//	while (S25FL_read_register(S25FL_RDSR1) & 0x01) {}
	volatile int i;
	for (i = 0; i < S25FL_DUMMY_CYCLES; i++) {}
}

void S25FL_write_enable() {
	// Setup params
	S25FL_tx_len = 1;
	S25FL_rx_len = 0;

	// Copy data into buffer
	S25FL_tx_buf[0] = S25FL_WREN;

	// Send
	S25FL_ss_set();
	spi_transceive(S25FL_tx_len, S25FL_rx_len);
	S25FL_ss_clear();

	// Wait for Write Enable Latch to be set
//	while (!(S25FL_read_register(S25FL_RDSR1) & 0x02)) {}
	S25FL_write_wait();
}

void S25FL_write_disable() {
	// Setup params
	S25FL_tx_len = 1;
	S25FL_rx_len = 0;

	// Copy data into buffer
	S25FL_tx_buf[0] = S25FL_WRDI;

	// Send
	S25FL_ss_set();
	spi_transceive(S25FL_tx_len, S25FL_rx_len);
	S25FL_ss_clear();
}

void S25FL_read(uint32_t address, uint8_t* buffer, uint32_t length) {
	// Setup params
	S25FL_tx_len = 4; 		// 1 command byte and 3 address bytes
	S25FL_rx_len = length;

	// Copy data into buffer
	S25FL_tx_buf[0] = S25FL_READ;
	S25FL_tx_buf[1] = (address >> 16) & 0xFF;
	S25FL_tx_buf[2] = (address >> 8) & 0xFF;
	S25FL_tx_buf[3] = address & 0xFF;
	if (length > SPI_BUFFER_SIZE) {
		length = SPI_BUFFER_SIZE;
		printf("Warning: reading too much data, increase buffer size?\n");
	}

	// Send
	S25FL_ss_set();
	spi_transceive(S25FL_tx_len, S25FL_rx_len);
	S25FL_ss_clear();

	// Copy data to buffer
	int i;
	for (i = 0; i < length; ++i)
		buffer[i] = S25FL_rx_buf[i + 4];
}

void S25FL_erase_4k(uint32_t address) {
	// Check if this does anything (only works with 64 kB sectors)
	if (S25FL_e_size == S25FL_E_256) {
		printf("Warning: erase 4k command ignored\n");
		return;
	}

	// Setup params
	S25FL_tx_len = 4; // 1 command byte and 3 address bytes
	S25FL_rx_len = 0;

	// Copy data into buffer
	S25FL_tx_buf[0] = S25FL_P4E;
	S25FL_tx_buf[1] = (address >> 16) & 0xFF;
	S25FL_tx_buf[2] = (address >> 8) & 0xFF;
	S25FL_tx_buf[3] = address & 0xFF;

	// Send
	S25FL_write_enable();
	S25FL_ss_set();
	spi_transceive(S25FL_tx_len, S25FL_rx_len);
	S25FL_ss_clear();

	S25FL_write_wait();
}

void S25FL_erase_sector(uint32_t address) {
	// Setup params
	S25FL_tx_len = 4; // 1 command byte and 3 address bytes
	S25FL_rx_len = 0;

	// Copy data into buffer
	S25FL_tx_buf[0] = S25FL_SE;
	S25FL_tx_buf[1] = (address >> 16) & 0xFF;
	S25FL_tx_buf[2] = (address >> 8) & 0xFF;
	S25FL_tx_buf[3] = address & 0xFF;

	// Send
	S25FL_write_enable();
	S25FL_ss_set();
	spi_transceive(S25FL_tx_len, S25FL_rx_len);
	S25FL_ss_clear();

	S25FL_write_wait();
}

void S25FL_erase_bulk() {
	// Setup params
	S25FL_tx_len = 1; // 1 command byte
	S25FL_rx_len = 0;

	// Copy data into buffer
	S25FL_tx_buf[0] = S25FL_BE;

	// Send
	S25FL_write_enable();
	S25FL_ss_set();
	spi_transceive(S25FL_tx_len, S25FL_rx_len);
	S25FL_ss_clear();

	S25FL_write_wait();
}
