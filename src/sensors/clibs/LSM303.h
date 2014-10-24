#ifndef LSM303_H
#define LSM303_H

#include "i2c_llu6x.h"
#include <stdint.h>
#include <math.h>

#define LSM303_SA0_LOW_ADDRESS		(0x3C) // D with SA0 low
#define LSM303_SA0_HIGH_ADDRESS		(0x3A) // D with SA0 high
#define LSM303D_WHO_ID				0x49

// Registers
#define LSM303_OUT_TEMP_L		0x05
#define LSM303_OUT_TEMP_H		0x06
#define LSM303_STATUS_M			0x07
#define LSM303_OUT_X_L_M		0x08
#define LSM303_OUT_X_H_M		0x09
#define LSM303_OUT_Y_L_M		0x0A
#define LSM303_OUT_Y_H_M		0x0B
#define LSM303_OUT_Z_L_M		0x0C
#define LSM303_OUT_Z_H_M		0x0D
#define LSM303_WHO_AM_I			0x0F
#define LSM303_INT_CTRL_M		0x12
#define LSM303_INT_SRC_M		0x13
#define LSM303_INT_THS_L_M		0x14
#define LSM303_INT_THS_H_M		0x15
#define LSM303_OFFSET_X_L_M		0x16
#define LSM303_OFFSET_X_H_M		0x17
#define LSM303_OFFSET_Y_L_M		0x18
#define LSM303_OFFSET_Y_H_M		0x19
#define LSM303_OFFSET_Z_L_M		0x1A
#define LSM303_OFFSET_Z_H_M		0x1B
#define LSM303_REFERENCE_X		0x1C
#define LSM303_REFERENCE_Y		0x1D
#define LSM303_REFERENCE_Z		0x1E
#define LSM303_CTRL_REG0		0x1F
#define LSM303_CTRL_REG1		0x20
#define LSM303_CTRL_REG2		0x21
#define LSM303_CTRL_REG3		0x22
#define LSM303_CTRL_REG4		0x23
#define LSM303_CTRL_REG5		0x24
#define LSM303_CTRL_REG6		0x25
#define LSM303_CTRL_REG7		0x26
#define LSM303_STATUS_A			0x27
#define LSM303_OUT_X_L_A		0x28
#define LSM303_OUT_X_H_A		0x29
#define LSM303_OUT_Y_L_A		0x2A
#define LSM303_OUT_Y_H_A		0x2B
#define LSM303_OUT_Z_L_A		0x2C
#define LSM303_OUT_Z_H_A		0x2D
#define LSM303_FIFO_CTRL		0x2E
#define LSM303_FIFO_SRC			0x2F
#define LSM303_IG_CFG1			0x30
#define LSM303_IG_SRC1			0x31
#define LSM303_IG_THS1			0x32
#define LSM303_IG_DUR1			0x33
#define LSM303_IG_CFG2			0x34
#define LSM303_IG_SRC2			0x35
#define LSM303_IG_THS2			0x36
#define LSM303_IG_DUR2			0x37
#define LSM303_CLICK_CFG		0x38
#define LSM303_CLICK_SRC		0x39
#define LSM303_CLICK_THS		0x3A
#define LSM303_TIME_LIMIT		0x3B
#define LSM303_TIME_LATENCY		0x3C
#define LSM303_TIME_WINDOW		0x3D
#define LSM303_Act_THS			0x3E
#define LSM303_Act_DUR			0x3F

// Dimensions
#define LSM303_ACCEL_X		1
#define LSM303_ACCEL_Y		2
#define LSM303_ACCEL_Z		3
#define LSM303_MAG_X		4
#define LSM303_MAG_Y		5
#define LSM303_MAG_Z		6
#define LSM303_MAG_HEADING 	7
#define LSM303_TEMPERATURE	8

I2C_ID_T LSM303_i2c_id;
uint8_t LSM303_slave_address;

bool LSM303_init(I2C_ID_T id_in);

// Dimensions are LSM303_ACCEL_X, LSM303_ACCEL_Y, LSM303_ACCEL_Z,
// LSM303_MAG_X, LSM303_MAG_Y, LSM303_MAG_Z, LSM303_MAG_HEADING
// and LSM303_TEMPERATURE for this sensor
float LSM303_read_data(uint8_t dimension);

/*
Enables the LSM303's accelerometer and magnetometer. Also:
- Sets sensor full scales (gain) to default power-on values, which are
  +/- 2 g for accelerometer and +/- 1.3 gauss for magnetometer
  (+/- 4 gauss on LSM303D).
- Selects 50 Hz ODR (output data rate) for accelerometer and 7.5 Hz
  ODR for magnetometer (6.25 Hz on LSM303D). (These are the ODR
  settings for which the electrical characteristics are specified in
  the datasheets.)
- Enables high resolution modes (if available).
Note that this function will also reset other settings controlled by
the registers it writes to.
*/
void LSM303_enable();

// Read data
int16_t LSM303_read_accel_raw(uint8_t dimension);
float LSM303_read_accel_g(uint8_t dimension);
int16_t LSM303_read_mag_raw(uint8_t dimension);
float LSM303_read_mag_gauss(uint8_t dimension);
int16_t LSM303_read_temperature_raw();
// Using same raw conversion as LPS sensor, probably needs calibration
float LSM303_read_temperature_C();

// Low level register work
uint8_t LSM303_read_reg(uint8_t reg_addr);
void LSM303_write_reg(uint8_t reg_addr, uint8_t data);

// Unused
//bool detect_device();

#endif /* LSM303_H */
