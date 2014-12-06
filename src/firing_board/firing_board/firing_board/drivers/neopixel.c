/*
 * neopixel.c
 *
 * Created: 11/29/2014 1:29:12 PM
 *  Author: Max Zhao
 */ 

#include "hardware_config.h"
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "./neopixel.h"

#define __disable_irq() cli()
#define __enable_irq() sei()

rgb_color channel_colors[CHANNELS];

void neopixel_init(void) {
	NEOPIXEL_PORT &= ~_BV(NEOPIXEL_PIN_NUM);
	NEOPIXEL_DDR |= _BV(NEOPIXEL_PIN_NUM);
	memset(channel_colors, 0, sizeof(channel_colors));
}

// Derived from PoluluLedStrip driver for Arduino: https://github.com/pololu/pololu-led-strip-arduino/blob/master/PololuLedStrip/PololuLedStrip.h#L362
static void __attribute__((aligned(16))) neopixel_write(const rgb_color * colors, unsigned int count)
{
	#if defined(__AVR__)

	#elif defined(__arm__)
	Pio * port = g_APinDescription[pin].pPort;
	uint32_t pinValue = g_APinDescription[pin].ulPin;
	PIO_SetOutput(port, pinValue, LOW, 0, 0);

	#endif

	__disable_irq();   // Disable interrupts temporarily because we don't want our pulse timing to be messed up.

	while(count--)
	{
		// Send a color to the LED strip.
		// The assembly below also increments the 'colors' pointer,
		// it will be pointing to the next color at the end of this loop.
		#if defined(__AVR__)
		asm volatile(
		"ld __tmp_reg__, %a0+\n"         // Advance pointer from red to green.
		"ld __tmp_reg__, %a0\n"          // Read the green component and leave the pointer pointing to green.
		"rcall send_led_strip_byte%=\n"  // Send green component.
		"ld __tmp_reg__, -%a0\n"         // Read the red component and leave the pointer at red.
		"rcall send_led_strip_byte%=\n"  // Send green component.
		"ld __tmp_reg__, %a0+\n"         // Advance pointer from red to green.
		"ld __tmp_reg__, %a0+\n"         // Advance pointer from green to blue.
		"ld __tmp_reg__, %a0+\n"         // Read the blue component and leave the pointer on the next color's red.
		"rcall send_led_strip_byte%=\n"  // Send blue component.
		"rjmp led_strip_asm_end%=\n"     // Jump past the assembly subroutines.

		// send_led_strip_byte subroutine:  Sends a byte to the LED strip.
		"send_led_strip_byte%=:\n"
		"rcall send_led_strip_bit%=\n"  // Send most-significant bit (bit 7).
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"  // Send least-significant bit (bit 0).
		"ret\n"

		// send_led_strip_bit subroutine:  Sends single bit to the LED strip by driving the data line
		// high for some time.  The amount of time the line is high depends on whether the bit is 0 or 1,
		// but this function always takes the same time (2 us).
		"send_led_strip_bit%=:\n"
		#if F_CPU == 8000000
		"rol __tmp_reg__\n"                      // Rotate left through carry.
		#endif
		"sbi %2, %3\n"                           // Drive the line high.
		#if F_CPU != 8000000
		"rol __tmp_reg__\n"                      // Rotate left through carry.
		#endif

		#if F_CPU == 16000000
		"nop\n" "nop\n"
		#elif F_CPU == 20000000
		"nop\n" "nop\n" "nop\n" "nop\n"
		#endif

		"brcs .+2\n" "cbi %2, %3\n"              // If the bit to send is 0, drive the line low now.

		#if F_CPU == 8000000
		"nop\n" "nop\n"
		#elif F_CPU == 16000000
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		#elif F_CPU == 20000000
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		"nop\n" "nop\n"
		#endif

		"brcc .+2\n" "cbi %2, %3\n"              // If the bit to send is 1, drive the line low now.

		"ret\n"
		"led_strip_asm_end%=: "
		: "=b" (colors)
		: "0" (colors),         // %a0 points to the next color to display
		"I" (_SFR_IO_ADDR(NEOPIXEL_PORT)),   // %2 is the port register (e.g. PORTC)
		"I" (NEOPIXEL_PIN_NUM)     // %3 is the pin number (0-8)
		);

		#elif defined(__arm__)
		asm volatile(
		"ldrb r12, [%0, #1]\n"    // Load green.
		"lsls r12, r12, #24\n"    // Put green in MSB of color register.
		"ldrb r3, [%0, #0]\n"     // Load red.
		"lsls r3, r3, #16\n"
		"orrs r12, r12, r3\n"     // Put red in color register.
		"ldrb r3, [%0, #2]\n"     // Load blue.
		"lsls r3, r3, #8\n"
		"orrs r12, r12, r3\n"     // Put blue in LSB of color register.
		"rbit r12, r12\n"         // Reverse the bits so we can use right rotations.
		"adds  %0, %0, #3\n"      // Advance pointer to next color.
		
		"mov r3, #24\n"           // Initialize the loop counter register.

		"send_led_strip_bit%=:\n"
		"str %[val], %[set]\n"            // Drive the line high.
		"rrxs r12, r12\n"                 // Rotate right through carry.

		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"

		"it cc\n" "strcc %[val], %[clear]\n"  // If the bit to send is 0, set the line low now.

		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"

		"it cs\n" "strcs %[val], %[clear]\n"  // If the bit to send is 1, set the line low now.

		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"

		"sub r3, r3, #1\n"                // Decrement the loop counter.
		"cbz r3, led_strip_asm_end%=\n"   // If we have sent 24 bits, go to the end.
		"b send_led_strip_bit%=\n"

		"led_strip_asm_end%=:\n"

		: "=r" (colors)
		: "0" (colors),
		[set] "m" (port->PIO_SODR),
		[clear] "m" (port->PIO_CODR),
		[val] "r" (pinValue)
		: "r3", "r12", "cc"
		);

		#endif

		if (0)
		{
			// Experimentally on an AVR we found that one NOP is required after the SEI to actually let the
			// interrupts fire.
			__enable_irq();
			asm volatile("nop\n");
			__disable_irq();
		}
	}
	__enable_irq();         // Re-enable interrupts now that we are done.
	_delay_us(50);  // Hold the line low for 50 microseconds to send the reset signal.
}

void neopixel_update(void) {
	neopixel_write(channel_colors, CHANNELS);
}