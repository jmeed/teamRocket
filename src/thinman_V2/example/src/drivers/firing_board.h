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

// Voltage types to be read from the firing board
typedef enum {
	VOLTAGE_EXTERNAL_BAT,
	VOLTAGE_BUS,
} firing_board_volt_channel_t;

// Flag indicating whether last communication failed with NACK
extern bool firing_board_transmit_error;

// Initialize firing board resources (mutexes)
void firing_board_init(void);
// Set up the firing board, return true if successful.
bool firing_board_setup(I2C_ID_T i2c_device);
// Read a voltage from the firing board as a float
float firing_board_read_volt(firing_board_volt_channel_t vchan);
// Command the firing board to start firing one channel
bool firing_board_fire_channel(uint8_t channel);
// Send a command to the firing board, and read the output.  Returns 0 if successful. Non-zero error code otherwise.
uint8_t firing_board_transceive_command(uint8_t command, const void* arguments, size_t arg_size, void* output, size_t output_size);

#endif /* FIRING_BOARD_H_ */
