#ifndef L3G_H
#define L3G_H

#include "Sensor.h"
#include "../lxp11u6x/i2c_11u6x.h"
#include <math.h>

#define SA0_LOW_ADDRESS    (0xD4 >> 1)
#define SA0_HIGH_ADDRESS   (0xD6 >> 1)
#define L3GD20_WHO_ID_1		0xD4
#define L3GD20_WHO_ID_2		0xD7

// Dimensions
#define SPIN_RATE_X	1
#define SPIN_RATE_Y	2
#define SPIN_RATE_Z	3
#define TEMPERATURE	4

class L3G : public Sensor {
// From abstract base class - see Sensor.h
protected:
	int8_t read_reg(uint8_t reg_addr);
	int write_reg(uint8_t reg_addr, uint8_t reg_data);
public:
	bool init(void* in);
	// Dimensions are SPIN_RATE_X, SPIN_RATE_Y, SPIN_RATE_Z, and TEMPERATURE for this sensor
	float read_data(uint8_t dimension);
	// Unused so far
	void set_mode(void* mode);
	// Unused so far
	uint8_t get_status(uint8_t status);

// Device specific members
	enum reg_addr {
		WHO_AM_I		0x0F,
		CTRL_REG1		0x20,
		CTRL_REG2		0x21,
		CTRL_REG3		0x22,
		CTRL_REG4		0x23,
		CTRL_REG5		0x24,
		REFERENCE		0x25,
		OUT_TEMP		0x26,
		STATUS_REG		0x27,
		OUT_X_L			0x28,
		OUT_X_H			0x29,
		OUT_Y_L			0x2A,
		OUT_Y_H			0x2B,
		OUT_Z_L			0x2C,
		OUT_Z_H			0x2D,
		FIFO_CTRL_REG	0x2E,
		FIFO_SRC_REG	0x2F,
		INT1_CFG		0x30,
		INT1_SRC		0x31,
		INT1_THS_XH		0x32,
		INT1_THS_XL		0x33,
		INT1_THS_YH		0x34,
		INT1_THS_YL		0x35,
		INT1_THS_ZH		0x36,
		INT1_THS_ZL		0x37,
		INT1_DURATION	0x38,
		LOW_ODR			0x39
	}
	typedef struct vector {
		float x, y, z;
	} vector;
	//vector g; // gyro angular velocity readings
	L3G() {}
	uint8_t get_address() {return slave_address;}
	// Turns on sensor and enables continuous output
	void enable();
    // Reading data
    int16_t read_spin_rate_raw(uint8_t dimension);
    // May need calibration
    float read_spin_rate_dps(uint8_t dimension);
    uint8_t read_temperature_raw();
 	// Using same raw conversion as LPS sensor, probably needs calibration
    float read_temperature_C();
	// Vector functions
    //static void vector_cross(const vector *a, const vector *b, vector *out);
    //static float vector_dot(const vector *a,const vector *b);
    //static void vector_normalize(vector *a);
private:
	I2C_ID_T i2c_id;
	uint8_t slave_address;
	bool detect_device();
};

#endif /* L3G_H */
