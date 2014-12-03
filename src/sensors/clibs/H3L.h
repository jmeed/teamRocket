#ifndef H3L_H
#define H3L_H

#include "i2c_11u6x.h"
#include <stdint.h>
#include <math.h>

// Identification
#define H3L_SA0_LOW_ADDRESS		(0x30)
#define H3L_SA0_HIGH_ADDRESS	(0x32)
#define H3L_WHO_ID				0x32

// Registers
#define H3L_WHO_AM_I		0x0F // r
#define H3L_CTRL_REG1		0x20 // rw
#define H3L_CTRL_REG2		0x21 // rw
#define H3L_CTRL_REG3		0x22 // rw
#define H3L_CTRL_REG4		0x23 // rw
#define H3L_CTRL_REG5		0x24 // rw
#define H3L_HP_FILTER_RESET	0x25 // r
#define H3L_REFERENCE		0x26 // rw
#define H3L_STATUS_REG		0x27 // r
#define H3L_OUT_X_L			0x28 // r
#define H3L_OUT_X_H			0x29 // r
#define H3L_OUT_Y_L			0x2A // r
#define H3L_OUT_Y_H			0x2B // r
#define H3L_OUT_Z_L			0x2C // r
#define H3L_OUT_Z_H			0x2D // r
#define H3L_INT1_CFG		0x30 // rw
#define H3L_INT1_SRC		0x31 // r
#define H3L_INT1_THS		0x32 // rw
#define H3L_INT1_DURATION	0x33 // rw
#define H3L_INT2_CFG		0x34 // rw
#define H3L_INT2_SRC		0x35 // r
#define H3L_INT2_THS		0x36 // rw
#define H3L_INT2_DURATION	0x37 // rw

// Dimensions
#define H3L_X	1
#define H3L_Y	2
#define H3L_Z	3

// Types
enum H3L_accel_scale {
	H3L_SCALE_100G, // 0x00
	H3L_SCALE_200G, // 0x01
	H3L_SCALE_NA,   // 0x10
	H3L_SCALE_400G  // 0x11
};
enum H3L_accel_odr {
	H3L_ODR_50,		// 0x00: 50 Hz, 37 Hz cutoff
	H3L_ODR_100,	// 0x01: 100 Hz, 74 Hz cutoff
	H3L_ODR_400,	// 0x02: 400 Hz, 292 Hz cutoff
	H3L_ODR_1000	// 0x03: 1000 Hz, 780 Hz cutoff
};

// The I2C identifier
I2C_ID_T H3L_i2c_id;
// The I2C slave address
uint8_t H3L_slave_address;
// Current full-range scale
enum H3L_accel_scale H3L_a_scale;
// Conversion factor (sensor scale) / (2^15)
float H3L_a_res;

// Initializes the device with the I2C device, and the scale/data rate
int H3L_init(I2C_ID_T id_in, enum H3L_accel_scale a_sc, enum H3L_accel_odr a_odr);

// Read accelerometer values
// Possible dimensions are H3L_X, H3L_Y, H3L_Z for this device
int16_t H3L_read_accel_raw(uint8_t dimension);
float H3L_read_accel_g(uint8_t dimension);

// Set the full range scale to H3L_SCALE_100G, H3L_SCALE_200G, or H3L_SCALLE_400G
void H3L_set_accel_scale(enum H3L_accel_scale a_sc);
// Set the output data rate to H3L_ODR_50, H3L_ODR_100, H3L_ODR_400, or H3L_ODR_1000
void H3L_set_accel_odr(enum H3L_accel_odr a_odr);

// Configure the device's first interrupt. Check datasheet for register values
void H3L_configure_int_1(uint8_t int1_cfg, uint8_t int1_ths, uint8_t duration);
// Configure the device's second interrupt. Check datasheet for register values
void H3L_configure_int_2(uint8_t int2_cfg, uint8_t int2_ths, uint8_t duration);

// Sets up the accelerometer to begin reading.
//	- H3L_CTRL_REG1 = 0x3F: Normal power, 1000 Hz ODR, all axes enabled
//  - H3L_CTRL_REG2 = 0x00: High-pass filters disabled
//  - H3L_CTRL_REG3 = 0x00: Interrupts active high, push-pull drain, int requests not latched
//  - H3L_CTRL_REG4 = 0x30: No block data update, LSB @ low address, 400 g range 
//  - H3L_CTRL_REG5 = 0x00: Turn on mode default, not using low power mode
void H3L_init_accel();

// Calculate the resolution of the accelerometer.
// This function will set the value of the a_res variable. a_scale must
// be set prior to calling this function.
void H3L_calc_a_res();

// Read from a register on the device
int8_t H3L_read_reg(uint8_t reg_addr);

// Write to a register on the device
void H3L_write_reg(uint8_t reg_addr, uint8_t data);

#endif /* H3L_H */
