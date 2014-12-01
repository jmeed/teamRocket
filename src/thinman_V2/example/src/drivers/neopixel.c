/*
 * neopixel.c
 *
 *  Created on: Nov 18, 2014
 *      Author: Max Zhao
 *
 * @remarks I'm giving up on using interrupts and/or DMA for neopixels on the LPC. For alternative chipsets it's possible to do
 * this unblocking using DMA and timers (PWM), but that requires a good (working) system and a decent PWM timer that can at least
 * control the output polarity, unlike the abomination that is TIM16B0.   See
 * https://github.com/ErichStyger/mcuoneclipse/tree/master/Examples/Eclipse/FRDM-KL25Z/FRDM-KL25Z_NeoPixel, more specifically
 * https://github.com/ErichStyger/mcuoneclipse/blob/master/Examples/Eclipse/FRDM-KL25Z/FRDM-KL25Z_NeoPixel/Sources/NeoPixel.c
 * for what non-blocking code might look like.  We might have better luck with SCT0/1
 *
 * Current solution is to use stop-the-world blocking code that disables all interrupts (with sufficiently low priority that
 * portDISABLE_INTERRUPTS) works on them, and activate the updates asynchronously when the system is in idle (idle hook)
 */

#include <string.h>
#include <Chip.h>
#include <FreeRTOS.h>
#include <task.h>
#include "./neopixel.h"
#include "./light_ws2812_cortex.h"

#define NEOPIXEL_FREQ ((uint32_t) (1/1.25e-6))
#define NEOPIXEL_0_HIGH_FREQ ((uint32_t) (1/0.4e-6))
#define NEOPIXEL_1_HIGH_FREQ ((uint32_t) (1/0.8e-6))
#define NEOPIXEL_UPDATE_DATA_FREQ NEOPIXEL_1_HIGH_FREQ
#define NEOPIXEL_RESET_FREQ ((uint32_t) (1/51e-6))

static uint32_t pixel_colors[NEOPIXEL_COUNT];
static uint8_t output_array[NEOPIXEL_COUNT * 3];
static uint8_t last_output_array[NEOPIXEL_COUNT * 3];

void neopixel_init(void) {
	memset(pixel_colors, 0, sizeof(pixel_colors));
	neopixel_refresh_now();
}

static bool check_update_output_array(void) {
	int i;
	bool changed = false;
	for (i = 0; i < NEOPIXEL_COUNT; i++) {
		output_array[i * 3] = pixel_colors[i] >> 16;
		output_array[i * 3 + 1] = pixel_colors[i] >> 8;
		output_array[i * 3 + 2] = pixel_colors[i] >> 0;
	}
	changed = (memcmp(output_array, last_output_array, sizeof(output_array)) != 0);
	memcpy(last_output_array, output_array, sizeof(output_array));
	return changed;
}

static void neopixel_send_data(void) {
	portDISABLE_INTERRUPTS();
	ws2812_sendarray(output_array, sizeof(output_array));
	portENABLE_INTERRUPTS();
}

void neopixel_refresh_now(void) {
	check_update_output_array();
	neopixel_send_data();
}

void neopixel_refresh_idle(void) {
	if (check_update_output_array()) {
		neopixel_send_data();
	}
}

void neopixel_set_color(uint32_t index, uint32_t color) {
	pixel_colors[index] = color;
}
