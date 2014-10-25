#ifndef SENSOR_H
#define SENSOR_H

#include <cstdint>

class Sensor {
protected:

    // Read a one byte register at the specified address in a sensor.
    virtual int8_t read_reg(uint8_t reg_addr) = 0;

    // Write one byte to the register specified by addr in a sensor.
    virtual void write_reg(uint8_t reg_addr, int8_t data) = 0;

public:

    // Perform all initialization steps associated with a specific sensor.
    // This will likely make use of write_reg() to send sensor-specific
    // command bytes.
    virtual bool init(void* in) = 0;

    // Read the full value of the data stored in a sensor, for one dimension.
    // This dimension could be X, Y, or Z for a gyro/accelerometer,
    // altitude or pressure for the barometric pressure sensor, etc.
    // Note: this is already calibrated, returns float.
    virtual float read_data(uint8_t dimension) = 0;

    // Configure mode options available to the sensor. Each sensor will
    // interpret the input to set all of its various modes. This function
    // will make use of write_reg() to set control registers.
    virtual void set_mode(void* mode) = 0;

    // Read a status about a sensor, such as data overrun, new data available,
    // etc. The status input variable will be a #DEFINE status register address 
    // specific to each sensor.
    virtual uint8_t get_status(uint8_t status) = 0;

};

#endif /* SENSOR_H */