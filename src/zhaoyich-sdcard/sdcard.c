#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "spi1_os.h"
#include "crc.h"
#include "./sdcard.h"

static SemaphoreHandle_t xMutexSDCard;

static void SDCardSlowMode() {
	MSS_SPI_configure_master_mode
	(
		SDCARD_SPI_DEVICE,
		SDCARD_SPI_SLAVE,
		MSS_SPI_MODE0,
		MSS_SPI_PCLK_DIV_256,
		8
	);
}

static void SDCardFastMode() {
	MSS_SPI_configure_master_mode
	(
		SDCARD_SPI_DEVICE,
		SDCARD_SPI_SLAVE,
		MSS_SPI_MODE0,
		MSS_SPI_PCLK_DIV_64,
		8
	);
}

static inline void SDCardSetSS() {
	MSS_GPIO_set_output(SDCARD_SPI_SLAVE_PIN, 0);
}

static inline void SDCardClearSS() {
	MSS_GPIO_set_output(SDCARD_SPI_SLAVE_PIN, 1);
}

static uint8_t CommandBuffer[20];

void SDCardWaitIdle() {
	int i;
	retry:
	memset(CommandBuffer, 0xff, SDCARD_IDLE_WAIT_BYTES);
	spi1_transmit(CommandBuffer, SDCARD_IDLE_WAIT_BYTES);

	for (i = 0; i < SDCARD_IDLE_WAIT_BYTES; i++) {
		if (CommandBuffer[i] != 0xff) {
			goto retry;
		}
	}
}

int SDCardSendCommand(uint8_t command, uint32_t param, uint8_t crc, void* buffer, size_t recvSize) {
	xSemaphoreTake(xMutexSDCard, portMAX_DELAY);
	int result = SDCARD_ERROR_GENERIC;
	int wait = SDCARD_SPI_MAX_WAIT;
	int i;
	uint8_t read = 0;
	uint8_t* data = (uint8_t*) buffer;
	command += 0x40;
	MSS_SPI_set_slave_select(SDCARD_SPI_DEVICE, SDCARD_SPI_SLAVE);
	SDCardSetSS();
	while (spi1_transmit_byte(0xff) != 0xff);

	result = SDCARD_ERROR_TRANSMIT_INTERRUPTED;

	CommandBuffer[0] = command;
	CommandBuffer[1] = param >> 24;
	CommandBuffer[2] = param >> 16;
	CommandBuffer[3] = param >> 8;
	CommandBuffer[4] = param;
	CommandBuffer[5] = (crc_crc7(CommandBuffer, 5) << 1) | 1;

	spi1_transmit(CommandBuffer, 6);

	for (i = 0; i < 6; i++) {
		if (CommandBuffer[i] != 0xff) {
			MSS_GPIO_set_output(MSS_GPIO_27, 0);
			__asm volatile ("nop");
			MSS_GPIO_set_output(MSS_GPIO_27, 1);
			goto fail;
		}
	}

	for (;;) {
		read = spi1_transmit_byte(0xff);
		if (read != 0xff) {
			result = read;
			break;
		}
		if (wait == 0) {
			goto fail;
		}
		-- wait;
	}

	// Read block instructions
	if (command == 0x40 + 17 || command == 0x40 + 18 || command == 0x40 + 24) {
		if (read != 0) {
			goto fail;
		}

		// Wait for Data token
		for (;;) {
			read = spi1_transmit_byte(0xff);
			if (read != 0xff) {
				result = read;
				break;
			}
		}

		if (result != 0xfe) { // Data token
			result <<= 8;
			goto fail;
		}
		result = 0;
	}

	// Clear array first
	if (recvSize > 0) {
		memset(data, 0xff, recvSize);
		spi1_transmit(data, recvSize);
	}

	if (command == 0x40 + 17 || command == 0x40 + 18 || command == 0x40 + 24) {
		uint16_t crc = 0;
		// Skip crc = 2 bytes
		crc = spi1_transmit_byte(0xff) << 8;
		crc |= spi1_transmit_byte(0xff);

		uint16_t checkCRC = crc_crc16(data, recvSize);

		if (crc != checkCRC) {
			result = SDCARD_ERROR_CRC_FAILED;
			goto fail;
		}
	}
	goto finish;
 fail:
	if (result == 0) {
		result = SDCARD_ERROR_GENERIC;
	}
 finish:
	MSS_SPI_clear_slave_select(SDCARD_SPI_DEVICE, SDCARD_SPI_SLAVE);
	SDCardClearSS();
	xSemaphoreGive(xMutexSDCard);
	return result;

}

void SDCardInit() {
	SDCardSlowMode();
	MSS_GPIO_config(SDCARD_SPI_SLAVE_PIN, MSS_GPIO_OUTPUT_MODE);
	SDCardClearSS();
	xMutexSDCard = xSemaphoreCreateMutex();
}

int SDCardStartup() {
	int result = SDCARD_ERROR_GENERIC;
	int i;
	int counter;

	// Generate 80 pulses on SCLK
	MSS_SPI_set_slave_select(SDCARD_SPI_DEVICE, SDCARD_SPI_SLAVE);
	for (i = 0; i < 10; i++) {
		// Send 80 clock pulses
		MSS_SPI_transfer_frame(SDCARD_SPI_DEVICE, 0xff);
	}
	MSS_SPI_clear_slave_select(SDCARD_SPI_DEVICE, SDCARD_SPI_SLAVE);

	// Reset device
	counter = SDCARD_RESET_ATTEMPTS;
	for (;;) {
		int response = SDCardSendCommand(0, 0, 0x95, NULL, 0);
		if (response == 1) {
			break;
		}
		if (response < 0) {
			result = response;
			if (response == SDCARD_ERROR_TRANSMIT_INTERRUPTED) {
				counter --;
				if (counter > 0) {
					continue;
				}
			}
			goto fail;
		}
	}

	// Wait for start. Tested limit is ~251 cycles
	for (i = 0; i < 10000; i++) {
		int response = SDCardSendCommand(1, 0, 0xff, NULL, 0);
		if (response == 0) {
			break;
		}
		if (response < 0) {
			result = response;
			goto fail;
		}
	}

	if (i >= 10000) {
		result = SDCARD_ERROR_IDLE_WAIT_TIMEOUT;
		goto fail;
	}

	// Read OCR
	{
		uint8_t buffer[4];
		int response;
		if ((response = SDCardSendCommand(58, 0, 0xff, &buffer, 4)) != 0) {
			goto fail;
		}
		if ((buffer[1] & 0xC0) == 0) {
			result = SDCARD_ERROR_VOLTAGE_NOT_SUPPORTED;
			goto fail;
		}
		// printf("SDCard OCR = 0x%02x%02x%02x%02x (%d)\r\n", buffer[1], buffer[2], buffer[3], buffer[4], buffer[0]);
	}

	// Set block size to 512
	if ((result = SDCardSendCommand(16, 512, 0xff, NULL, 0)) != 0) {
		goto fail;
	}

	// Enable CRC checking
	if ((result = SDCardSendCommand(59, 1, 0xff, NULL, 0)) != 0) {
		goto fail;
	}

	SDCardFastMode();
	result = SDCARD_ERROR_OK;
	goto finish;
 fail:
	if (result == 0) {
		result = SDCARD_ERROR_GENERIC;
	}
 finish:
	return result;
}
