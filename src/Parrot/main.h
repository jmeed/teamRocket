
/*
********************************************************************
* File : main.h
* Header for the functions required to do measurement
**********************************************************************
*/

/* *********
* Utilities 
************ */
void print_hex(unsigned int);


/***************************
* Miscellaneous Definations 
****************************/
#define DATA_DIR 0x01            // Assign PxCTL for Data direction
#define ALT_FUN  0x02            // Assign PxCTL for Alternate function
#define OUT_CTL  0x03            // Assign PxCTL for Output control 
#define OSC_UNLOCK_SEQ1		0xE7
#define OSC_UNLOCK_SEQ2		0x18
#define AFS1				0x07
#define AFS2				0x08
#define ALL_LEDS_OFF   0xFF 

#define CHANNELS 4	
#define SAMPLES 4	// = 2^N - 2.  So 6 for 4-sample average, or 10 for 8-sample average.  Also adjust right shift in read_sensors.c

#define DIFF_UNBUFF_2INT_OFF		0x6F	// Differential unbuffered, with 2V internal ref
#define DIFF_UNBUFF_2INT_HIGH_GAIN	0x12	// Differential unbuffered, with 2V internal ref
#define DIFF_BUFF_2INT	6F	// Differential unbuffered, with 2V internal ref 


