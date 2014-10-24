#ifndef LSM_H
#define LSM_H

#include "i2c_11u6x.h"
#include <stdint.h>
#include <math.h>

// Identification codes
#define LSM_XLG_SA0_LOW_ADDRESS		(0xD4 >> 1)
#define LSM_XLG_SA0_HIGH_ADDRESS	(0xD6 >> 1)
#define LSM_MAG_SA0_LOW_ADDRESS		(0x38 >> 1)
#define LSM_MAG_SA0_HIGH_ADDRESS	(0x3C >> 1)
#define LSM_XLG_WHO_ID	0x68
#define LSM_M_WHO_ID	0x3D

// Registers
#define LSM_ACT_THS				0x04
#define LSM_ACT_DUR				0x05
#define LSM_INT_GEN_CFG_XL		0x06
#define LSM_INT_GEN_THS_X_XL	0x07
#define LSM_INT_GEN_THS_Y_XL	0x08
#define LSM_INT_GEN_THS_Z_XL	0x09
#define LSM_INT_GEN_DUR_XL		0x0A
#define LSM_REFERENCE_G			0x0B
#define LSM_INT1_CTRL			0x0C
#define LSM_INT2_CTRL			0x0D
#define LSM_WHO_AM_I_XLG		0x0F
#define LSM_CTRL_REG1_G			0x10
#define LSM_CTRL_REG2_G			0x11
#define LSM_CTRL_REG3_G			0x12
#define LSM_ORIENT_CFG_G		0x13
#define LSM_INT_GEN_SRC_G		0x14
#define LSM_OUT_TEMP_L			0x15
#define LSM_OUT_TEMP_H			0x16
#define LSM_STATUS_REG1_XL		0x17
#define LSM_OUT_X_L_G			0x18
#define LSM_OUT_X_H_G			0x19
#define LSM_OUT_Y_L_G			0x1A
#define LSM_OUT_Y_H_G			0x1B
#define LSM_OUT_Z_L_G			0x1C
#define LSM_OUT_Z_H_G			0x1D
#define LSM_CTRL_REG4			0x1E
#define LSM_CTRL_REG5_XL		0x1F
#define LSM_CTRL_REG6_XL		0x20
#define LSM_CTRL_REG7_XL		0x21
#define LSM_CTRL_REG8			0x22
#define LSM_CTRL_REG9			0x23
#define LSM_CTRL_REG10			0x24
#define LSM_INT_GEN_SRC_XL		0x26
#define LSM_STATUS_REG2_XL		0x27 // typo in datasheet
#define LSM_OUT_X_L_XL			0x28
#define LSM_OUT_X_H_XL			0x29
#define LSM_OUT_Y_L_XL			0x2A
#define LSM_OUT_Y_H_XL			0x2B
#define LSM_OUT_Z_L_XL			0x2C
#define LSM_OUT_Z_H_XL			0x2D
#define LSM_FIFO_CTRL			0x2E
#define LSM_FIFO_SRC			0x2F
#define LSM_INT_GEN_CFG_G		0x30
#define LSM_INT_GEN_THS_XH_G	0x31
#define LSM_INT_GEN_THS_XL_G	0x32
#define LSM_INT_GEN_THS_YH_G	0x33
#define LSM_INT_GEN_THS_YL_G	0x34
#define LSM_INT_GEN_THS_ZH_G	0x35
#define LSM_INT_GEN_THS_ZL_G	0x36
#define LSM_INT_GEN_DUR_G		0x37
#define LSM_OFFSET_X_REG_L_M	0x05
#define LSM_OFFSET_X_REG_H_M	0x06
#define LSM_OFFSET_Y_REG_L_M	0x07
#define LSM_OFFSET_Y_REG_H_M	0x08
#define LSM_OFFSET_Z_REG_L_M	0x09
#define LSM_OFFSET_Z_REG_H_M	0x0A
#define LSM_WHO_AM_I_M 			0x0F
#define LSM_CTRL_REG1_M			0x20
#define LSM_CTRL_REG2_M			0x21
#define LSM_CTRL_REG3_M			0x22
#define LSM_CTRL_REG4_M			0x23
#define LSM_CTRL_REG5_M			0x24
#define LSM_STATUS_REG_M		0x27
#define LSM_OUT_X_L_M			0x28
#define LSM_OUT_X_H_M			0x29
#define LSM_OUT_Y_L_M			0x2A
#define LSM_OUT_Y_H_M			0x2B
#define LSM_OUT_Z_L_M			0x2C
#define LSM_OUT_Z_H_M			0x2D
#define LSM_INT_CFG_M			0x30
#define LSM_INT_SRC_M			0x31
#define LSM_INT_THS_L_M			0x32
#define LSM_INT_THS_H_M			0x33

