/*
 * bluetooth_commands.c
 *
 *  Created on: Nov 24, 2014
 *      Author: Max Zhao
 */


#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ff.h>
#include "./bluetooth_command.h"
#include "logging.h"

static bool bluetooth_mldp_active = false;
static char line_buffer[80];

typedef enum {
	BT_EVENT_CONNECTED,
	BT_EVENT_DISCONNECTED,
} EVENT_TYPE_T;

static void bluetooth_event(EVENT_TYPE_T event) {
	if (event == BT_EVENT_CONNECTED) {
		LOG_INFO("Bluetooth Connected");
		bluetooth_mldp_active = true;
	} else if (event == BT_EVENT_DISCONNECTED) {
		LOG_INFO("Bluetooth Disconnected.");
		// fprintf(stderr, "R,1\n");
		bluetooth_mldp_active = false;
	}
}


static void bluetooth_handle_command(const char* command_line) {
	static char command[6] = {0};
	static char buff[20] = {0};
	static uint8_t block[_MAX_SS];
	static FIL t_file;
	FRESULT res;

	if (strlen(command_line) == 0) return;
	if (sscanf(command_line, "%5s", command) == 0) return;

	if (strcmp(command, "fld") == 0) {
		fprintf(stderr, "=F %f %f %f ", 1.1, 1.2, 1.3);
		fprintf(stderr, "%f %f %f\n", 1.09,2.01,3.45);
	}

}

void task_bluetooth_commands(void* pvParameters) {
	fprintf(stderr, "SB,1\n");
	fprintf(stderr, "R,1\n");
	vTaskDelay(1000);
	Chip_UART0_SetBaud(LPC_USART0, 9600);
	fprintf(stderr, "\n");
//	fprintf(stderr, "SN,ROCKET\n");
	fprintf(stderr, "SN,RocketBrd\n");
	fprintf(stderr, "U\n");
	fprintf(stderr, "SR,30000800\n");
	fprintf(stderr, "A\n");
	fprintf(stderr, "R,1\n");

	for(;;) {
		while (true) {
			memset(line_buffer, 0, sizeof(line_buffer));
			fgets(line_buffer, 79, stdin);
			LOG_INFO("received %s", line_buffer);
			if (line_buffer[0] == 0 && line_buffer[1] != 0) {
				LOG_INFO("extras %s", &line_buffer[1]);
			}
			if (strcmp(&line_buffer[0], "Connected\r\n") == 0 || line_buffer[0] == 0 && strcmp(&line_buffer[1], "Connected\r\n") == 0) {
				bluetooth_event(BT_EVENT_CONNECTED);
			} else if (strcmp(line_buffer, "Connection End\r\n") == 0) {
				bluetooth_event(BT_EVENT_DISCONNECTED);
			} else if (bluetooth_mldp_active) {
				bluetooth_handle_command(line_buffer);
			}
		}
	}
}
