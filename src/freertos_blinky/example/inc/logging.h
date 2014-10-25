#pragma once
#include <FreeRTOS.h>
#include "task.h"
#include <stdlib.h>
#include "../src/drivers/uart0.h"
/**
 * This is blocking!
 */
#define LOG_CRITICAL(msg, ...) { \
	char buffer[50]; \
	uart0_write_string_critical("CRITICAL "); \
	sprintf(buffer, msg, ##__VA_ARGS__); \
	uart0_write_string_critical(buffer); \
}
#define LOG_NORMAL(type, msg, ...) {\
	uart0_write_string(type); \
	printf("%05d ", xTaskGetTickCount()); \
	printf(msg, ##__VA_ARGS__); \
	uart0_write_string("\n\r"); \
}
#define LOG_ERROR(msg, ...) { LOG_NORMAL("ERROR ", msg, ##__VA_ARGS__); }
#define LOG_WARN(msg, ...) { LOG_NORMAL("WARN  ", msg, ##__VA_ARGS__); }
#define LOG_INFO(msg, ...) { LOG_NORMAL("INFO  ", msg, ##__VA_ARGS__); }
#define LOG_DEBUG(msg, ...) { LOG_NORMAL("DEBUG ", msg, ##__VA_ARGS__); }
void exit_error(int error_code);
