/*
 * logging.c
 *
 *  Created on: Oct 19, 2014
 *      Author: Max Zhao
 */

#include <FreeRTOS.h>
#include <stdlib.h>
#include "board.h"
#include <task.h>
#include "./morse.h"
#include "logging.h"

void exit_error(int error_code) {
	taskDISABLE_INTERRUPTS();
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);
	blink_error_code(error_code);
	exit(error_code);
}
