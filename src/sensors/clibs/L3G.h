#ifndef L3G_H
#define L3G_H

#include "i2c_11u6x.h"
#include <stdint.h>
#include <math.h>

#define L3G_SA0_LOW_ADDRESS    (0xD4)
#define L3G_SA0_HIGH_ADDRESS   (0xD6)
#define L3GD20_WHO_ID_1		0xD4
#define L3GD20_WHO_ID_2		0xD7

// Registers
#define L3G_WHO_AM_I		0x0F
#define L3G_CTRL_REG1		0x20
#define L3G_CTRL_REG2		0x21
#define L3G_CTRL_REG3		0x22
#define L3G_CTRL_REG4		0x23
#define L3G_CTRL_REG5		0x24
#define L3G_REFERENCE		0x25
#define L3G_OUT_TEMP		0x26
#define L3G_STATUS_REG		0x27
#define L3G_OUT_X_L			0x28
#define L3G_OUT_X_H			0x29
#define L3G_OUT_Y_L			0x2A
#define L3G_OUT_Y_H			0x2B
#define L3G_OUT_Z_L			0x2C
#define L3G_OUT_Z_H			0x2D
#define L3G_FIFO_CTRL_REG	0x2E
#define L3G_FIFO_SRC_REG	0x2F
#define L3G_INT1_CFG		0x30
#define L3G_INT1_SRC		0x31
#define L3G_INT1_THS_XH		0x32
#define L3G_INT1_THS_XL		0x33
#define L3G_INT1_THS_YH		0x34
#define L3G_INT1_THS_YL		0x35
#define L3G_INT1_THS_ZH		0x36
#define L3G_INT1_THS_ZL		0x37
#define L3G_INT1_DURATION	0x38
#define L3G_LOW_ODR			0x39

// Dimensions
#define L3G_SPIN_RATE_X	1
#define L3G_SPIN_RATE_Y	2
#define L3G_SPIN_RATE_Z	3
#define L3G_TEMPERATURE	4

I2C_ID_T L3G_i2c_id;
uint8_t L3G_slave_address;

// Initialize the device with the I2C device ID
int L3G_init(I2C_ID_T id_in);

// Dimensions are L3G_SPIN_RATE_X, L3G_SPIN_RATE_Y, L3G_SPIN_RATE_Z, and L3G_TEMPERATURE for this sensor
float L3G_read_data(uint8_t dimension);

// Turns on sensor and enables continuous output
void L3G_enable();

// Reading data - called internally by L3G_read_data
int16_t L3G_read_spin_rate_raw(uint8_t dimension);
float L3G_read_spin_rate_dps(uint8_t dimension);
uint8_t L3G_read_temperature_raw();
float L3G_read_temperature_C();

// Low level register work
uint8_t L3G_read_reg(uint8_t reg_addr);
void L3G_write_reg(uint8_t reg_addr, uint8_t data);

// Unused, detects if device is present on I2C bus
//bool L3G_detect_device();

#endif /* L3G_H */
