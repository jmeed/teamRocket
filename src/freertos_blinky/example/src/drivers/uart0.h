/**
 * UART Generic Driver using LPC libraries
 */
#pragma once
#include <stdint.h>
#include "chip.h"

void uart0_init(void);
void uart0_setup(uint32_t baudrate, uint32_t config);
void uart0_write(const uint8_t* data, size_t size);
void uart0_write_string(const char* str);
void uart0_write_critical(const uint8_t* data, size_t size);
void uart0_write_string_critical(const char* str);
