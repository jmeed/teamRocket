#ifndef LSM303_H
#define LSM303_H

#include "Sensor.h"
#include "../lxp11u6x/i2c_11u6x.h"
#include <math.h>

// Dimensions


class LSM303 : public Sensor {
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

	}

};

#endif /* LSM303_H */
