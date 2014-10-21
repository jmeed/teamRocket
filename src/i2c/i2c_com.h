/*
 * i2c_com.h
 *
 *  Created on: Apr 13, 2013
 *      Author: ftheo
 */

#ifndef I2C_COM_H_
#define I2C_COM_H_

#include <stdint.h>

void send_i2c_msg(uint8_t w_addr, uint8_t reg, uint8_t length);
void write_i2c_register(uint8_t w_addr, uint8_t reg, uint8_t value);
uint8_t read_i2c_register(uint8_t r_addr, uint8_t w_addr, uint8_t reg);


#endif /* I2C_COM_H_ */
