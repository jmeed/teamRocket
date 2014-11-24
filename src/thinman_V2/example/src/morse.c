/*
 * logging.c
 *
 *  Created on: Oct 19, 2014
 *      Author: Max Zhao
 */

#include <stdint.h>
#include "chip.h"
#include "board.h"
#include "morse.h"

// Source: http://www.kg6haf.com/embedded_morse.html
// Playing Morse Code on an LED
// Numbers (0-9) morse code handling implemented by Yichen Zhao 2014-4-17

#define LED_ON() Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, true);
#define LED_OFF() Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, false);

void DELAY_MS(uint16_t ms) {
	volatile int i;
	volatile uint16_t ms2 = ms;
    while (ms2 > 0) {
        for (i = 0; i < 10000; i++);
        ms2 --;
    }
}

#define DWORD unsigned long    /* Set this up as appropriate for your platform */
#define BYTE unsigned char     /* Set this up as appropriate for your platform */

// Character Data Table
//   high nibble is element count (number of dits and dahs in the character)
//   low nibble is reversed bit pattern for char with 1 == dah, 0 == dit
static const BYTE morse_data[26] =
{ 0x22,0x41,0x45,0x31,0x10,0x44,0x33,0x40,0x20,0x4E,
  0x35,0x42,0x23,0x21,0x37,0x46,0x4B,0x32,0x30,0x11,
  0x34,0x48,0x36,0x49,0x4D,0x43
};

static const BYTE morse_num_data[10] =
{
	0xbf, 0xbe, 0xbc, 0xb8, 0xb0, 0xa0, 0xa1, 0xa3, 0xa7, 0xaf
};

// Timings
#define WPM       13
#define T_MS_UNIT ((DWORD) ( 1600UL / WPM )) // A bit slower
#define T_DIT     ( T_MS_UNIT * 1 )
#define T_DAH     ( T_MS_UNIT * 3 )

#define T_INTRA   ( T_MS_UNIT * 1 + ( T_MS_UNIT >> 1 ))     /* stretch by 1/2 unit */
#define T_LETTER  ( T_MS_UNIT * 3 + ( T_MS_UNIT * .075 ))   /* stretch by 7.5%, think I meant to do 3/4! */
#define T_SPACE   ( T_MS_UNIT * 7 + ( T_MS_UNIT >> 1 ))     /* stretch by 1/2 unit */

static void morse_char_play( char c ) {
    BYTE byProgram, nLen, mask, ii;
	if (c >= 'a' && c <= 'z') c += 'A' - 'a';
	if (c >= '0' && c <= '9') {
		byProgram = morse_num_data[c - '0'];
	} else if (c >= 'A' && c <= 'Z') {
		byProgram = morse_data[c - 'A'];
	} else {
        DELAY_MS( T_SPACE );
        return;
	}

    nLen = ( byProgram >> 4 );   // char length in high nibble
    if (nLen > 4) {
    	nLen >>= 1;
    }

    for( mask = 1, ii = 0 ; ii < nLen ; ii++, mask <<= 1 )
    {
        if ( ii != 0 )
            DELAY_MS( T_INTRA );
        LED_ON();
        DELAY_MS( ( mask & byProgram ) ? T_DAH : T_DIT );
        LED_OFF();
    }
}

static void morse_string_play( const char * psz ) {
    for( /**/ ; *psz ; psz++ )
    {
        if ( *psz == ' ' )
            DELAY_MS( T_SPACE );
        else
        {
            morse_char_play( *psz );
            DELAY_MS( T_LETTER );
        }
    }
}

void morseStop( const char * psz ) {
    // Play string infinitely on LED in morse and stop app
    // Maybe turn of interrupts or call debugger here in debug builds

    while( 1 )
    {
        morse_string_play( psz );
        DELAY_MS( T_SPACE * 2 );
    }
}

void morsePlay( const char * psz ) {
    morse_string_play( psz );
}

void morseInt(unsigned int num) {
	char result[10];
	int length = 0;
	bool first_loop = true;
    int i, j;
	while (num > 0 || first_loop) {
		result[length++] = (num % 10) + '0';
		num /= 10;
		first_loop = false;
	}

	for (i = 0, j = length - 1; i < j; i++, j--) {
		char temp = result[i];
		result[i] = result[j];
		result[j] = temp;
	}
	result[length] = 0;
	morse_string_play(result);
}

void morsePause() {
	DELAY_MS( T_SPACE * 2 );
}

void blink_error_code(int code) {
	// Morse code
	for(;;) {
		morseInt(code);
		morsePause();
	}
}

