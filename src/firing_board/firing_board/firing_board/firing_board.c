/*
 * firing_board.c
 *
 * Created: 11/28/2014 9:21:01 PM
 *  Author: Max Zhao
 */ 


#include "hardware_config.h"
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "error_codes.h"
#include "drivers/neopixel.h"
#include "drivers/USI_TWI_Slave.h"
#include "morse.h"

typedef enum {
	COLOR_DISCONNECTED,    // #7f7f00
	COLOR_CONNECTED_READY, // #00ff00
	COLOR_FIRE_READY_1,    // #661141
	COLOR_FIRE_READY_2,    // #5b7814
	COLOR_FIRING,          // #ff0000
	COLOR_FIRED,           // #00007f
	COLOR_OFF,             // #000000
	DEFINED_COLORS_COUNT,
} defined_colors_t;

const rgb_color color_definitions[DEFINED_COLORS_COUNT] PROGMEM = {
	{
		0x7f, 0x7f, 0x00
	},
	{
		0x00, 0xff, 0x00
	},
	{
		0x66, 0x11, 0x41
	}, 
	{
		0x5b, 0x78, 0x14
	},
	{
		0xff, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x7f
	},
	{
		0x00, 0x00, 0x00
	}
};

uint8_t color_division = 20;

void update_channel_colors() {
	neopixel_update(); // neopixel_write(channel_colors, CHANNELS);
}

void set_channel_color(uint8_t channel, defined_colors_t color) {
	if (channel == CHANNELS) {
		uint8_t i;
		for (i = 0; i < CHANNELS; i++) {
			channel_colors[i].red = pgm_read_byte(&color_definitions[color].red) / color_division;
			channel_colors[i].blue = pgm_read_byte(&color_definitions[color].blue) / color_division;
			channel_colors[i].green = pgm_read_byte(&color_definitions[color].green) / color_division;
		}
	} else {
		channel_colors[channel].red = pgm_read_byte(&color_definitions[color].red) / color_division;
		channel_colors[channel].blue = pgm_read_byte(&color_definitions[color].blue) / color_division;
		channel_colors[channel].green = pgm_read_byte(&color_definitions[color].green) / color_division;
	}
	// memcpy(&channel_colors[channel], &color_definitions[color], sizeof(rgb_color));
	update_channel_colors();
}

void adc_init(void) {
	ADMUX = 0 | _BV(REFS1); // Enable 2.56V ref
	ADCSRB = _BV(REFS2);
	ADCSRA = _BV(ADPS1) | _BV(ADPS0) | _BV(ADPS2);
	ADCSRA |= _BV(ADEN);
}

uint16_t adc_convert(uint8_t admux) {
	ADMUX &= ~0b11111;
	ADCSRB &= ~_BV(MUX5);
	ADMUX |= admux & 0b11111;
	if (admux & 0b100000) {
		ADCSRB |= _BV(MUX5);
	}
	ADCSRA |= _BV(ADSC);
	while (!(ADCSRA & _BV(ADIF)));
	uint16_t result = ADC;
	ADCSRA |= _BV(ADIF);
	return result;
}

void init_channels(void) {
	// Required even for channel detection to isolate the channels so that they don't cross pull voltage;
	PORTA = 0;
	PORTB = 0;
	DDRA |= _BV(1) | _BV(3) | _BV(4) | _BV(6);
	PORTB |= _BV(1);
	DDRB |= _BV(1);
}

typedef enum {
	SYSTEM_MODE_BOOTING,
	SYSTEM_MODE_NO_POWER,
	SYSTEM_MODE_READY,
	SYSTEM_MODE_FIRE_READY,
	SYSTEM_MODE_FIRING
} system_mode_t;

