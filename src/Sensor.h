#ifndef SENSOR_H
#define SENSOR_H

#include <cstdint>

class Sensor {
protected:

    // Read a one byte register at the specified address in a sensor.
    virtual int8_t read_reg(uint8_t addr) = 0;

    // Write one byte to the register specified by addr in a sensor.
    virtual int8_t write_reg(uint8_t addr, uint8_t data) = 0;

public:

    // Perform all initialization steps associated with a specific sensor.
    // This will likely make use of write_reg() to send sensor-specific
    // command bytes.
    virtual void init() = 0;

    // Read the full value of the data stored in a sensor, for one dimension.
    // This dimension could be X, Y, or Z for a gyro/accelerometer,
    // altitude or pressure for the barometric pressure sensor, etc.
    //
    // Note: this returns a full 32-bit int. For a sensor with 16 bit output, 
    // this function must call read_reg() twice, and combine the two int8_ts
    // into a int32_t, with sign extension. Similarly, a 24 bit sensor must
    // call read_reg() three times for each data point.
    virtual int32_t read_data(uint8_t dimension) = 0;

    // Configure mode options available to the sensor. The input variables
    // mode_n will be #DEFINE statements specific to each sensor. This function
    // will make use of write_reg() to set control registers.
    //
    // Note: There are 2 extra optional arguments available for sensors that have
    // a large number of different aspects that can be configured. For example,
    // an accelerometer could use mode_0 to set the power mode, mode_1 to set the
    // data-rate, mode_2 to set which axes are enabled, and mode_3 to configure
    // interrupts. If more variables are needed, encode multiple options into one
    // mode_n variable, add more mode_n inputs, or directly use write_reg().
    virtual void set_mode(uint8_t mode_0, uint8_t mode_1 = 0, uint8_t mode_2 = 0
        uint8_t mode_3 = 0, uint8_t mode_4 = 0) = 0;

    // Read a status about a sensor, such as data overrun, new data available,
    // etc. The status input variable will be a #DEFINE status register address 
    // specific to each sensor.
    virtual uint8_t get_status(uint8_t status) = 0;

};

#endif /* SENSOR_H */
