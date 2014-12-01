/*
 * firing_board.c
 *
 *  Created on: Nov 29, 2014
 *      Author: Max Zhao
 */

#include <FreeRTOS.h>
#include <semphr.h>
#include <Chip.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "./firing_board.h"
#include "logging.h"

static I2C_ID_T firing_board_i2c_device;
static xSemaphoreHandle mutex;

void firing_board_init(void) {
	mutex = xSemaphoreCreateMutex();
}

bool firing_board_setup(I2C_ID_T i2c_device) {
	uint8_t device_id;
	firing_board_i2c_device = i2c_device;

	if (firing_board_transceive_command(0x00, NULL, 0, &device_id, sizeof(device_id)) != 0) {
		return false;
	}

	if (device_id != FIRING_BOARD_DEVICE_ID) {
		return false;
	}

	return true;
}
float firing_board_read_volt(firing_board_volt_channel_t vchan) {
	uint16_t raw_output;
	if (firing_board_transceive_command(0x01, &vchan, 1, &raw_output, sizeof(raw_output)) != 0) {
		return NAN;
	}
	return raw_output * (2.56f / 1023 * ((82+10) / 10));
}

bool firing_board_fire_channel(uint8_t channel) {
	return firing_board_transceive_command(0x03, &channel, 1, NULL, 0) == 0;
}

uint8_t firing_board_transceive_command(uint8_t command, const void* arguments, size_t arg_size, void* output, size_t output_size) {
	static uint8_t buf[16];
	if (arg_size + 1 > sizeof(buf)) {
		LOG_ERROR("Firing board command overflow at %d bytes of argument", arg_size);
		return 0xff;
	}
	if (output_size + 1 > sizeof(buf)) {
		LOG_ERROR("Firing board read overflow at %d bytes", output_size);
		return 0xff;
	}

	xSemaphoreTake(mutex, portMAX_DELAY);
	buf[0] = command;
	if (arg_size > 0) {
		memcpy(buf + 1, arguments, arg_size);
	}

	Chip_I2C_MasterSend(firing_board_i2c_device, FIRING_BOARD_ADDRESS, buf, arg_size + 1);

	size_t read_size = Chip_I2C_MasterRead(firing_board_i2c_device, FIRING_BOARD_ADDRESS, buf, output_size + 1);

	if (read_size == 0) {
		xSemaphoreGive(mutex);
		return 0xfe;
	}

	uint8_t result = buf[0];
	memcpy(output, buf + 1, output_size);

	xSemaphoreGive(mutex);
	return result;
}
