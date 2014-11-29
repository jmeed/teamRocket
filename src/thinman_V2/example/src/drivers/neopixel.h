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
#define NEOPIXEL_TIMER_DEV LPC_TIMER16_0
#define NEOPIXEL_TIMER_INTERRUPT_HANDLER TIMER16_0_IRQHandler
#define NEOPIXEL_TIMER_IRQ TIMER_16_0_IRQn
#define NEOPIXEL_OUTPUT_MATCH 2
#define NEOPIXEL_RESET_MATCH 0
#define NEOPIXEL_RELOAD_MATCH 1

void neopixel_init(void);
void neopixel_set_color(uint32_t index, uint32_t color);
bool neopixel_refresh(void);

inline void neopixel_update_color_single(uint32_t index, uint32_t color) {
	neopixel_set_color(index, color);
	neopixel_refresh();
}

#endif /* NEXPIXEL_H_ */
