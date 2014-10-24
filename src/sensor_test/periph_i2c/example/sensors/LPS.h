#ifndef LPS_H
#define LPS_H

#include "i2c_11u6x.h"
#include <stdint.h>
#include <math.h>

#define LPS_SA0_LOW_ADDRESS		(0xB8)
#define LPS_SA0_HIGH_ADDRESS	(0xBA)
#define LPS331AP_WHO_ID			0xBB

// Registers
#define LPS_REF_P_XL		 0x08
#define LPS_REF_P_L 		 0x09
#define LPS_REF_P_H 		 0x0A
#define LPS_WHO_AM_I		 0x0F
#define LPS_RES_CONF		 0x10
#define LPS_CTRL_REG1		 0x20
#define LPS_CTRL_REG2		 0x21
#define LPS_CTRL_REG3		 0x22
#define LPS_INT_CFG_REG		 0x23
#define LPS_INT_SOURCE_REG 	 0x24
#define LPS_THS_P_LOW_REG	 0x25
#define LPS_THS_P_HIGH_REG 	 0x26
#define LPS_STATUS_REG		 0x27
#define LPS_OUT_PRESS_XL	 0x28
#define LPS_OUT_PRESS_L 	 0x29
#define LPS_OUT_PRESS_H 	 0x2A
#define LPS_OUT_TEMP_L 		 0x2B
#define LPS_OUT_TEMP_H  	 0x2C
#define LPS_AMP_CTRL		 0x30 // Typos in datasheet this may be 0x2D
#define LPS_RPDS_L			 0x39
#define LPS_RPDS_H			 0x3A
#define LPS_DELTA_PRESS_XL 	 0x3C
#define LPS_DELTA_PRESS_L 	 0x3D
#define LPS_DELTA_PRESS_H	 0x3E

// Dimensions
#define LPS_ALTITUDE 		1
#define LPS_TEMPERATURE 	2

I2C_ID_T LPS_i2c_id;
uint8_t LPS_slave_address;

int LPS_init(I2C_ID_T id_in);

// Dimensions are LPS_ALTITUDE and LPS_TEMPERATURE for this sensor
float LPS_read_data(uint8_t dimension);

// Turns on sensor and enables continuous output
void LPS_enable();

// Read data
int32_t LPS_read_pressure_raw();
float LPS_read_pressure_millibars();
int16_t LPS_read_temperature_raw();
float LPS_read_temperature_C();
// Formula only applies to 11 km / 36000 ft
float LPS_pressure_to_altitude_m(float pressure_mbar, float altimeter_setting_mbar);

// Low level register work
uint8_t LPS_read_reg(uint8_t reg_addr);
void LPS_write_reg(uint8_t reg_addr, uint8_t data);

// Unused
//bool detect_device();

#endif /* LPS_H */
