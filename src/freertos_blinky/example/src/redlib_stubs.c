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

int __sys_write(int iFileHandle, char *pcBuffer, int iLength) {
	if (iFileHandle == 1) {
		// stderr totally doesn't work
		uart0_write((const uint8_t*) pcBuffer, iLength);
		logging_log_persistent(pcBuffer, iLength);
	}
	return 0;
}

void __sys_appexit() {
	blink_error_code(ERROR_CODE_UNKNOWN_ERROR);
}
