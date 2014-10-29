/*!
 *\file	spi_routines.c	
 *\brief	This file contains routines related to SPI interface.
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

/* Local Includes */

#include "spi_routines.h"
//#include "global_defintion.h"
#include "timer.h"

/* Global variable declaration */

char g_Bit_Counter;									/*!< Variable used to count the bits
																transmitted or received on SPI lines */
unsigned char g_SPI_Data_Write;					/*!<	This variable will hold address/data 
																to be written on SPI interface */								
unsigned char g_SPI_Data_Read;					/*!<	This variable holds the data read
																on SPI interface */
char g_Operation_Complete;							/*!<	This variable indicates whether the 
																SPI operation(read/write)is complete. */
char g_State_High;									/*!<	This variable is used for transmission
																of data bits on high state of timer1 out*/
//extern int write_list[4];															 	

/*!
 *		This function configures MASTER controller's port pin, to function as SPI.
 *		MOSI (Master output slave input) pin, CLK (clock) pin, SS(slave select)pin
 *		are configured as output. MISO (master input slave output) pin is configured
 *		as input pin. Timer1 is confiugred for clock generation. Configuration is
 *		defined in spi_routines.h file.
 */

void Init_SPI_Master( void )
{
	MISO_PORT_ADDR = I_O_FUNCTION;						/* selecting data direction for 
																		MISO pin */
	MISO_PORT_CNTRL |= (MISO_PORT_PIN);					/* configuring MSIO port pin as 
																		input */
	MISO_PORT_ADDR = FALSE;									

	MOSI_PORT_ADDR = I_O_FUNCTION;						/* selecting data direction for 
																		MOSI pin */
	MOSI_PORT_CNTRL &= ~(MOSI_PORT_PIN);				/* configuring MOSI port pin as
																		output */
	MOSI_PORT_ADDR = FALSE;

	SS_PORT_ADDR	= I_O_FUNCTION;						/* selecting data direction for 
																		SS pin */	
	SS_PORT_CNTRL	 &= ~(SS_PORT_PIN);					/* configuring SS port pin as
																		output */
	SS_PORT_ADDR	= FALSE;

#if ( MOSI_PORT_PIN_DEFAULT == HIGH )

	MOSI_PORT_OUT	|=	MOSI_PORT_PIN;						/* Default status of MOSI pin is
																		HIGH.  */
#else

	MOSI_PORT_OUT	&=	MOSI_PORT_PIN;						/* Default status of MOSI pin is
																		LOW.  */
#endif

#if ( SS_PORT_PIN_DEFAULT == HIGH )

	SS_PORT_OUT		|= SS_PORT_PIN;						/* Default status of SS pin is
																		HIGH .*/

#else
	SS_PORT_OUT		&= ~SS_PORT_PIN;						/* Default status of SS pin is
																		LOW . */
#endif

	//Init_SPI_Clock();											/* Initialize timer 1 to generate
																			//clock */
}



void Assert_SS( void )
{
#if ( MOSI_PORT_PIN_DEFAULT == HIGH )					/* default status of MOSI port pin
																		is HIGH as defined in spi_routines.h
																		file */
	
	MOSI_PORT_OUT	|=	MOSI_PORT_PIN;						/* Changing to default status after
																		operation is complete. */

#else																/* default status of MOSI port pin
																		is LOW as defined in spi_routines.h
																		file */
	
	MOSI_PORT_OUT	&=	~MOSI_PORT_PIN;					/* Changing to default status after
																		operation is complete. */

#endif

	SS_PORT_OUT	&=	~SS_PORT_PIN;							/* Make SS line low */
//	SS_PORT_OUT |= SS_PORT_PIN;							/* Make SS line high */
}

/*!
 *		This function makes slave select line Low to disbale the slave
 */
void Deassert_SS( void )
{
#if ( MOSI_PORT_PIN_DEFAULT == HIGH )					/* default status of MOSI port pin
																		is HIGH as defined in spi_routines.h
																		file */

	MOSI_PORT_OUT	|=	MOSI_PORT_PIN;						/* Changing to default status after
																		operation is complete. */

#else
	
	MOSI_PORT_OUT	&=	~MOSI_PORT_PIN;					/* default status of MOSI port pin
																		is LOW as defined in spi_routines.h
																		file */

#endif

		SS_PORT_OUT |= SS_PORT_PIN;							/* Make SS line high */

}


unsigned char Shift_One_Byte( char data )
{
char read_data, i;

read_data = 0;

i = 8;
do
{
/* Setting the output pin low and then checking
to see if it should be high generates slightly
smaller code than checking and then setting high or
low.
*/
MOSI_PORT_OUT &= ~MOSI_PORT_PIN;
if(data & 0x80)
MOSI_PORT_OUT |= MOSI_PORT_PIN;

CLK_PORT_OUT |= CLK_PORT_PIN;
data <<= 1;
read_data <<= 1;
if(MISO_PORT_IN & MISO_PORT_PIN)
read_data++;
CLK_PORT_OUT &= ~CLK_PORT_PIN; // Set the clock low
} while( --i != 0);
return read_data;
}

/*
And now up to four bytes.

Type char pointers are used to pick off each individual byte. Otherwise
the compiler produces lots of extra code because of the
long data type.

This is highly dependent on the "endian" nature of how data
is stored. This compiler appears to have opted for "big
endian".
*/

long Shift_Bytes(long data, char num_bytes)
{
long read_data;
char *rp, *dp;
char i;

i = num_bytes;
read_data = 0;

rp = ((char *)&read_data) + (4 - num_bytes);
dp = ((char *)&data) + (4 - num_bytes);

do
*rp++ = Shift_One_Byte(*dp++);
while ( --i != 0 );

return read_data;
}




