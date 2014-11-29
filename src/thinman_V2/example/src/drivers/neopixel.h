/*
 * nexpixel.h
 *
 *  Created on: Nov 18, 2014
 *      Author: Max Zhao
 */
#include <Chip.h>

#ifndef NEXPIXEL_H_
#define NEXPIXEL_H_

#define NEOPIXEL_COUNT 2

#define NEOPIXEL_COLOR_FROM_RGB(rgb)  (((rgb >> 16) << 8) | (((rgb >> 8) & 0xff) << 16) | (rgb & 0xff))

void neopixel_init(void);
void neopixel_set_color(uint32_t index, uint32_t color);
void neopixel_refresh_now(void);
void neopixel_refresh_idle(void);


#endif /* NEXPIXEL_H_ */
