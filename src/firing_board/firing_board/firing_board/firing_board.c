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
#include "neopixel.h"
#include "morse.h"

typedef enum {
	COLOR_DISCONNECTED,
	COLOR_CONNECTED_READY,
	COLOR_FIRE_READY_1,
	COLOR_FIRE_READY_2,
	COLOR_FIRING,
	COLOR_FIRED,
	COLOR_OFF,
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

#define EXT_BAT_THRES 40 // ~ 1.0V
static void test_channel_detect(void) {
	static uint8_t channel_mux[] = {6, 4, 0, 2};
	static uint16_t external_bat_volt;
	for(;;) {
		uint8_t i;
		external_bat_volt = adc_convert(8);
		if (external_bat_volt < EXT_BAT_THRES) {
			set_channel_color(CHANNELS, COLOR_DISCONNECTED);
			_delay_ms(100);
			set_channel_color(CHANNELS, COLOR_OFF);
			_delay_ms(100);
			continue;
		}
		
		for (i = 0; i < CHANNELS; i++) {
			uint16_t result = adc_convert(channel_mux[i]);

			if (result > external_bat_volt / 2) {
				set_channel_color(i, COLOR_CONNECTED_READY);
			} else {
				set_channel_color(i, COLOR_DISCONNECTED);
			}
		}
	}
}

int main(void)
{
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
	
	test_channel_detect();
	int channel = 0;
    while(1)
    {
		int i;
		set_channel_color(channel, COLOR_DISCONNECTED);
		_delay_ms(2000);
		
		set_channel_color(channel, COLOR_CONNECTED_READY);
		_delay_ms(1000);
		
		for (i = 0; i < 10; i++) {
			_delay_ms(100);
			set_channel_color(channel, COLOR_FIRE_READY_1);
			_delay_ms(100);
			set_channel_color(channel, COLOR_FIRE_READY_2);
		}
		
		set_channel_color(channel, COLOR_FIRING);
		_delay_ms(500);
		
		set_channel_color(channel, COLOR_FIRED);
		_delay_ms(5000);
        //TODO:: Please write your application code 
		channel ++;
		channel %= CHANNELS;
    }
}