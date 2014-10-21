/*
 * i2c_com.c
 *
 *  Created on: Apr 13, 2013
 *      Author: ftheo
 */

#include "i2c_com.h"
#include <assert.h>
#include "i2c.h"

// The message to send should be in the I2CMasterBuffer
void send_i2c_msg(uint8_t w_addr, uint8_t reg, uint8_t length) {
	I2CWriteLength = 2 + length;
	I2CReadLength = 0;
	I2CMasterBuffer[0] = w_addr;
	I2CMasterBuffer[1] = reg;
	I2CEngine();

	assert(I2CMasterState == I2C_OK);
}

void write_i2c_register(uint8_t w_addr, uint8_t reg, uint8_t value) {
	I2CMasterBuffer[2] = value;
	send_i2c_msg(w_addr, reg, 1);
}

uint8_t read_i2c_register(uint8_t r_addr, uint8_t w_addr, uint8_t reg) {
	I2CWriteLength = 2;
	I2CReadLength = 1;
	I2CMasterBuffer[0] = w_addr;
	I2CMasterBuffer[1] = reg;
	I2CMasterBuffer[2] = r_addr;
	I2CEngine();

	assert(I2CMasterState == I2C_OK);
	return I2CSlaveBuffer[0];
}
