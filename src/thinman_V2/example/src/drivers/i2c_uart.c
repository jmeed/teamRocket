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
#include "logging.h"

typedef uint8_t byte;
typedef int8_t int8;
typedef int16_t int1;

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
#define I2CWRITE     0x00
#define I2CREAD      0x01

#define CHANA      0
#define CHANB      1

#define UART_ADDR 0x90


//
//***********************************************
byte ReadUART(int8 RegAddr, int1 CHAN)   // Internal register address plus channel #(0 or 1)
   { // returns byte read from the UART register
   byte  data;
   //
   data = (RegAddr << 3) | (CHAN << 1);
   Chip_I2C_MasterSend(I2C1, UART_ADDR >> 1, &data, 1);
   Chip_I2C_MasterRead(I2C1, UART_ADDR >> 1, &data, 1);
   return(data);
   }
 //
 //*********************************************
void WriteUART(int8 RegAddr, int1 CHAN, byte Data) // Internal register address plus channel #(0 or 1)
   { // sends data byte to selected UART register

	uint8_t buffer[2];
	buffer[0] = (RegAddr << 3) | (CHAN << 1);
	buffer[1] = Data;
    Chip_I2C_MasterSend(I2C1, UART_ADDR >> 1, buffer, 2);
   }
//
//*********************************************
void UART_Send_Char(int1 CHAN, byte Data) //channel #(0 or 1) plus the data byte to be sent
{ // send byte to UART Xmit via the I2C bus
     WriteUART(THR, CHAN, Data);  // send data to UART Transmit Holding Register
}

void UART_Send_String(int1 CHAN, const char* str) {
	int i;
	for (i = 0; i < strlen(str); i++) {
		UART_Send_Char(CHAN, str[i]);
	}
}

uint8_t UART_Read_TXLVL(int1 CHAN) {
	return ReadUART(TXLVL, CHAN);
}
//
//*******************************************************
void Init_SC16IS752 (void)
  {
  // This init routine initializes ChannelS A and B
  //
  // Channel A Setups
  //Prescaler in MCR defaults on MCU reset to the value of 1
  WriteUART(LCR, CHANA, 0x80); // 0x80 to program baud rate divisor
  WriteUART(DLL, CHANA, 0x4e); // 0x4e = 9600baud // 0x18=9600K, 0x06 =38,42K with X1=3.6864MHz
  WriteUART(DLH, CHANA, 0x00); //
//
  WriteUART(LCR, CHANA, 0xBF); // access EFR register
  WriteUART(EFR, CHANA, 0X10); // enable enhanced registers
  //
  WriteUART(LCR, CHANA, 0x03); // 8 data bits, 1 stop bit, no parity
  WriteUART(FCR, CHANA, 0x07); // reset TXFIFO, reset RXFIFO, enable FIFO mode

  // Channel B Setups
  //Prescaler in MCR defaults on MCU reset to the value of 1
  WriteUART(LCR, CHANB, 0x80); // 0x80 to program baud rate divisor
  WriteUART(DLL, CHANB, 0x4e); // 0x18=9600K, 0x06 =38,42K with X1=3.6864MHz
  WriteUART(DLH, CHANB, 0x00); //
//
  WriteUART(LCR, CHANB, 0xBF); // access EFR register
  WriteUART(EFR, CHANB, 0X10); // enable enhanced registers
  //
  WriteUART(LCR, CHANB, 0x03); // 8 data bits, 1 stop bit, no parity
  WriteUART(FCR, CHANB, 0x07); // reset TXFIFO, reset RXFIFO, enable FIFO mode
  }
//
//*********************************************
char Poll_UART_RHR(CHAN)
   { // Poll UART to determine if data is waiting
     char data = 0x00;
//
     if (ReadUART(LSR, CHAN) & 0x01) // is data waiting??
        { // data present in receiver FIFO
             data = ReadUART(RHR, CHAN);
         }
// return received char or zero
     return(data);
    }
//
//*********************************************
void Set_GPIO_Dir(bits)
   { // Set Direction on UART GPIO Port pins GPIO0 to GPIO7
     // 0=input   1=Output
     WriteUART(IOControl, 0, 0x03); // Set the IOControl Register to GPIO Control
     WriteUART(IODir,0, bits); // output the control bits to the IO Direction Register
    }
//*********************************************
byte Read_GPIO()
   { // Read UART GPIO Port
     char data = 0x00;
//
     data=ReadUART(IOState,0); // get GPIO Bits state 0-7

// return data bits state or zero
     return(data);
    }
//
//*********************************************
void Write_GPIO(data)
   { // Load UART GPIO Port
     WriteUART(IOState,0, data); // set GPIO Output pins state 0-7
    }
//
//*********************************************
