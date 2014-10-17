#ifndef LSM303_H
#define LSM303_H

#include "Sensor.h"
#include "../lxp11u6x/i2c_11u6x.h"
#include <math.h>

#define SA0_HIGH_ADDRESS	0b0011101 // D with SA0 high
#define SA0_LOW_ADDRESS		0b0011110 // D with SA0 low
#define LSM303D_WHO_ID		0x49

// Dimensions
#define ACCEL_X		1
#define ACCEL_Y		2
#define ACCEL_Z		3
#define MAG_X		4
#define MAG_Y		5
#define MAG_Z		6
#define MAG_HEADING 7
#define TEMPERATURE	8

class LSM303 : public Sensor {
// From abstract base class - see Sensor.h
protected:
	int8_t read_reg(uint8_t reg_addr);
	int write_reg(uint8_t reg_addr, uint8_t data);
public:
	bool init(void* in);
	// Dimensions are ACCEL_X, ACCEL_Y, ACCEL_Z, MAG_X, MAG_Y, MAG_Z, MAG_HEADING and TEMPERATURE for this sensor
	float read_data(uint8_t dimension);
	// Unused so far
	void set_mode(void* mode);
	// Unused so far
	uint8_t get_status(uint8_t status);

// Device specific members
	enum reg_addr {
		TEMP_OUT_L		0x05,
		TEMP_OUT_H		0x06,
		STATUS_M		0x07,
		OUT_X_L_M		0x08,
		OUT_X_H_M		0x09,
		OUT_Y_L_M		0x0A,
		OUT_Y_H_M		0x0B,
		OUT_Z_L_M		0x0C,
		OUT_Z_H_M		0x0D,
		WHO_AM_I		0x0F,
		INT_CTRL_M		0x12,
		INT_SRC_M		0x13,
		INT_THS_L_M		0x14,
		INT_THS_H_M		0x15,
		OFFSET_X_L_M	0x16,
		OFFSET_X_H_M	0x17,
		OFFSET_Y_L_M	0x18,
		OFFSET_Y_H_M	0x19,
		OFFSET_Z_L_M	0x1A,
		OFFSET_Z_H_M	0x1B,
		REFERENCE_X		0x1C,
		REFERENCE_Y		0x1D,
		REFERENCE_Z		0x1E,
		CTRL_REG0		0x1F,
		CTRL_REG1		0x20,
		CTRL_REG2		0x21,
		CTRL_REG3		0x22,
		CTRL_REG4		0x23,
		CTRL_REG5		0x24,
		CTRL_REG6		0x25,
		CTRL_REG7		0x26,
		STATUS_A		0x27,
		OUT_X_L_A		0x28,
		OUT_X_H_A		0x29,
		OUT_Y_L_A		0x2A,
		OUT_Y_H_A		0x2B,
		OUT_Z_L_A		0x2C,
		OUT_Z_H_A		0x2D,
		FIFO_CTRL		0x2E,
		FIFO_SRC		0x2F,
		IG_CFG1			0x30,
		IG_SRC1			0x31,
		IG_THS1			0x32,
		IG_DUR1			0x33,
		IG_CFG2			0x34,
		IG_SRC2			0x35,
		IG_THS2			0x36,
		IG_DUR2			0x37,
		CLICK_CFG		0x38,
		CLICK_SRC		0x39,
		CLICK_THS		0x3A,
		TIME_LIMIT		0x3B,
		TIME_LATENCY	0x3C,
		TIME_WINDOW		0x3D,
		Act_THS			0x3E,
		Act_DUR			0x3F
	}
	template <typename T> struct vector{
      T x, y, z;
    };
	LSM303() {}
	uint8_t get_address() {return slave_address;}
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
	void enable();
	void set_timeout(unsigned int timeout);
	unsigned int get_timeout();
	//bool timeout_occured();
	int16_t read_accel_raw(uint8_t dimension);
	// May need calibration
	float read_accel_g(uint8_t dimension);
	int16_t read_mag_raw(uint8_t dimension);
	//float read_mag_gauss(uint8_t dimension);
	int16_t read_temperature_raw();
 	// Using same raw conversion as LPS sensor, probably needs calibration
	float read_temperature_C();
	/*
	Returns the angular difference in the horizontal plane between a
	default vector and north, in degrees.

	The default vector here is chosen to point along the surface of the
	PCB, in the direction of the top of the text on the silkscreen.
	This is the +X axis on the Pololu LSM303D carrier and the -Y axis on
	the Pololu LSM303DLHC, LSM303DLM, and LSM303DLH carriers.
	*/
	float read_mag_heading();
	/*
	Returns the angular difference in the horizontal plane between the
	"from" vector and north, in degrees.
	*/
	template <typename T> float heading(vector<T> from);

	// Vector functions
    template <typename Ta, typename Tb, typename To> static void vector_cross(const vector<Ta> *a, const vector<Tb> *b, vector<To> *out);
    template <typename Ta, typename Tb> static float vector_dot(const vector<Ta> *a,const vector<Tb> *b);
    static void vector_normalize(vector<float> *a);
private:
	I2C_ID_T i2c_id;
	uint8_t slave_address;
	//unsigned int io_timeout;
	//bool did_timeout;
	bool detect_device();
};

#endif /* LSM303_H */
