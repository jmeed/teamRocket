/*!
 *\file	spi_routines.h	
 *\brief	This file contains macros for SPI interface and global
 *			function declaration used by spi_routines.c file
 */
/*				 
 *			ZILOG DISCLAIMER
 *
 *			THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *			EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *			OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *			NONINFRINGEMENT. 
 *			IN NO EVENT SHALL ZILOG, INC BE LIABLE FOR ANY CLAIM, DAMAGES
 *			OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 *			OTHERWISE,ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 *			SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *			Copyright (C) 2006 by  ZILOG, Inc.
 *			All Rights Reserved.
 */

/* System includes */

#include <ez8.h>

/* Macro */

/*	Mode of operation of SPI. This code is designed for SPI mode of operation
	CPOL = 0 and CPHA = 1.*/

#define CPOL		LOW
#define CPHA		HIGH

/* This is a user configurable area. User can select any of the port pins 
	available on micro-controller except few as MOSI, MISO and SS. 
	For that user needs to add the information in below configurations. 
	For example if user is selecting pins from PORTC to function as MOSI/MISO/SS
	then he/she has to change the port pin number in following definition.
	If user is selecting PORTA with pin number as same as port c, then he/she
	has to change port address, port control,port input, port output defintion.*/

//MISO pin definition
#define MISO_PORT_ADDR	PCADDR				
#define MISO_PORT_CNTRL	PCCTL
#define MISO_PORT_IN		PCIN
#define MISO_PORT_PIN	PIN7

//MOSI pin definition
#define MOSI_PORT_ADDR	PCADDR
#define MOSI_PORT_CNTRL	PCCTL
#define MOSI_PORT_OUT	PCOUT
#define MOSI_PORT_PIN	PIN3
#define MOSI_PORT_PIN_DEFAULT	HIGH

//SS pin definition
#define SS_PORT_ADDR		PCADDR
#define SS_PORT_CNTRL	PCCTL
#define SS_PORT_OUT		PCOUT
#define SS_PORT_PIN		PIN1
#define SS_PORT_PIN_DEFAULT	HIGH

//CLK pin definition
#define CLK_PORT_ADDR		PCADDR
#define CLK_PORT_CNTRL		PCCTL
#define CLK_PORT_OUT		PCOUT
#define CLK_PORT_PIN		PIN2
#define CLK_PORT_PIN_DEFAULT	LOW

/* Port pin Mask */

#define PIN0		0x01
#define PIN1		0x02
#define PIN2		0x04
#define PIN3		0x08
#define PIN4		0x10
#define PIN5		0x20
#define PIN6		0x40
#define PIN7		0x80

#define HIGH		0xFF
#define LOW			0x00
#define WREN		0x06
#define WRITE		0x02
#define READ		0x03

#define I_O_FUNCTION		0x01;

/* Configuration data for SPI slave */

#define CONFIG_ADDR	0x80
#define CONFIG_DATA	0xE0						/*	A control byte needs to be written to
															configuration address of slave (DS1722)
														 	to set it to, continuous mode and 8 bit
															resolution. */

/* Address from which temperature value can be read from slave (DS1772) */ 

#define ADDR_01 0x01
#define ADDR_02 0x02

/* global functions declaration */

/*!
 *\brief	This function initializes SPI master.
 *
 */
void Init_SPI_Master( void );

/*!
 *\brief	This function initializes the SPI slave.
 *
 */
//void Init_SPI_Slave( void );

/*!
 *\brief	This function accepts data and address to perform write operation.
 *
 */
//void Write_SPI( int,int );

/*!
 *\brief	This function writes a list of up to 4 ints to SPI
 *
 */
//void Write_list_SPI( int,int,int[4] );

/*!
 *\brief	This function reads character from MISO line.
 *
 */
//int Read_SPI(int );

/*!
 *\brief	This function makes slave select line to active state.
 *
 */
void Assert_SS( void );

/*!
 *\brief	This function makes slave select line deactive state.
 *
 */
void Deassert_SS( void );

/*!
 *\brief	This function writes a character onto MOSI line.
 *
 */
//void Write_Byte( unsigned char );

/*!
 *\brief	This function starts SPI clock generation.
 *
 */
//void Start_CLK( void );

/*!
 *\brief	This function stops SPI clock generation.
 *
 */
//void Stop_CLK( void );

//int Shift_Bits( int data, char num_bytes );

/*!
 *\brief	Replaces timer and ISR for banging SPI bits
 *
 */
long Shift_Bytes(long data, char num_bytes);

unsigned char Shift_One_Byte( char data );
/*!
 *\brief	Replaces timer and ISR for banging SPI bits
 *
 */

/* End of File */

/* End of File */
