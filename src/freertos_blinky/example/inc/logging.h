#pragma once
#include <stdlib.h>
#include "../src/drivers/uart0.h"
/**
 * This is blocking!
 */
#define LOG_CRITICAL(msg, ...) { fprintf(stderr, msg, ##__VA_ARGS__); }
#define LOG_ERROR(msg, ...) {}
#define LOG_WARN(msg, ...) {}
#define LOG_INFO(msg, ...) {}
#define LOG_DEBUG(msg, ...) {}
void exit_error(int error_code);
