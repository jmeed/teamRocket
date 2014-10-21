/**
 * UART Generic Driver using LPC libraries
 */
#pragma once
#include <stdint.h>
#include "chip.h"

void uart0_init(void);
void uart0_setup(uint32_t baudrate, uint32_t config);

/**
 * XXX: currently there is no guarantee that this function will write everything, and won't busy-wait block (so, one of the two).
 * Requires better documentation reading and preferably checking source code.
 * Based on the interrupt handling code in uart_0_11u6x, writing more than the buffer can hold (64 bytes?) may lead to incomplete
 * writes that cannot be reflected through the return value.
 */
void uart0_write(const uint8_t* data, size_t size);
void uart0_write_string(const char* str);
void uart0_write_critical(const uint8_t* data, size_t size);
void uart0_write_string_critical(const char* str);
