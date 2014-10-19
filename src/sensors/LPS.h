#ifndef LPS_H
#define LPS_H

#include "Sensor.h"
#include "../lxp11u6x/i2c_11u6x.h"

#define SA0_LOW_ADDRESS		0b1011100
#define SA0_HIGH_ADDRESS	0b1011101
#define LPS331AP_WHO_ID		0xBB

// Dimensions
#define ALTITUDE 		1
#define TEMPERATURE 	2

class LPS : public Sensor {
// From abstract base class - see Sensor.h
protected:
	int8_t read_reg(uint8_t reg_addr);
	int write_reg(uint8_t reg_addr, uint8_t data);
public:
	bool init(void* in);
	// Dimensions are ALTITUDE and TEMPERATURE for this sensor
	float read_data(uint8_t dimension);
	// Unused so far
	void set_mode(void* mode);
	// Unused so far
	uint8_t get_status(uint8_t status);

// Device specific members
	enum reg_addr {
		REF_P_XL		= 0x08,
		REF_P_L 		= 0x09,
		REF_P_H 		= 0x0A,
		WHO_AM_I		= 0x0F,
		RES_CONF		= 0x10,
		CTRL_REG1		= 0x20,
		CTRL_REG2		= 0x21,
		CTRL_REG3		= 0x22,
		INT_CFG_REG		= 0x23,
		INT_SOURCE_REG 	= 0x24,
		THS_P_LOW_REG	= 0x25,
		THS_P_HIGH_REG 	= 0x26,
		STATUS_REG		= 0x27,
		OUT_PRESS_XL	= 0x28,
		OUT_PRESS_L 	= 0x29,
		OUT_PRESS_H 	= 0x2A,
		OUT_TEMP_L 		= 0x2B,
		OUT_TEMP_H  	= 0x2C,
		AMP_CTRL		= 0x30, // Typos in datasheet, this may be 0x2D
		RPDS_L			= 0x39,
		RPDS_H			= 0x3A,
		DELTA_PRESS_XL 	= 0x3C,
		DELTA_PRESS_L 	= 0x3D,
		DELTA_PRESS_H	= 0x3E
	}
	LPS() {}
	uint8_t get_address() {return slave_address;}
	// Turns on sensor and enables continuous output
	void enable();
	int32_t read_pressure_raw();
	float read_pressure_millibars();
	int16_t read_temperature_raw();
	float read_temperature_C();
	// Formula only applies to 11 km / 36000 ft
	static float pressure_to_altitude_m(float pressure_mbar, float altimeter_setting_mbar = 1013.25);
private:
	I2C_ID_T i2c_id;
	uint8_t slave_address;
	bool detect_device();
};

#endif /* LPS_H */
