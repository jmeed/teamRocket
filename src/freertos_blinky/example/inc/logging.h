#pragma once
#include <stdlib.h>
#include "../src/drivers/uart0.h"
/**
 * This is blocking!
 */
#define LOG_CRITICAL(msg, ...) { \
	char buffer[50]; \
	sprintf(buffer, msg, ##__VA_ARGS__); \
	uart0_write_string_critical(buffer); \
}
#define LOG_ERROR(msg, ...) {}
#define LOG_WARN(msg, ...) {}
#define LOG_INFO(msg, ...) {}
#define LOG_DEBUG(msg, ...) {}
void exit_error(int error_code);
