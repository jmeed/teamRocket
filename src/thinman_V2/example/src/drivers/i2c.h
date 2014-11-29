/*
 * i2c.h
 *
 *  Created on: Nov 23, 2014
 *      Author: Max Zhao
 */

#ifndef I2C_H_
#define I2C_H_

#include <FreeRTOS.h>
#include <semphr.h>
#include <Chip.h>

typedef struct {
	I2C_ID_T i2c_device;
	xSemaphoreHandle mutex;
	xSemaphoreHandle sem_ready;
} i2c_device_t;

extern i2c_device_t i2c_devices[I2C_NUM_INTERFACE];

void i2c_init(void);
void i2c_setup_master(I2C_ID_T device);

#endif /* I2C_H_ */
