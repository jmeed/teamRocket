/*
 * neopixel.c
 *
 *  Created on: Nov 18, 2014
 *      Author: Max Zhao
 */

#include <Chip.h>
#include "./neopixel.h"

#define NEOPIXEL_FREQ ((uint32_t) (1/1.25e-6))
#define NEOPIXEL_0_HIGH_FREQ ((uint32_t) (1/0.4e-6))
#define NEOPIXEL_1_HIGH_FREQ ((uint32_t) (1/0.8e-6))
#define NEOPIXEL_UPDATE_DATA_FREQ NEOPIXEL_1_HIGH_FREQ
#define NEOPIXEL_RESET_FREQ ((uint32_t) (1/51e-6))

static uint32_t pixel_colors[NEOPIXEL_COUNT];
static uint32_t system_pclk_freq;

static uint32_t bit_index = 0;
static uint32_t pixel_index = 0;
static uint32_t refreshing = 0;

#define NEOPIXEL_CYCLE_COUNT (system_pclk_freq / NEOPIXEL_FREQ)
#define NEOPIXEL_UPDATE_COUNT (system_pclk_freq / NEOPIXEL_UPDATE_DATA_FREQ)
#define NEOPIXEL_RESET_COUNT (system_pclk_freq / NEOPIXEL_RESET_FREQ)
#define NEOPIXEL_0_COUNT (system_pclk_freq / NEOPIXEL_0_HIGH_FREQ)
#define NEOPIXEL_1_COUNT (system_pclk_freq / NEOPIXEL_1_HIGH_FREQ)

static bool load_bit(void) {
	if (pixel_index >= NEOPIXEL_COUNT && bit_index >= 1) {
		return false;
	}

	if (pixel_index < NEOPIXEL_COUNT) {
		if (pixel_colors[pixel_index] & (1 << (23 - bit_index))) {
			Chip_TIMER_SetMatch(NEOPIXEL_TIMER_DEV, NEOPIXEL_OUTPUT_MATCH, NEOPIXEL_1_COUNT);
		} else {
			Chip_TIMER_SetMatch(NEOPIXEL_TIMER_DEV, NEOPIXEL_OUTPUT_MATCH, NEOPIXEL_0_COUNT);
		}
	} else {
		// Reset
		Chip_TIMER_SetMatch(NEOPIXEL_TIMER_DEV, NEOPIXEL_OUTPUT_MATCH, 0);
	}

	bit_index++;
	if (bit_index >= 24) {
		pixel_index ++;
		bit_index = 0;
	}

	return true;
}

void NEOPIXEL_TIMER_INTERRUPT_HANDLER(void) {
	if (Chip_TIMER_MatchPending(NEOPIXEL_TIMER_DEV, NEOPIXEL_RELOAD_MATCH)) {
		Chip_TIMER_ClearMatch(NEOPIXEL_TIMER_DEV, NEOPIXEL_RELOAD_MATCH);
		if (!load_bit()) {
			// Reset slot
			Chip_TIMER_SetMatch(NEOPIXEL_TIMER_DEV, NEOPIXEL_RESET_MATCH, NEOPIXEL_RESET_COUNT);
			Chip_TIMER_MatchEnableInt(NEOPIXEL_TIMER_DEV, NEOPIXEL_RESET_MATCH);
		}
	} else if (Chip_TIMER_MatchPending(NEOPIXEL_TIMER_DEV, NEOPIXEL_RESET_MATCH)) {
		Chip_TIMER_ClearMatch(NEOPIXEL_TIMER_DEV, NEOPIXEL_RESET_MATCH);
		Chip_TIMER_Disable(NEOPIXEL_TIMER_DEV);
		refreshing = 0;
	}
}

// Note: output signal should INV.
void neopixel_init(void) {
	Chip_TIMER_Init(NEOPIXEL_TIMER_DEV);
	Chip_TIMER_Reset(NEOPIXEL_TIMER_DEV);
	Chip_TIMER_PrescaleSet(NEOPIXEL_TIMER_DEV, 0);
	Chip_TIMER_Disable(NEOPIXEL_TIMER_DEV);
	NEOPIXEL_TIMER_DEV->TC = 0;
	system_pclk_freq = Chip_Clock_GetSystemClockRate();

	Chip_TIMER_SetMatch(NEOPIXEL_TIMER_DEV, NEOPIXEL_RESET_MATCH, NEOPIXEL_CYCLE_COUNT);
	Chip_TIMER_ResetOnMatchEnable(NEOPIXEL_TIMER_DEV, NEOPIXEL_RESET_MATCH);
	Chip_TIMER_SetMatch(NEOPIXEL_TIMER_DEV, NEOPIXEL_OUTPUT_MATCH, 0);
	NEOPIXEL_TIMER_DEV->PWMC |= 1 << NEOPIXEL_OUTPUT_MATCH;

	Chip_TIMER_SetMatch(NEOPIXEL_TIMER_DEV, NEOPIXEL_RELOAD_MATCH, NEOPIXEL_UPDATE_COUNT);
	Chip_TIMER_MatchEnableInt(NEOPIXEL_TIMER_DEV, NEOPIXEL_RELOAD_MATCH);
	Chip_TIMER_MatchEnableInt(NEOPIXEL_TIMER_DEV, NEOPIXEL_RESET_MATCH);

	// Start interrupt to run one reset cycle on the neopixel
	refreshing = 1;
	bit_index = 1;
	pixel_index = NEOPIXEL_COUNT;
	NVIC_EnableIRQ(NEOPIXEL_TIMER_IRQ);
	Chip_TIMER_Enable(NEOPIXEL_TIMER_DEV);
}



bool neopixel_refresh(void) {
	if (refreshing > 0) {
		// XXX: schedule another reload. Needs locking to prevent race conditions
		return false;
	}
	refreshing = 1;
	bit_index = 0;
	pixel_index = 0;

	Chip_TIMER_SetMatch(NEOPIXEL_TIMER_DEV, NEOPIXEL_RESET_MATCH, NEOPIXEL_CYCLE_COUNT);
	Chip_TIMER_MatchEnableInt(NEOPIXEL_TIMER_DEV, NEOPIXEL_RELOAD_MATCH);
	Chip_TIMER_MatchDisableInt(NEOPIXEL_TIMER_DEV, NEOPIXEL_RESET_MATCH);

	NEOPIXEL_TIMER_DEV->TC = 0;
	load_bit();
	Chip_TIMER_Enable(NEOPIXEL_TIMER_DEV);
	return true;
}

void neopixel_set_color(uint32_t index, uint32_t color) {
	pixel_colors[index] = color;
}
