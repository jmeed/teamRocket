/*
 * redlib_stubs.c
 *
 * Getting printf and stuff to work...
 *  Created on: Oct 19, 2014
 *      Author: Max Zhao
 */

#include "./drivers/uart0.h"
#include "morse.h"
#include "error_codes.h"
#include "logging.h"
#include "drivers/cdc_vcom.h"

int __sys_write(int iFileHandle, char *pcBuffer, int iLength) {
	if (iFileHandle == 1) {
		// stderr totally doesn't work
		// uart0_write((const uint8_t*) pcBuffer, iLength);
		logging_log_persistent(pcBuffer, iLength);
		// vcom_write((uint8_t*) pcBuffer, iLength);
	} else if (iFileHandle == 2) {
		uart0_write((const uint8_t*) pcBuffer, iLength);
		// vcom_write((uint8_t*) pcBuffer, iLength);
	}
	return iLength;
}

int __sys_readc(void) {
//	static char read_buffer[20];
//	static int buf_size = 0;
//	static int buf_index = 0;
//	while (buf_index >= buf_size) {
//		buf_size = vcom_bread((uint8_t*) read_buffer, 20);
//		buf_index = 0;
//	}
//
//	return read_buffer[buf_index++];
	int c = uart0_readchar();
//	printf("%c", c);
	return c;
}

void __sys_appexit() {
	blink_error_code(ERROR_CODE_UNKNOWN_ERROR);
}

void __assertion_failed(char *_Expr) {
	LOG_CRITICAL("Assertion failed %s", _Expr);
	exit_error(ERROR_CODE_ASSERTION_FAILED);
}
