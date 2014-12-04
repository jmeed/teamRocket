/*
 * i2c_uart.h
 *
 *  Created on: Dec 3, 2014
 *      Author: Max Zhao
 */

#ifndef I2C_UART_H_
#define I2C_UART_H_

typedef enum {
	I2C_UART_CHANA = 0,
	I2C_UART_CHANB = 1,
} i2c_uart_channel_t;

#define UART_ADDR 0x90
#define I2C_UART_I2C_ID I2C1

extern bool i2c_uart_transmit_error;

void i2c_uart_send_byte(i2c_uart_channel_t CHAN, uint8_t Data);
void i2c_uart_send_string(i2c_uart_channel_t CHAN, const char* str);
uint8_t i2c_uart_get_tx_free(i2c_uart_channel_t CHAN);
bool i2c_uart_init(void);
int i2c_uart_readc(i2c_uart_channel_t CHAN);
void i2c_uart_set_gpio_direction(uint8_t bits);
uint8_t i2c_uart_read_gpio();
void i2c_uart_write_gpio(uint8_t data);

#endif /* I2C_UART_H_ */
