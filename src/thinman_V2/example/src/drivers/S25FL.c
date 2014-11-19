#include "S25FL.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include "logging.h"
#include "error_codes.h"
#include "chip.h"

static spi_device_t* S25FL_spi_device = NULL;
static enum Page_Size S25FL_p_size;
static enum Erase_Size S25FL_e_size;
static xSemaphoreHandle mutex_flash;
static xSemaphoreHandle mutex_write_flash_sector;

size_t S25FL_read_sector_count = 0;
size_t S25FL_write_sector_count = 0;
size_t S25FL_erase_sector_count = 0;

static void S25FL_write_wait();

inline void flash_enter() {
	xSemaphoreTake(mutex_flash, portMAX_DELAY);
}

inline void flash_exit() {
	xSemaphoreGive(mutex_flash);
}

void S25FL_init(spi_device_t* spi_device, enum Page_Size p_sz_in, enum Erase_Size e_sz_in) {
	// Store global state
	mutex_flash = xSemaphoreCreateMutex();
	mutex_write_flash_sector = xSemaphoreCreateMutex();
	S25FL_spi_device = spi_device;
	S25FL_p_size = p_sz_in;
	S25FL_e_size = e_sz_in;

	// Set up slave select
	Chip_IOCON_PinMuxSet(LPC_IOCON, S25FL_SS_PORT, S25FL_SS_PIN,
		(IOCON_FUNC0 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, S25FL_SS_PORT, S25FL_SS_PIN);

	S25FL_ss_clear();

	S25FL_reset();

	// Set config if not defaults
	if (S25FL_p_size != S25FL_P_256)
		S25FL_set_page_size(S25FL_p_size);
	if (S25FL_e_size != S25FL_E_64)
		S25FL_set_erase_size(S25FL_e_size);
}

void S25FL_write_registers(uint8_t status_1, uint8_t config, uint8_t status_2) {
	// Setup params
	S25FL_write_enable();

	uint8_t tx_buf[4];

	// Copy data into buffer
	tx_buf[0] = S25FL_WRR;
	tx_buf[1] = status_1;
	tx_buf[2] = config;
	tx_buf[3] = status_2;

	// Send
	S25FL_ss_set();
	spi_send(S25FL_spi_device, tx_buf, 4);
	S25FL_ss_clear();

	S25FL_write_wait();
}

uint8_t S25FL_read_register(uint8_t reg) {

	uint8_t buf[2];

	// Copy data into buffer
	buf[0] = reg;

	// Send
	S25FL_ss_set();
	spi_transceive(S25FL_spi_device, buf, 2);
	S25FL_ss_clear();

	return buf[1];
}

void S25FL_set_page_size(enum Page_Size page_size) {
	flash_enter();
	uint8_t status_1 = S25FL_read_register(S25FL_RDSR1);
	uint8_t config = S25FL_read_register(S25FL_RDCR);
	uint8_t status_2 = S25FL_read_register(S25FL_RDSR2);
	status_2 &= 0xBF; // mask out page size bit
	status_2 |= (((uint8_t)page_size << 6) & 0x40); // set page size bit
	S25FL_write_registers(status_1, config, status_2);
	flash_exit();
}

void S25FL_set_erase_size(enum Erase_Size erase_size) {
	flash_enter();
	uint8_t status_1 = S25FL_read_register(S25FL_RDSR1);
	uint8_t config = S25FL_read_register(S25FL_RDCR);
	uint8_t status_2 = S25FL_read_register(S25FL_RDSR2);
	status_2 &= 0x7F; // mask out erase size bit
	status_2 |= (((uint8_t)erase_size << 7) & 0x80); // set page size bit
	S25FL_write_registers(status_1, config, status_2);
	flash_exit();
}

void S25FL_reset() {
	flash_enter();
	S25FL_ss_set();
	spi_transceive_byte(S25FL_spi_device, S25FL_RESET);
	S25FL_ss_clear();
	flash_exit();
}

void S25FL_ss_set() {
	Chip_GPIO_SetPinState(LPC_GPIO, S25FL_SS_PORT, S25FL_SS_PIN, 0);
}

void S25FL_ss_clear() {
	Chip_GPIO_SetPinState(LPC_GPIO, S25FL_SS_PORT, S25FL_SS_PIN, 1);
}

void S25FL_write(uint32_t address, uint8_t* buffer, uint32_t length) {
	flash_enter();
	// Setup params
	S25FL_write_enable();
	uint8_t tx_buf[4];

	// Copy data into buffer
	tx_buf[0] = S25FL_PP;
	tx_buf[1] = (address >> 16) & 0xFF;
	tx_buf[2] = (address >> 8) & 0xFF;
	tx_buf[3] = address & 0xFF;

	// Send
	S25FL_ss_set();
	spi_send(S25FL_spi_device, tx_buf, 4);
	spi_send(S25FL_spi_device, buffer, length);
	S25FL_ss_clear();

	S25FL_write_wait();
	flash_exit();
}

static void S25FL_write_wait() {
	while (S25FL_read_register(S25FL_RDSR1) & 0x01) {}
}

void S25FL_write_enable() {
	// Send
	S25FL_ss_set();
	spi_transceive_byte(S25FL_spi_device, S25FL_WREN);
	S25FL_ss_clear();

	// Wait for Write Enable Latch to be set
	while (!(S25FL_read_register(S25FL_RDSR1) & 0x02)) {}
}

void S25FL_write_disable() {
	// Setup params

	// Send
	S25FL_ss_set();
	spi_transceive_byte(S25FL_spi_device, S25FL_WRDI);
	S25FL_ss_clear();
}

void S25FL_read(uint32_t address, uint8_t* buffer, uint32_t length) {
	// Setup params
	flash_enter();

	uint8_t tx_buf[4];
	// Copy data into buffer
	tx_buf[0] = S25FL_READ;
	tx_buf[1] = (address >> 16) & 0xFF;
	tx_buf[2] = (address >> 8) & 0xFF;
	tx_buf[3] = address & 0xFF;

	// Send
	S25FL_ss_set();
	spi_send(S25FL_spi_device, tx_buf, 4);
	spi_receive(S25FL_spi_device, buffer, length);
	S25FL_ss_clear();

	flash_exit();
}

void S25FL_erase_4k(uint32_t address) {
	// Check if this does anything (only works with 64 kB sectors)
	if (S25FL_e_size == S25FL_E_256) {
		LOG_WARN("erase 256k command ignored");
		return;
	}

	S25FL_erase_sector_count ++;

	flash_enter();
	S25FL_write_enable();
	// Setup params
	uint8_t tx_buf[4];
	// Copy data into buffer
	tx_buf[0] = S25FL_P4E;
	tx_buf[1] = (address >> 16) & 0xFF;
	tx_buf[2] = (address >> 8) & 0xFF;
	tx_buf[3] = address & 0xFF;

	// Send
	S25FL_ss_set();
	spi_send(S25FL_spi_device, tx_buf, 4);
	S25FL_ss_clear();

	S25FL_write_wait();
	flash_exit();
}

void S25FL_erase_sector(uint32_t address) {
	// Setup params
	flash_enter();
	S25FL_write_enable();

	uint8_t tx_buf[4];
	// Copy data into buffer
	tx_buf[0] = S25FL_SE;
	tx_buf[1] = (address >> 16) & 0xFF;
	tx_buf[2] = (address >> 8) & 0xFF;
	tx_buf[3] = address & 0xFF;

	// Send
	S25FL_ss_set();
	spi_send(S25FL_spi_device, tx_buf, 4);
	S25FL_ss_clear();

	S25FL_write_wait();
	flash_exit();
}

void S25FL_erase_bulk() {
	// Setup params

	flash_enter();
	S25FL_write_enable();

	// Send
	S25FL_ss_set();
	spi_transceive_byte(S25FL_spi_device, S25FL_BE);
	S25FL_ss_clear();

	S25FL_write_wait();
	flash_exit();
}


uint8_t erase_buffer[512 * 3];

static bool check_erase_necessary(const uint8_t* data, uint32_t sector) {
	// @lock necessary
	int i;
	S25FL_read_sector(erase_buffer, sector);
	for (i = 0; i < 512; i++) {
		uint8_t datab = data[i];
		uint8_t currentb = erase_buffer[i];

		// all bits that should be set to 1 should be 1 in the current data
		if ((datab & currentb) != datab) {
			return true;
		}
	}
	return false;
}

void S25FL_read_sector(uint8_t* buffer, uint32_t sector) {
	uint32_t address = sector * 512;
	S25FL_read_sector_count ++;
	S25FL_read(address, buffer, 512);
}

void S25FL_read_sectors(uint8_t* buffer, uint32_t sector, size_t count) {
	while (count > 0) {
		S25FL_read_sector(buffer, sector);
		sector += 1;
		count -= 1;
		buffer += 512;
	}
}

static void S25FL_write_sector_direct(const uint8_t* buffer, uint32_t sector) {
	S25FL_write_sector_count ++;
	S25FL_write(sector * 512, (uint8_t*) buffer, 512);
}

void S25FL_write_sector(const uint8_t* buffer, uint32_t sector) {
	xSemaphoreTake(mutex_write_flash_sector, portMAX_DELAY);
	if (check_erase_necessary(buffer, sector)) {
		const uint32_t erase_sector = sector & (~3); // Clear lower 2 bits
		// Backup sectors
		{
			uint32_t backup_sector_i = 0;
			int i;
			for (i = 0; i < 4; i++) {
				uint32_t current_sector = erase_sector + i;
				if (current_sector == sector) continue;
				S25FL_read_sector(&erase_buffer[backup_sector_i * 512], current_sector);
				backup_sector_i ++;
			}

			if (backup_sector_i != 3) {
				exit_error(ERROR_CODE_FLASH_SECTOR_BACKUP_INCORRECT);
			}
		}

		// Erase sectors
		S25FL_erase_4k(erase_sector * 512);

		// Restore sectors
		{
			uint32_t restore_sector_i = 0;
			int i;
			for (i = 0; i < 4; i++) {
				uint32_t current_sector = erase_sector + i;
				if (current_sector == sector) continue;
				S25FL_write_sector_direct(&erase_buffer[restore_sector_i * 512], current_sector);
				restore_sector_i ++;
			}

			if (restore_sector_i != 3) {
				exit_error(ERROR_CODE_FLASH_SECTOR_RESTORE_INCORRECT);
			}
		}
	}
	// Write sector

	S25FL_write_sector_direct((uint8_t*) buffer, sector);
	xSemaphoreGive(mutex_write_flash_sector);
}

void S25FL_write_sectors(const uint8_t* data, uint32_t sector, size_t count) {
	// XXX: optimize
	while (count > 0) {
		S25FL_write_sector(data, sector);
		sector ++;
		count --;
		data += 512;
	}
}

bool S25FL_initialized(void) {
	return S25FL_spi_device != NULL;
}
