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
#include "volatile_flight_data.h"

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

	if (strcmp(command, "ls") == 0) {
		static FILINFO fno;
	    static DIR dir;

	    res = f_opendir(&dir, "/");
	    if (res != FR_OK) {
	    	fprintf(stderr, "root dir open failed with erro %d\n", res);
	    	return;
	    }

	    for(;;) {
	    	res = f_readdir(&dir, &fno);
	    	if (res != FR_OK || fno.fname[0] == 0) break;
	    	if (fno.fname[0] == '.') continue;
	    	if (fno.fattrib & AM_DIR) {
	    		fprintf(stderr, "D %d %s\n", fno.fsize, fno.fname);
	    	} else {
	    		fprintf(stderr, "F %d %s\n", fno.fsize, fno.fname);
	    	}
	    }
	    fprintf(stderr, "END\n");
	    if (res != FR_OK) {
	    	fprintf(stderr, "readdir failed with error %d\n", res);
	    }
	} else if (strcmp(command, "cat") == 0) {
		if (sscanf(command_line, "%5s %19s", command, buff) == 0) {
			fprintf(stderr, "Need <filename>\n");
			return;
		}
		res = f_open(&t_file, buff, FA_OPEN_EXISTING | FA_READ);
		if (res != FR_OK) {
			fprintf(stderr, "open file %s failed with error %d\n", buff, res);
			return;
		}

		for(;;) {
			UINT read = 0;
			res = f_read(&t_file, block, 40, &read);
			if (res != FR_OK) {
				break;
			}
			if (read == 0) break;
			uart0_write(block, read);
			// vcom_write(block, read);

		}

		fprintf(stderr, "END\n");
		if (res != FR_OK) {
			fprintf(stderr, "f_read failed with error %d\n", res);
		}

		f_close(&t_file);
	} else if (strcmp(command, "rm") == 0) {
		if (sscanf(command_line, "%5s %19s", command, buff) == 0) {
			fprintf(stderr, "Need <filename>\n");
			return;
		}
		res = f_unlink(buff);
		if (res != FR_OK) {
			fprintf(stderr, "Failed to unlink file %s with error %d\n", buff, res);
		}
	} else if (strcmp(command, "appd") == 0) {
		if (sscanf(command_line, "%5s %19s", command, buff) == 0) {
			fprintf(stderr, "Need <filename>\n");
			return;
		}
		res = f_open(&t_file, buff, FA_OPEN_ALWAYS | FA_WRITE);
		if (res != FR_OK) {
			fprintf(stderr, "Failed to open file %s with error %d\n", buff, res);
			return;
		}

		res = f_lseek(&t_file, f_size(&t_file));
		if (res != FR_OK) {
			fprintf(stderr, "Failed to seek to end with error %d\n", res);
			goto fail;
		}

		if (sscanf(command_line, "%5s %19s", command, buff) == 0) {
			fprintf(stderr, "Need <string>\n");
			goto fail;
		}
		res = f_write(&t_file, buff, strlen(buff), NULL);
		if (res != FR_OK) {
			fprintf(stderr, "Failed to write with error %d\n", res);
			goto fail;
		}

		fail:
		f_close(&t_file);
	}  else if (strcmp(command, "fld") == 0) {
		float cur_spd = 0;
		double elapsed_time = time_arr[0] - time_arr[4];
		double vertical_change = abs(alt_arr[0] - alt_arr[4]);
		if( elapsed_time != 0 ) {
			cur_spd = vertical_change / elapsed_time;
		}
		if( cur_spd > max_spd ) {
			max_spd = cur_spd;
		}

		fprintf(stderr, "=F %f %f %f %f %f %f %f\n", max_alt, max_acc, descent_rate, time_arr[0], max_spd, cur_spd, alt_arr[0], res);
	} else if (strcmp(command, "stat") == 0) {
		fprintf(stderr, "=S Status Message\n", res);
	} else if (strcmp(command, "par") == 0) {
		fprintf(stderr, "=P Parameter Message\n", res);
	}  else {
		fprintf(stderr, "Invalid command %s\n", command);
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
