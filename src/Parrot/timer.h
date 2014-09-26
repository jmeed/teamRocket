/*!
 *\file	timer.h	
 *\brief	This file contains MACROS for generating timer1 reload
 *			values and declaration of global functions.
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

/* System Includes */

#include <ez8.h>

/* Local Includes */

//#include "global_defintion.h"
#define EXTERNAL 20000000						/* External crystal frequency in Hz */
#define INTERNAL 5529600						/* Internal crystal frequency in Hz */
#define CLOCK_SOURCE EXTERNAL					/* Selecting Internal/External crystal 
															as source */
#define CRYSTAL_FREQ	CLOCK_SOURCE			/*	macro used for generating re-load value
															for timer1 and generating baudrate
															reload values. */

/* Macros */

#if ( CLOCK_SOURCE  == EXTERNAL )
#define SPEED				100000				/*!< SPEED is in bits/sec. This value is 
															equal to 100K bits/sec. This is
															the max speed achieved with external
															crystal of 20.00 MHz. User should not
															define value more than this.*/

#else
#define SPEED				25000					/*!< SPEED is in bits/sec. This value is 
															equal to 25K bits/sec. This is
															the max speed achieved with internal
															crystal of 5.53 MHz. User should not
															define value more than this.*/
#endif


/* Timer1 reload values */

#define RELOAD_HIGH 		( ( CRYSTAL_FREQ / ( SPEED * 2 ) ) / 256 )
#define RELOAD_LOW		( ( CRYSTAL_FREQ / ( SPEED * 2 ) ) % 256 )
																		

/* Global Function declaration */

/*!
 *\brief	This function configures Timer1 to generate SPI clock.
 *
 */
void Init_SPI_Clock( void );

/*!
 *\brief	This function initializes system clock source.
 *
 */
void Init_System_Clock( void );

/* End of File */
