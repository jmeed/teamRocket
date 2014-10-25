#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "crc.h"
#include "spi.h"
#include "logging.h"
#include "./sdcard.h"

static xSemaphoreHandle xMutexSDCard;

static void SDCardSlowMode() {
	spi_set_bit_rate(SDCARD_SPI_DEVICE, 100000);
}

static void SDCardFastMode() {
	spi_set_bit_rate(SDCARD_SPI_DEVICE, 1000000);
}

static inline void SDCardSetSS() {
	Chip_GPIO_SetPinState(LPC_GPIO, SDCARD_SPI_SLAVE_PORT, SDCARD_SPI_SLAVE_PIN, false);
}

static inline void SDCardClearSS() {
	Chip_GPIO_SetPinState(LPC_GPIO, SDCARD_SPI_SLAVE_PORT, SDCARD_SPI_SLAVE_PIN, true);
}

static uint8_t CommandBuffer[20];

void SDCardWaitIdle() {
	int i;
	retry:
	memset(CommandBuffer, 0xff, SDCARD_IDLE_WAIT_BYTES);
	spi_transceive(SDCARD_SPI_DEVICE, CommandBuffer, SDCARD_IDLE_WAIT_BYTES);

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
	SDCardSetSS();
	while (spi_transceive_byte(SDCARD_SPI_DEVICE, 0xff) != 0xff);

	result = SDCARD_ERROR_TRANSMIT_INTERRUPTED;

	CommandBuffer[0] = command;
	CommandBuffer[1] = param >> 24;
	CommandBuffer[2] = param >> 16;
	CommandBuffer[3] = param >> 8;
	CommandBuffer[4] = param;
	CommandBuffer[5] = (crc_crc7(CommandBuffer, 5) << 1) | 1;

	spi_transceive(SDCARD_SPI_DEVICE, CommandBuffer, 6);

	for (i = 0; i < 6; i++) {
		if (CommandBuffer[i] != 0xff) {
//			MSS_GPIO_set_output(MSS_GPIO_27, 0);
			__asm volatile ("nop");
//			MSS_GPIO_set_output(MSS_GPIO_27, 1);
			goto fail;
		}
	}

	for (;;) {
		read = spi_transceive_byte(SDCARD_SPI_DEVICE, 0xff);
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
			read = spi_transceive_byte(SDCARD_SPI_DEVICE, 0xff);
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
		spi_transceive(SDCARD_SPI_DEVICE, data, recvSize);
	}

	if (command == 0x40 + 17 || command == 0x40 + 18 || command == 0x40 + 24) {
		uint16_t crc = 0;
		// Skip crc = 2 bytes
		crc = spi_transceive_byte(SDCARD_SPI_DEVICE, 0xff) << 8;
		crc |= spi_transceive_byte(SDCARD_SPI_DEVICE, 0xff);

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
	SDCardClearSS();
	xSemaphoreGive(xMutexSDCard);
	return result;

}

void SDCardInit() {
	SDCardSlowMode();
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, SDCARD_SPI_SLAVE_PORT, SDCARD_SPI_SLAVE_PIN);
	SDCardClearSS();
	xMutexSDCard = xSemaphoreCreateMutex();
}

int SDCardStartup() {
	int result = SDCARD_ERROR_GENERIC;
	int i;
	int counter;

	// Generate 80 pulses on SCLK
	LOG_DEBUG("Sending 80 pulses");
	for (i = 0; i < 10; i++) {
		// Send 80 clock pulses
		spi_transceive_byte(SDCARD_SPI_DEVICE, 0xff);
	}

	// Reset device
	LOG_DEBUG("Resetting SDCard");
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

	LOG_DEBUG("Waiting for SDCARD to start");
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
