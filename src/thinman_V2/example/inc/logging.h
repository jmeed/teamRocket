#pragma once
#include <stdio.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include "task.h"
#include <stdlib.h>
#include "../src/drivers/uart0.h"
/**
 * This is blocking!
 */
extern uint32_t logging_counter;
#define LOG_CRITICAL(msg, ...) { \
	vTaskSuspendAll(); \
	LOG_NORMAL("CRITICAL ", msg, ##__VA_ARGS__); \
	logging_flush_persistent(); \
}
#define LOG_NORMAL(type, msg, ...) {\
	logging_enter(); \
	printf("%s %04x %05d ", type, logging_counter++, xTaskGetTickCount()); \
	printf(msg, ##__VA_ARGS__); \
	puts("\r"); \
	logging_exit(); \
}
#define LOG_ERROR(msg, ...) { LOG_NORMAL("ERROR", msg, ##__VA_ARGS__); }
#define LOG_WARN(msg, ...) { LOG_NORMAL("WARN ", msg, ##__VA_ARGS__); }
#define LOG_INFO(msg, ...) { LOG_NORMAL("INFO ", msg, ##__VA_ARGS__); }
#define LOG_DEBUG(msg, ...) { LOG_NORMAL("DEBUG", msg, ##__VA_ARGS__); }
void exit_error(int error_code);
void exit_error_msg(int error_code, const char* message);

#define EXIT_ERROR_MSG(error_code, msg, ...) { \
	char buf[100]; \
	sprintf(buf, msg, ##__VA_ARGS__); \
	exit_error_msg(error_code, buf); \
}

void logging_init(void);

int logging_init_persistent(void);
void logging_log_persistent(const char* s, size_t size);
void logging_flush_persistent(void);

void logging_enter(void);
void logging_exit(void);
