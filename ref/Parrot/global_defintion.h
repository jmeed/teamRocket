/*!
 *\file	global_defintion.h	
 *\brief	This file contains Macros for selecting External/Internal
 *			clock source.
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

/* Constants */

/*@{*/

/*!
 *\brief	User can change the frequency of external crystal to the desired value
 *			which is present on hardware.
 */
#define EXTERNAL 20000000						/* External crystal frequency in Hz */
#define INTERNAL 5529600						/* Internal crystal frequency in Hz */

/*@}*/

/* Macros */

#define CLOCK_SOURCE EXTERNAL					/* Selecting Internal/External crystal 
															as source */
#define CRYSTAL_FREQ	CLOCK_SOURCE			/*	macro used for generating re-load value
															for timer1 and generating baudrate
															reload values. */
#define TRUE	0xFF
#define FALSE	0x00

/* End of File */               
