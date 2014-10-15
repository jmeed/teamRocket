#ifndef LPS_H
#define LPS_H

#include "Sensor.h"

#define SA0_LOW_ADDRESS		0b1011100
#define SA0_HIGH_ADDRESS	0b1011101
#define LPS331AP_WHO_ID		0xBB
#define TEST_REG_NACK		-1

class LPS : public Sensor {

// From abstract base class - see Sensor.h
protected:
	int8_t read_reg(uint8_t addr);
	int8_t write_reg(uint8_t addr, uint8_t data);
public:
	bool init();
	int32_t read_data(uint8_t dimension);
	void set_mode(void* mode);
	uint8_t get_status(uint8_t status);

// Other members
	enum regAddr {
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
		PRESS_OUT_XL	= 0x28,
		PRESS_OUT_L 	= 0x29,
		PRESS_OUT_H 	= 0x2A,
		TEMP_OUT_L 		= 0x2B,
		TEMP_OUT_H  	= 0x2C,
		AMP_CTRL		= 0x30, // Typos in datasheet, this may be 0x2D
		RPDS_L			= 0x39,
		RPDS_H			= 0x3A,
		DELTA_PRESS_XL 	= 0x3C,
		DELTA_PRESS_L 	= 0x3D,
		DELTA_PRESS_H	= 0x3E,
	}

	LPS();
	uint8_t get_address() {return address;}
	// Turns on sensor and enables continuous output
	void enable_default();
	int32_t read_pressure_raw();
	float read_pressure_millibars();
	int16_t read_temperature_raw();
	float read_temperature_C();
	static float pressure_to_altitude_meters(float pressure_mbar, float altimeter_setting_mbar = 1013.25);
private:
	uint8_t address;
	bool detect_device();
	//int testWhoAmI(uint8_t address);
};

#endif /* LPS_H */
