/*
 * i2c.c
 *
 *  Created on: Nov 23, 2014
 *      Author: Max Zhao
 */

#include "./i2c.h"
#include <Chip.h>

i2c_device_t i2c_devices[I2C_NUM_INTERFACE];

void i2c_init(void) {
	int i;
	i2c_devices[0].i2c_device = I2C0;
	i2c_devices[1].i2c_device = I2C1;

	for (i = 0; i < I2C_NUM_INTERFACE; i++) {
		i2c_devices[i].mutex = xSemaphoreCreateMutex();
		vSemaphoreCreateBinary(i2c_devices[i].sem_ready);
		xSemaphoreTake(i2c_devices[i].sem_ready, 0);
	}
	NVIC_EnableIRQ(I2C0_IRQn);
	NVIC_EnableIRQ(I2C1_IRQn);
}

static void i2c_freertos_master_handler(I2C_ID_T id, I2C_EVENT_T event)
{
	/* Only WAIT event needs to be handled */
	if (event == I2C_EVENT_WAIT) {
		xSemaphoreTake(i2c_devices[id].sem_ready, portMAX_DELAY);
	} else if (event == I2C_EVENT_DONE) {
		xSemaphoreGiveFromISR(i2c_devices[id].sem_ready, NULL);
	} else if (event == I2C_EVENT_LOCK) {
		xSemaphoreTake(i2c_devices[id].mutex, portMAX_DELAY);
	} else if (event == I2C_EVENT_UNLOCK) {
		xSemaphoreGive(i2c_devices[id].mutex);
	}
}

void i2c_setup_master(I2C_ID_T device) {
	Chip_I2C_Init(device);
	Chip_I2C_SetMasterEventHandler(device, i2c_freertos_master_handler);
}

void I2C0_IRQHandler(void)
{
	if (Chip_I2C_IsMasterActive(I2C0)) {
		Chip_I2C_MasterStateHandler(I2C0);
	}
	else {
		Chip_I2C_SlaveStateHandler(I2C0);
	}
}


void I2C1_IRQHandler(void)
{
	if (Chip_I2C_IsMasterActive(I2C1)) {
		Chip_I2C_MasterStateHandler(I2C1);
	}
	else {
		Chip_I2C_SlaveStateHandler(I2C1);
	}
}