// Dimensions
#define LSM_GYRO_X	1
#define LSM_GYRO_Y	2
#define LSM_GYRO_Z	3
#define LSM_ACCEL_X	4
#define LSM_ACCEL_Y	5
#define LSM_ACCEL_Z	6
#define LSM_MAG_X	7
#define LSM_MAG_Y	8
#define LSM_MAG_Z	9

// The I2C identifier
I2C_ID_T LSM_i2c_id;
// The I2C slave addresses for both devices
uint8_t LSM_xlg_address;
uint8_t LSM_mag_address;
// Store the current full-range scales
gyro_scale LSM_g_scale;
accel_scale LSM_a_scale;
mag_scale LSM_m_scale;

// Store the current resolution for each sensor
// Units of these values would be DPS (or g's or Gs's) per ADC tick.
// This value is calculated as (sensor scale) / (2^15).
float LSM_g_res, LSM_a_res, LSM_m_res;

// gyro_scale defines the possible full-scale ranges of the gyroscope:
enum gyro_scale {
	G_SCALE_245DPS,		// 00:  245 degrees per second
	G_SCALE_500DPS,		// 01:  500 dps
	G_SCALE_NA,			// 10:	n/a
	G_SCALE_2000DPS		// 11:  2000 dps
};
// accel_scale defines all possible FSR's of the accelerometer:
enum accel_scale {
	A_SCALE_2G,	// 00:  2g
	A_SCALE_4G,	// 01:  4g
	A_SCALE_NA,	// 10:  na
	A_SCALE_8G	// 11   8g
};
// mag_scale defines all possible FSR's of the magnetometer:
enum mag_scale {
	M_SCALE_4GS,	// 00:  2Gs
	M_SCALE_8GS, 	// 01:  4Gs
	M_SCALE_12GS,	// 10:  8Gs
	M_SCALE_16GS	// 11:  12Gs
};
// gyro_odr defines all possible data rate/bandwidth combos of the gyro:
enum gyro_odr {
	G_POWER_DOWN,		// Power-down mode (0x00)
	G_ODR_14_9,			// 14.9 Hz, Cutoff 5 Hz (0x01)
	G_ODR_59_5,			// 59.5 Hz, Cutoff 19 Hz, (0x02)
	G_ODR_119,			// 119.0 Hz, Cutoff 38 Hz, (0x03)
	G_ODR_238,			// 238.0 Hz, Cutoff 76 Hz, (0x04)
	G_ODR_476,			// 476.0 Hz, Cutoff 100 Hz, (0x05)
	G_ODR_952			// 952 Hz, Cutoff 100 Hz, (0x06)
};
// accel_oder defines all possible output data rates of the accelerometer:
enum accel_odr {
	A_POWER_DOWN, 	// Power-down mode (0x00)
	A_ODR_10,		// 10.0 Hz (0x01)
	A_ODR_50,		// 50.0 Hz (0x02)
	A_ODR_119,		// 119.0 Hz (0x03)
	A_ODR_238,		// 238.0 Hz (0x04)
	A_ODR_476,		// 476.0 Hz (0x05)
	A_ODR_952		// 952.0 Hz (0x06)
};
// mag_oder defines all possible output data rates of the magnetometer:
enum mag_odr {
	M_ODR_0_625,	// 0.625 Hz (0x00)
	M_ODR_1_25,		// 1.25 Hz (0x01)
	M_ODR_2_5,		// 2.5 Hz (0x02)
	M_ODR_5,		// 5.0 Hz (0x03)
	M_ODR_10,		// 10.0 Hz (0x04)
	M_ODR_20,		// 20.0 Hz (0x05)
	M_ODR_40,		// 40.0 Hz (0x06)
	M_ODR_80		// 80.0 Hz (0x07)
};
// accel_abw defines all possible anti-aliasing filter rates of the accelerometer:
enum accel_abw {
	A_ABW_408,		// 408 Hz (0x0)
	A_ABW_211,		// 211 Hz (0x1)
	A_ABW_105,		// 105 Hz (0x2)
	A_ABW_50		//  50 Hz (0x3)
};

