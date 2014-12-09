/*
 * nexpixel.h
 *
 *  Created on: Nov 18, 2014
 *      Author: Max Zhao
 */
#include <Chip.h>

#ifndef NEXPIXEL_H_
#define NEXPIXEL_H_

// The number of Neopixels to be controlled
#define NEOPIXEL_COUNT 2
// Convert a 24-bit color from 0xRRGGBB format to 0xGGRRBB format (used by WS2812B)
#define NEOPIXEL_COLOR_FROM_RGB(rgb)  (((rgb >> 16) << 8) | (((rgb >> 8) & 0xff) << 16) | (rgb & 0xff))

// Initialize neopixel (set all to black)
void neopixel_init(void);
// Update the color of a neopixel.  Does not actually change color until next refresh
void neopixel_set_color(uint32_t index, uint32_t color);
// Force a neopixel update now
void neopixel_refresh_now(void);
// Update neopixels iff at least one pixel's color had been changed.  Intended to be used in vApplicationIdleHook
void neopixel_refresh_idle(void);


#endif /* NEXPIXEL_H_ */