#define EXT_BAT_THRES 40 // ~ 1.0V
#define IS_ANY_CHARGE(channel_v, ext_v) (channel_v > (ext_v / 2))
// Approximately < 10KOhm
#define IS_UNFIRED_CHARGE(channel_v, ext_v) (channel_v > (ext_v * 9 / 10))
system_mode_t system_mode;
uint16_t external_bat_volt;
uint16_t bus_volt;
uint8_t system_mode;
uint8_t fire_channel = 0;
static void main_loop(void) {
	static uint8_t channel_mux[] = {6, 4, 0, 2};
	const static uint8_t channel_pins[] = {6, 4, 1, 3};
	system_mode = SYSTEM_MODE_NO_POWER;
	static uint16_t counter = 0;
	for(;;) {
		uint8_t i;
		external_bat_volt = adc_convert(8);
		bus_volt = adc_convert(7);
		if (external_bat_volt < EXT_BAT_THRES) {
			system_mode = SYSTEM_MODE_NO_POWER;
			set_channel_color(CHANNELS, COLOR_DISCONNECTED);
			_delay_ms(100);
			set_channel_color(CHANNELS, COLOR_OFF);
			_delay_ms(100);
			continue;
		}
		
		if (system_mode == SYSTEM_MODE_NO_POWER) {
			system_mode = SYSTEM_MODE_READY;
			fire_channel = 0;
		}
		
		if (system_mode != SYSTEM_MODE_FIRING) {			
			if (system_mode == SYSTEM_MODE_READY) {
				for (i = 0; i < CHANNELS; i++) {
					uint16_t result = adc_convert(channel_mux[i]);

					if (IS_UNFIRED_CHARGE(result, external_bat_volt)) {
						set_channel_color(i, COLOR_CONNECTED_READY);
					} else if (IS_ANY_CHARGE(result, external_bat_volt)) {
						set_channel_color(i, COLOR_FIRED);
					} else {
						set_channel_color(i, COLOR_DISCONNECTED);
					}
				}

				if (fire_channel > 0 && fire_channel <= CHANNELS) {
					system_mode = SYSTEM_MODE_FIRE_READY;
					counter = 0;
				} else {
					fire_channel = 0;
				}
			} else if (system_mode == SYSTEM_MODE_FIRE_READY) {
				if (fire_channel == 0 || fire_channel > CHANNELS) {
					system_mode = SYSTEM_MODE_READY;
					fire_channel = 0;
				} else {
					if (((counter / 100) % 2) == 1) {
						set_channel_color(fire_channel - 1, COLOR_FIRE_READY_1);
					} else {
						set_channel_color(fire_channel - 1, COLOR_FIRE_READY_2);
					}
					if (counter >= 2000) {
						system_mode = SYSTEM_MODE_FIRING;
					}
				}
			}
			
			_delay_ms(1);
		} else if (system_mode == SYSTEM_MODE_FIRING) {
			uint8_t temp_chan = fire_channel;
			set_channel_color(fire_channel - 1, COLOR_FIRING);
			fire_channel = 0;
			if (temp_chan > 0 && temp_chan <= CHANNELS) {
				temp_chan -= 1;
				// Master Arm
				PORTB &= ~_BV(1);
			
				// Channel ARM
				PORTA |= _BV(channel_pins[temp_chan]);
				_delay_ms(10);
				
				// Disarm
				PORTA &= ~_BV(channel_pins[temp_chan]);
			
				PORTB |= _BV(1);
			}
			system_mode = SYSTEM_MODE_READY;
		}
		counter ++;
	}
}

#define DEVICE_ID 0x93

typedef enum {
	I2C_COMMAND_READ_DEVICE_ID,
	I2C_COMMAND_READ_VOLTS,
	I2C_COMMAND_READ_SYSTEM_MODE,
	I2C_COMMAND_FIRE,
} i2c_command_t;

void handle_i2c_command_non_blocking(void) {
	// This method should be as fast as possible, must not hold for too long or I2C read to this address will block the SCK clock line extremely long
	int16_t command = usi_twi_receive_byte_non_blocking();
	if (command == I2C_COMMAND_READ_DEVICE_ID) {
		usi_twi_tranmit_byte_non_blocking(0x00);
		usi_twi_tranmit_byte_non_blocking(DEVICE_ID);
	} else if (command == I2C_COMMAND_READ_VOLTS) {
		uint16_t result_value = 0;
		switch(usi_twi_receive_byte_non_blocking()) {
			case 0:
				result_value = external_bat_volt;
				break;
			case 1:
				result_value = bus_volt;
				break;
			default:
				usi_twi_tranmit_byte_non_blocking(0x01);
				return;
		}
		usi_twi_tranmit_byte_non_blocking(0x00);
		usi_twi_tranmit_uint16_nb(result_value);
	} else if (command == I2C_COMMAND_READ_SYSTEM_MODE) {
		usi_twi_tranmit_byte_non_blocking(0x00);
		usi_twi_tranmit_byte_non_blocking(system_mode);
	} else if (command == I2C_COMMAND_FIRE) {
		int16_t channel = usi_twi_receive_byte_non_blocking();
		fire_channel = channel;
		if (channel > 0 && channel <= CHANNELS) {
			usi_twi_tranmit_byte_non_blocking(0x00);
		} else {
			usi_twi_tranmit_byte_non_blocking(0x01);
		}
	} else {
		usi_twi_tranmit_byte_non_blocking(0x01);
	}
}

int main(void)
{
	system_mode = SYSTEM_MODE_BOOTING;
	fire_channel = 0;
	init_channels();
	neopixel_init();
	// Set clock division to 1, making 8MHz / 1 = 8MHz. See http://embdev.net/topic/291954 for the cause of assembly.
	// This could be achieved using lfuse, but, this is more general applicable for our hardware platform without another complicated fuse write that is easy to mess up (lock up, for example).
	asm volatile (
	  "st Z,%1" "\n\t"
	  "st Z,%2"
	  : :
	  "z" (&CLKPR),
	  "r" ((uint8_t) (1<<CLKPCE)),
	  "r" ((uint8_t) 0)  // new CLKPR value 0: 1/1 clock
	);
	
	while (CLKPR & _BV(CLKPCE));  // wait until timeout

	update_channel_colors();
	adc_init();
	USI_TWI_Slave_Initialise(12);
	
	sei();

	main_loop();	
	blink_error_code(ERROR_CODE_MAINLOOP_FALL_THRU);
}