// Store the bias offsets for calibration
//float a_bias[3];
//float g_bias[3];

// Initialize the gyro, accelerometer, and magnetometer.
// This will set up the scale and output rate of each sensor. It'll also
// "turn on" every sensor and every axis of every sensor. Returns true
// if communication was successful with both devices.
bool LSM_init(	I2C_ID_T id_in,
			gyro_scale g_sc,
			accel_scale a_sc,
			mag_scale m_sc,
			gyro_odr g_odr,
			accel_odr a_odr, 
			mag_odr m_odr);

// Reads raw gyro output registers for one axis: GYRO_X, GYRO_Y, or GYRO_Z
int16_t LSM_read_gyro_raw(uint8_t dimension);

// Reads DPS gyro output for one axis: GYRO_X, GYRO_Y, or GYRO_Z
// Relies on g_scale and g_res being correct
float LSM_read_gyro_dps(uint8_t dimension);

// Reads raw accel output registers for one axis: ACCEL_X, ACCEL_Y, or ACCEL_Z
// Relies on a_scale and a_res being correct
int16_t LSM_read_accel_raw(uint8_t dimension);

// Reads G accel output for one axis: ACCEL_X, ACCEL_Y, or ACCEL_Z
float LSM_read_accel_g(uint8_t dimension);

// Reads raw mag output registers for one axis: MAG_X, MAG_Y, or MAG_Z
int16_t LSM_read_mag_raw(uint8_t dimension);

// Reads Gauss mag output for one axis: MAG_X, MAG_Y, or MAG_Z
// Relies on m_scale and m_res being correct
float LSM_read_mag_gs(uint8_t dimension);

// Reads raw temperature output registers
int16_t LSM_read_temperature_raw();

// Reads C temperature output
// NO CALIBRATION YET
float LSM_read_temperature_C();

// Set full-scale range for gyroscope
// Can set G_SCALE_245DPS, G_SCALE_500DPS, or G_SCALE_2000DPS
void LSM_set_gyro_scale(gyro_scale g_sc);

// Set full-scale range for accelerometer
// Can set A_SCALE_2G, A_SCALE_4G, A_SCALE_8G
void LSM_set_accel_scale(accel_scale a_sc);

// Set full-scale range for magnetometer
// Can set M_SCALE_4GS, M_SCALE_8GS, M_SCALE_12GS, M_SCALE_16GS
void LSM_set_mag_scale(mag_scale m_sc);

// Set the output data rate for gyroscope
// Can set G_POWER_DOWN, G_ODR_14_9, G_ODR_59_5, G_ODR_119, G_ODR_238, G_ODR_476, G_ODR_952
void LSM_set_gyro_odr(gyro_odr g_odr);

// Set the output data rate for accelerometer
// Can set A_POWER_DOWN, A_ODR_10, A_ODR_50, A_ODR_119, A_ODR_238, A_ODR_476, A_ODR_952
void LSM_set_accel_odr(accel_odr a_odr);

// Set the output data rate for magnetometer
// Can set M_ODR_0_625, M_ODR_1_25, M_ODR_2_5, M_ODR_5, M_ODR_10, M_ODR_20, M_ODR_40, M_ODR_80
void LSM_set_mag_odr(mag_odr m_odr);

// Set the anti-aliasing filter rate of the accelerometer
// Can set A_ABW_408, A_ABW_211, A_ABW_105, A_ABW_50
void LSM_set_accel_abw(accel_abw a_abw);

// Configure gyro interrupt output
// int1_cfg = sets AND/OR and high/low interrupt gen for each axis
// int1_ths_x = 16 bit interrupt threshold value for x-axis
// int1_ths_y = 16 bit interrupt threshold value for y-axis
// int1_ths_z = 16 bit interrupt threshold value for z-axis
// duration = duration an interrupt holds after triggering
void LSM_configure_gyro_int(uint8_t int1_cfg, uint16_t int1_ths_x,
	uint16_t int1_ths_y, uint16_t int1_ths_z, uint8_t duration);

