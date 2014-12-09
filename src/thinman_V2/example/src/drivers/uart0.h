/**
 * UART Generic Driver using LPC libraries
 */
#pragma once
#include <stdint.h>
#include "chip.h"

#define UART0_WRITE_RB_SIZE 128
#define UART0_READ_RB_SIZE 64

// Initialize UART0 resources (mutexes, semaphores)
void uart0_init(void);
// Setup UART0 to a certain baudrate and bits (stop, parity, etc)
void uart0_setup(uint32_t baudrate, uint32_t config);

// Write the given block of data to UART0. Blocks until finish
void uart0_write(const uint8_t* data, size_t size);
// Write a string to UART0. Blocks until finish
void uart0_write_string(const char* str);
// Write a given block of data to UART0, force using busy waiting (in case FreeRTOS crash)
void uart0_write_critical(const uint8_t* data, size_t size);
// Write a string to UART0, force using busy waiting (in case FreeRTOS crash)
void uart0_write_string_critical(const char* str);
// Read a block from UART0 buffer.  Returns the amount of bytes actually read.  Guarantees at least one byte is read every invocation.  Blocks if non available.
size_t uart0_read(char* buf, size_t size);
// Read a single character from UART0.  Blocks if none available.
int  uart0_readchar();
