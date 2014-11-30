/*
 * firing_board.h
 *
 *  Created on: Nov 29, 2014
 *      Author: Max Zhao
 */

#ifndef FIRING_BOARD_H_
#define FIRING_BOARD_H_

#define FIRING_BOARD_DEVICE_ID 0x93
#define FIRING_BOARD_ADDRESS 12

typedef enum {
	VOLTAGE_EXTERNAL_BAT,
	VOLTAGE_BUS,
} firing_board_volt_channel_t;

void firing_board_init(void);
bool firing_board_setup(I2C_ID_T i2c_device);
float firing_board_read_volt(firing_board_volt_channel_t vchan);
bool firing_board_fire_channel(uint8_t channel);
uint8_t firing_board_transceive_command(uint8_t command, const void* arguments, size_t arg_size, void* output, size_t output_size);

#endif /* FIRING_BOARD_H_ */
