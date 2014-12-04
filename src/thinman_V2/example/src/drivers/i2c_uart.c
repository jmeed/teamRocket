////////////////////////////////////////////////////////////////////////////////
// SC16IS752 UART Routines Library
//
// Filename     : SC16IS752 UART I2C Routines.c
// Programmer   : Dave Yeatman
// Version      : 10/27/2008
// Remarks      : This example provides framework code to communicate with
//                the NXP SC16IS752 Dual Channel UART using I2C. This UART driver
//                has been tested for both UART Channel 0 (A) and Channel 1 (B).
//                The GPIO I/O functions have also been tested.
//
//                More information on the UART can be found at www.NXP.com:
////////////////////////////////////////////////////////////////////////////////
//
// SC16IS752 Dual UART Register Defines
//

#include <Chip.h>
#include <stdint.h>
#include <string.h>
#include "./i2c_uart.h"
#include "logging.h"

#define RHR          0x00 //  Recv Holding Register is 0x00 in READ Mode
#define THR          0x00 //  Xmit Holding Register is 0x00 in WRITE Mode
//
#define IER          0x01  // Interrupt Enable Register
//
#define IIR          0x02  // Interrupt Identification Register in READ Mode
#define FCR          0x02  // FIFO Control Register in WRITE Mode
//
#define LCR          0x03  // Line Control Register
#define MCR          0x04  // Modem Control Register
#define LSR          0x05  // Line status Register
#define MSR          0x06  // Modem Status Register
#define SPR          0x07  // ScratchPad Register
#define TCR          0x06  // Transmission Control Register
#define TLR          0x07  // Trigger Level Register
#define TXLVL        0x08  // Xmit FIFO Level Register
#define RXLVL        0x09  // Recv FIFO Level Register
#define IODir        0x0A  // I/O P:ins Direction Register
#define IOState      0x0B  // I/O Pins State Register
#define IOIntEna     0x0C  // I/O Interrupt Enable Register
#define IOControl    0x0E  // I/O Pins Control Register
#define EFCR         0x0F  // Extra Features Control Register
//
#define DLL          0x00  // Divisor Latch LSB  0x00
#define DLH          0x01  // Divisor Latch MSB  0x01
//
#define EFR          0x02  // Enhanced Function Register
//
bool i2c_uart_transmit_error;
//
//***********************************************
static uint8_t i2c_uart_read_reg(uint8_t RegAddr, i2c_uart_channel_t CHAN)   // Internal register address plus channel #(0 or 1)
{ // returns byte read from the UART register
	uint8_t data, buffer;
	data = (RegAddr << 3) | (CHAN << 1);
	if (Chip_I2C_MasterCmdRead(I2C_UART_I2C_ID, UART_ADDR >> 1, data, &buffer, 1) == 0) {
		i2c_uart_transmit_error = true;
		return 0;
	}
	return buffer;
}
 //
 //*********************************************
static void i2c_uart_write_reg(uint8_t RegAddr, i2c_uart_channel_t CHAN, uint8_t Data) // Internal register address plus channel #(0 or 1)
{ // sends data byte to selected UART register

	uint8_t buffer[2];
	buffer[0] = (RegAddr << 3) | (CHAN << 1);
	buffer[1] = Data;
    if (Chip_I2C_MasterSend(I2C_UART_I2C_ID, UART_ADDR >> 1, buffer, 2) == 0) {
    	i2c_uart_transmit_error = true;
    }
}
//
//*********************************************
void i2c_uart_send_byte(i2c_uart_channel_t CHAN, uint8_t Data) //channel #(0 or 1) plus the data byte to be sent
{ // send byte to UART Xmit via the I2C bus
     i2c_uart_write_reg(THR, CHAN, Data);  // send data to UART Transmit Holding Register
}

void i2c_uart_send_string(i2c_uart_channel_t CHAN, const char* str) {
	int i;
	for (i = 0; i < strlen(str); i++) {
		i2c_uart_send_byte(CHAN, str[i]);
	}
}

uint8_t i2c_uart_get_tx_free(i2c_uart_channel_t CHAN) {
	return i2c_uart_read_reg(TXLVL, CHAN);
}
//
//*******************************************************
bool i2c_uart_init(void)
{
  // This init routine initializes ChannelS A and B
  //
  // Channel A Setups
  //Prescaler in MCR defaults on MCU reset to the value of 1
  i2c_uart_transmit_error = false;
  i2c_uart_write_reg(LCR, I2C_UART_CHANA, 0x80); // 0x80 to program baud rate divisor
  i2c_uart_write_reg(DLL, I2C_UART_CHANA, 0x4e); // 0x4e = 9600baud // 0x18=9600K, 0x06 =38,42K with X1=3.6864MHz
  i2c_uart_write_reg(DLH, I2C_UART_CHANA, 0x00); //
//
  i2c_uart_write_reg(LCR, I2C_UART_CHANA, 0xBF); // access EFR register
  i2c_uart_write_reg(EFR, I2C_UART_CHANA, 0X10); // enable enhanced registers
  //
  i2c_uart_write_reg(LCR, I2C_UART_CHANA, 0x03); // 8 data bits, 1 stop bit, no parity
  i2c_uart_write_reg(FCR, I2C_UART_CHANA, 0x07); // reset TXFIFO, reset RXFIFO, enable FIFO mode

  // Channel B Setups
  //Prescaler in MCR defaults on MCU reset to the value of 1
  i2c_uart_write_reg(LCR, I2C_UART_CHANB, 0x80); // 0x80 to program baud rate divisor
  i2c_uart_write_reg(DLL, I2C_UART_CHANB, 0x4e); // 0x18=9600K, 0x06 =38,42K with X1=3.6864MHz
  i2c_uart_write_reg(DLH, I2C_UART_CHANB, 0x00); //
//
  i2c_uart_write_reg(LCR, I2C_UART_CHANB, 0xBF); // access EFR register
  i2c_uart_write_reg(EFR, I2C_UART_CHANB, 0X10); // enable enhanced registers
  //
  i2c_uart_write_reg(LCR, I2C_UART_CHANB, 0x03); // 8 data bits, 1 stop bit, no parity
  i2c_uart_write_reg(FCR, I2C_UART_CHANB, 0x07); // reset TXFIFO, reset RXFIFO, enable FIFO mode

  return !i2c_uart_transmit_error;
}
//
//*********************************************
int i2c_uart_readc(i2c_uart_channel_t CHAN)
{ // Poll UART to determine if data is waiting
	if (i2c_uart_read_reg(LSR, CHAN) & 0x01) // is data waiting??
	{ // data present in receiver FIFO
		return i2c_uart_read_reg(RHR, CHAN);
	}
	return -1;
}
//
//*********************************************
void i2c_uart_set_gpio_direction(uint8_t bits)
{ // Set Direction on UART GPIO Port pins GPIO0 to GPIO7
	// 0=input   1=Output
	i2c_uart_write_reg(IOControl, 0, 0x03); // Set the IOControl Register to GPIO Control
	i2c_uart_write_reg(IODir, 0, bits); // output the control bits to the IO Direction Register
}
//*********************************************
uint8_t i2c_uart_read_gpio()
{ // Read UART GPIO Port
     return i2c_uart_read_reg(IOState, 0); // get GPIO Bits state 0-7
}
//
//*********************************************
void i2c_uart_write_gpio(uint8_t data)
{ // Load UART GPIO Port
	i2c_uart_write_reg(IOState, 0, data); // set GPIO Output pins state 0-7
}
//
//*********************************************
