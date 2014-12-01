/*
 * neopixel.h
 *
 * Created: 11/29/2014 1:27:50 PM
 *  Author: Max Zhao
 */ 


#ifndef NEOPIXEL_H_
#define NEOPIXEL_H_


typedef struct rgb_color
{
	unsigned char red, green, blue;
} rgb_color;

extern rgb_color channel_colors[CHANNELS];

#define NEOPIXEL_DDR DDRB
#define NEOPIXEL_PORT PORTB
#define NEOPIXEL_PIN_NUM 3

void neopixel_init(void);
// void neopixel_write(const rgb_color * colors, unsigned int count);
void neopixel_update(void);


#endif /* NEOPIXEL_H_ */