/*
 * logging.c
 *
 *  Created on: Oct 19, 2014
 *      Author: Max Zhao
 */

#include <FreeRTOS.h>
#include <semphr.h>
#include <stdlib.h>
#include "board.h"
#include <task.h>
#include "./morse.h"
#include "logging.h"
#include "ff.h"

static FIL log_file;
static bool persistent_initialized = false;
static xSemaphoreHandle logging_mutex;

uint32_t logging_counter = 0;
void exit_error(int error_code) {
	taskDISABLE_INTERRUPTS();
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);
	blink_error_code(error_code);
	exit(error_code);
}

void logging_init(void) {
	logging_mutex = xSemaphoreCreateMutex();
}

int logging_init_persistent() {
	int result;
	result = f_open(&log_file, "evrythng.log", FA_WRITE | FA_OPEN_ALWAYS);
	if (result != FR_OK) return result;

	result = f_lseek(&log_file, f_size(&log_file));
	if (result != FR_OK) return result;

	persistent_initialized = true;
	return FR_OK;
}

void logging_log_persistent(const char* s, size_t size) {
	if (!persistent_initialized) return;
	UINT written;
	int result;
	while (size > 0) {
		result = f_write(&log_file, s, size, &written);
		if (result != FR_OK) break;
		size -= written;
	}
}

void logging_flush_persistent() {
	if (!persistent_initialized) return;
	logging_enter();
	f_sync(&log_file);
	logging_exit();
}

void logging_enter(void) {
	xSemaphoreTake(logging_mutex, portMAX_DELAY);
}

void logging_exit(void) {
	xSemaphoreGive(logging_mutex);
}

void logging_config_assert_failed(const char* file, uint32_t line) {
	LOG_CRITICAL("configASSERT failed at %s:%d", file, line);
}
