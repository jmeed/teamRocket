#ifndef H3L_H
#define H3L_H

#include "../lcp11u6x/i2c_11u6x.h"
#include <cstdint>

// Identification
#define H3L_WHO_ID		0x32

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

class H3L {
public:
	enum accel_scale {
		H3L_SCALE_100G, // 0x00
		H3L_SCALE_200G, // 0x01
		H3L_SCALE_NA,   // 0x10
		H3L_SCALE_400G  // 0x11
	};
	enum accel_odr {
		H3L_ODR_50,		// 0x00: 50 Hz, 37 Hz cutoff
		H3L_ODR_100,	// 0x01: 100 Hz, 74 Hz cutoff
		H3L_ODR_400,	// 0x02: 400 Hz, 292 Hz cutoff
		H3L_ODR_1000	// 0x03: 1000 Hz, 780 Hz cutoff
	}
	H3L(uint8_t slave_address, I2C_ID_T id);
	bool init(accel_scale a_sc);
	int16_t read_accel_raw(uint8_t dimension);
	float read_accel_g(uint8_t dimension);
	void set_accel_scale(accel_scale a_sc);
	void set_accel_odr(accel_odr a_odr);
	void configure_int_1();
	void configure_int_2();
private:
	// I2C id
	I2C_ID_T i2c_id;

	// The I2C slave address
	uint8_t slave_address;

	// Current full-range scale
	accel_scale a_scale;

	// Conversion factor (sensor scale) / (2^15)
	float a_res;

	// Sets up the accelerometer to begin reading.
	//	- H3L_CTRL_REG1 = 0x3F: Normal power, 1000 Hz ODR, all axes enabled
	//  - H3L_CTRL_REG2 = 0x00: High-pass filters disabled
	//  - H3L_CTRL_REG3 = 0x00: Interrupts active high, push-pull drain, int requests not latched
	//  - H3L_CTRL_REG4 = 0x00: 
	//  - H3L_CTRL_REG5 = 0x00: 
	void init_accel();

	// Calculate the resolution of the accelerometer.
	// This function will set the value of the a_res variable. a_scale must
	// be set prior to calling this function.
	void calc_a_res();

	// Read from a register on the device
	int8_t read_reg(uint8_t reg_addr);

	// Write to a register on the device
	void write_reg(uint8_t reg_addr, uint8_t data);
};

#endif /* H3L_H */