// Configure accel interrupt output
// int1_cfg = sets AND/OR and high/low interrupt gen for each axis
// int1_ths_x = 16 bit interrupt threshold value for x-axis
// int1_ths_y = 16 bit interrupt threshold value for y-axis
// int1_ths_z = 16 bit interrupt threshold value for z-axis
// duration = duration an interrupt holds after triggering
void LSM_configure_accel_int(uint8_t int1_cfg, uint8_t int1_ths_x,
	uint8_t int1_ths_y, uint8_t int1_ths_z, uint8_t duration);

// Configure mag interrupt output
// int1_cfg = sets AND/OR and high/low interrupt gen for each axis
// int1_ths = 16 bit interrupt threshold value
// duration = duration an interrupt holds after triggering
void LSM_configure_mag_int(uint8_t int1_cfg, uint16_t int1_ths);

// Calculate bias and scaling factors for all sensor calibrations
//
// This is a function that uses the FIFO to accumulate sample of accelerometer and gyro data, average
// them, scales them to  gs and deg/s, respectively, and then passes the biases to the main sketch
// for subtraction from all subsequent data. There are no gyro and accelerometer bias registers to store
// the data as there are in the ADXL345, a precursor to the LSM, or the MPU-9150, so we have to
// subtract the biases ourselves. This results in a more accurate measurement in general and can
// remove errors due to imprecise or varying initial placement. Calibration of sensor data in this manner
// is good practice.
// void LSM_calibrate(float gbias[3], float abias[3]);

// Sets up the gyroscope to begin reading
//	- LSM_CTRL_REG1_G = 0x38: 14.9 Hz ODR, 5 Hz cutoff, 2000 DPS, 00 BW_G[1:0]
//	- LSM_CTRL_REG2_G = 0x00: INT_SEL and OUT_SEL set to 00 and 00 (?)
//	- LSM_CTRL_REG3_G = 0x00: Low-power disabled, high-pass filter disabled 
//	- LSM_CTRL_REG4_G = 0x38: Output enabled on all axes
void LSM_init_gyro();

// Sets up the accelerometer to begin reading.
//	- LSM_CTRL_REG5_XL = 0x38: No decimation, output enabled on all axes
//  - LSM_CTRL_REG6_XL = 0xD8: 952 Hz ODR, 8g range, default BW
//  - LSM_CTRL_REG7_XL = 0x00: High resolution disabled, low and high pass filter disabled
void LSM_init_accel();

// Sets up the magnetometer to begin reading.
//  - LSM_CTRL_REG1_M = 0xC0: Temperature compensation enabled, high-performance XY, self test disabled
//  - LSM_CTRL_REG2_M = 0x00: 4 gauss full scale range, normal mode
//  - LSM_CTRL_REG3_M = 0x00: I2C enabled, low power mode disabled, continuous-conversion mode
//  - LSM_CTRL_REG4_M = 0x0C: High performance Z, data LSB at lower address
//  - LSM_CTRL_REG5_M = 0x00: Block data update disabled
void LSM_init_mag();

// Calculate the resolution of the gyroscope.
// This function will set the value of the g_res variable. g_scale must
// be set prior to calling this function.
void LSM_calc_g_res();

// Calculate the resolution of the accelerometer.
// This function will set the value of the a_res variable. a_scale must
// be set prior to calling this function.
void LSM_calc_a_res();

// Calculate the resolution of the magnetometer.
// This function will set the value of the m_res variable. m_scale must
// be set prior to calling this function.
void LSM_calc_m_res();

// Read a register on the accel/gyro device
int8_t LSM_read_reg_xlg(uint8_t reg_addr);

// Write a register on the accel/gyro device
void LSM_write_reg_xlg(uint8_t reg_addr, uint8_t data);

// Read a register on the magnetometer device
int8_t LSM_read_reg_mag(uint8_t reg_addr);

// Write a register on the magnetometer
void LSM_write_reg_mag(uint8_t reg_addr, uint8_t data);

#endif /* LSM_H */
