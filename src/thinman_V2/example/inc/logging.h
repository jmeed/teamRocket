
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include "task.h"
#include <stdlib.h>
#include "../src/drivers/uart0.h"
// The number of messages that have been logged so far
extern uint32_t logging_counter;
// Log a critical message. This call will block the whole processor until the action is done
#define LOG_CRITICAL(msg, ...) { \
	vTaskSuspendAll(); \
	LOG_NORMAL("CRITICAL ", msg, ##__VA_ARGS__); \
	logging_flush_persistent(); \
}
// Log a normal message.
#define LOG_NORMAL(type, msg, ...) {\
	logging_enter(); \
	printf("%s %04x %05d ", type, logging_counter++, xTaskGetTickCount()); \
	printf(msg, ##__VA_ARGS__); \
	puts("\r"); \
	logging_exit(); \
}

// Log levels
#define LOG_ERROR(msg, ...) { LOG_NORMAL("ERROR", msg, ##__VA_ARGS__); }
#define LOG_WARN(msg, ...) { LOG_NORMAL("WARN ", msg, ##__VA_ARGS__); }
#define LOG_INFO(msg, ...) { LOG_NORMAL("INFO ", msg, ##__VA_ARGS__); }
#define LOG_DEBUG(msg, ...) { LOG_NORMAL("DEBUG", msg, ##__VA_ARGS__); }

// Halt the processor and blink error_code using morse code
void exit_error(int error_code);
// Halt the processor and blick error_code along side message using morse code
void exit_error_msg(int error_code, const char* message);

// Halt the processor and blink error_code along side a message formatted using sprintf with msg and subsequent arguments, using morse code
#define EXIT_ERROR_MSG(error_code, msg, ...) { \
	char buf[100]; \
	sprintf(buf, msg, ##__VA_ARGS__); \
	exit_error_msg(error_code, buf); \
}

// Initialize logging resources (mutex)
void logging_init(void);

// Initialize persistent (SD card based) logging
int logging_init_persistent(void);
// Log a message to persistent storage, if initialized
void logging_log_persistent(const char* s, size_t size);
// Flush the persistent log storage, if initialized
void logging_flush_persistent(void);

// Enter a logging statement by acquiring the logging mutex
void logging_enter(void);
// Exit a logging statement by releasing the logging mutex
void logging_exit(void);
