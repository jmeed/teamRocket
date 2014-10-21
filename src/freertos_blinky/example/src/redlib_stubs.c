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

int __sys_write(int iFileHandle, char *pcBuffer, int iLength) {
	if (iFileHandle == 1) {
		uart0_write((const uint8_t*) pcBuffer, iLength);
	} else if (iFileHandle == 2) {
		uart0_write_critical((const uint8_t*) pcBuffer, iLength);
	}
	return 0;
}

void __sys_appexit() {
	blink_error_code(ERROR_CODE_UNKNOWN_ERROR);
}
