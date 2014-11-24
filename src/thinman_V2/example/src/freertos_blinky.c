/*
 * @brief FreeRTOS Blinky example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include <stdio.h>
#include <string.h>
#include "chip.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "logging.h"
#include "error_codes.h"
#include "ff.h"
#include "drivers/uart0.h"
#include "drivers/spi.h"
#include "drivers/sdcard.h"
#include "drivers/usb.h"
#include "drivers/neopixel.h"
#include "drivers/cdc_vcom.h"
#include "drivers/S25FL.h"
#include "drivers/i2c.h"
#include "sensors/LPS.h"
#include "sensors/LSM.h"
#include "sensors/H3L.h"

#define SDCARD_START_RETRY_LIMIT 10

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */

static void setup_pinmux() {
	// SPI0 SDCard
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 21, (IOCON_FUNC2 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // MOSI
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 20, (IOCON_FUNC2 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // SCK
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 21, (IOCON_FUNC2 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // MISO
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, (IOCON_FUNC4 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 23, (IOCON_FUNC0 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // SSEL
	Chip_IOCON_PinMuxSet(LPC_IOCON, 2, 2, (IOCON_FUNC0 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);  // #RESET

	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 10, (IOCON_FUNC3 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN | IOCON_INV_EN); // NEOPixel

	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 17, (IOCON_FUNC0 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // Bluetooth wakeup


	// I2C on-board
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4,
		(IOCON_FUNC1 | IOCON_FASTI2C_EN) | IOCON_DIGMODE_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5,
		(IOCON_FUNC1 | IOCON_FASTI2C_EN) | IOCON_DIGMODE_EN);

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 20);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 2);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 2, 2);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 17);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, 17);
}

static void debug_uart_init(void) {
	logging_init();
	// Chip_Clock_SetUSARTNBaseClockRate((115200 * 256), false);
	uart0_init();
	uart0_setup(115200, UART0_LCR_WLEN8 | UART0_LCR_SBS_1BIT | UART0_LCR_PARITY_DIS);

	if (0) {
		usb_init_freertos();
		vcom_init_freertos();
		if (usb_init()) {
			if (usb_initialize_cdc_vcom()) {
				usb_connect();
			} else {
				LOG_ERROR("USB CDC initialization failed");
			}

		} else {
			LOG_ERROR("USB initialization failed");
		}
	}
}

#define ONBOARD_I2C I2C0
#define SENSOR_PRIORITY (tskIDLE_PRIORITY + 2)
static void i2c_onboard_init(void) {
	i2c_setup_master(ONBOARD_I2C);
	Chip_I2C_SetClockRate(ONBOARD_I2C, 100000);
}


static void hardware_init(void) {
	// Setup UART clocks
	setup_pinmux();
	spi_init();
	spi_setup_device(SPI_DEVICE_1, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE0, true);
	SDCardInit();
	i2c_init();
	i2c_onboard_init();
	neopixel_init();
}

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
}

/* LED0 toggle thread */
static void vLEDTask0(void *pvParameters) {
	bool LedState = false;
	while (1) {
		Board_LED_Set(0, LedState);
		LedState = (bool) !LedState;

		vTaskDelay(configTICK_RATE_HZ / 2);
	}
}

/* LED1 toggle thread */
static void vLEDTask1(void *pvParameters) {
	bool LedState = false;
	static uint32_t num;
	while (1) {
		Board_LED_Set(1, LedState);
		LedState = (bool) !LedState;
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 2, !Chip_GPIO_GetPinState(LPC_GPIO, 0, 2));
		num += 0x0a;
		num &= 0xff;
		neopixel_set_color(0, num | num << 8 | num << 16);
		neopixel_set_color(1, ~(num | num << 8 | num << 16));
		neopixel_refresh();
		vTaskDelay(configTICK_RATE_HZ * 2);
	}
}

/* LED2 toggle thread */
static void vLEDTask2(void *pvParameters) {
	bool LedState = false;
	while (1) {
		Board_LED_Set(2, LedState);
		LedState = (bool) !LedState;

		vTaskDelay(configTICK_RATE_HZ);
	}
}

static void vFlushLogs(void* pvParameters) {
	while (1) {
		vTaskDelay(1000);
		SDCardDumpLogs();
		logging_flush_persistent();
	}
}

static void vBaro(void* pvParameters) {
	static FIL f_baro_log;
	static char baro_str_buf[0x20];
	int result;
	int counter = 0;
	if (LPS_init(ONBOARD_I2C)) {
		LOG_INFO("LPS initialized");
	} else {
		LOG_ERROR("LPS failed to initialize");
	}
	LPS_enable();
	strcpy(baro_str_buf, "BARO.TAB");
	{
		int rename_number = 1;
		while(true) {
			if (f_stat(baro_str_buf, NULL) == FR_OK) {
				sprintf(baro_str_buf, "BARO%d.TAB", rename_number);
				rename_number ++;
				continue;
			}
			break;
		}
	}
	LOG_INFO("Baro output is %s", baro_str_buf);
	result = f_open(&f_baro_log, baro_str_buf, FA_WRITE 	| FA_CREATE_ALWAYS);

	if (result != FR_OK) {
		LOG_ERROR("Failed to open Baro log file %s with error code %d", baro_str_buf, result);
		vTaskSuspend(NULL);
	}

	int out;
	while (true) {
		float temp, alt;
		temp = LPS_read_data(LPS_TEMPERATURE);
		alt = LPS_read_data(LPS_ALTITUDE);
		if (result == FR_OK) {
			sprintf(baro_str_buf, "%d\t%f\t%f\n", xTaskGetTickCount(), temp, alt);
			if ((out = f_puts(baro_str_buf, &f_baro_log)) != strlen(baro_str_buf)) {
				LOG_ERROR("Baro log failed %d", out);
			}
			if ((counter % 50) == 0) {
//				LOG_INFO("Syncing Baro");
				out = f_sync(&f_baro_log);
//				LOG_INFO("Sync Done");
				if (result != FR_OK) {
					LOG_ERROR("Baro sync failed %d", out);
				}
			}
		}
		vTaskDelay(50);
		counter ++;
	}
}

static void vIMU(void* pvParameters) {
	static FIL f_imu_log;
	static char imu_str_buf[0x40];
	LOG_INFO("Initializing IMU");
	if (LSM_init(ONBOARD_I2C, G_SCALE_245DPS, A_SCALE_4G, M_SCALE_4GS, G_ODR_952, A_ODR_952, M_ODR_80)) {
		LOG_INFO("IMU initialized");
	} else {
		LOG_ERROR("IMU failed to initialize");
	}

	int result;
	int counter = 0;
	strcpy(imu_str_buf, "IMU.TAB");
	{
		int rename_number = 1;
		while(true) {
			if (f_stat(imu_str_buf, NULL) == FR_OK) {
				sprintf(imu_str_buf, "IMU%d.TAB", rename_number);
				rename_number ++;
				continue;
			}
			break;
		}
	}
	LOG_INFO("Imu output is %s", imu_str_buf);
	result = f_open(&f_imu_log, imu_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
	if (result != FR_OK) {
		LOG_ERROR("Failed to open IMU log file");
		vTaskSuspend(NULL);
	}
	for (;;) {
		float ax, ay, az, gx, gy, gz, mx, my, mz;
		ax = LSM_read_accel_g(LSM_ACCEL_X);
		ay = LSM_read_accel_g(LSM_ACCEL_Y);
		az = LSM_read_accel_g(LSM_ACCEL_Z);
		gx = LSM_read_accel_g(LSM_GYRO_X);
		gy = LSM_read_accel_g(LSM_GYRO_Y);
		gz = LSM_read_accel_g(LSM_GYRO_Z);
		mx = LSM_read_accel_g(LSM_MAG_X);
		my = LSM_read_accel_g(LSM_MAG_Y);
		mz = LSM_read_accel_g(LSM_MAG_Z);

		if (result == FR_OK) {
			sprintf(imu_str_buf, "%d\t%f\t%f\t%f\t%f\t", xTaskGetTickCount(), ax, ay, az, gx, gy);
			f_puts(imu_str_buf, &f_imu_log);
			sprintf(imu_str_buf, "%f\t%f\t%f\t%f\t%f\n", gz, mx, my, mz);
			f_puts(imu_str_buf, &f_imu_log);
			if ((counter % 50) == 0) {
				f_sync(&f_imu_log);
			}
		}

		vTaskDelay(50);
		counter += 1;
	}
}

static void vHighG(void* pvParameters) {
	static FIL f_highg_log;
	static char highg_str_buf[0x40];
	LOG_INFO("Initializing HighG");
	if (H3L_init(ONBOARD_I2C, H3L_SCALE_100G, H3L_ODR_100)) {
		LOG_INFO("HighG initialized");
	} else {
		LOG_ERROR("HighG failed to initialize");
	}

	int result;
	int counter = 0;
	strcpy(highg_str_buf, "HIGHG.TAB");
	{
		int rename_number = 1;
		while(true) {
			if (f_stat(highg_str_buf, NULL) == FR_OK) {
				sprintf(highg_str_buf, "HIGHG%d.TAB", rename_number);
				rename_number ++;
				continue;
			}
			break;
		}
	}
	LOG_INFO("HighG output is %s", highg_str_buf);
	result = f_open(&f_highg_log, highg_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
	if (result != FR_OK) {
		LOG_ERROR("Failed to open HighG log file");
		vTaskSuspend(NULL);
	}
	for (;;) {
		float ax, ay, az;
		ax = H3L_read_accel_g(H3L_X);
		ay = H3L_read_accel_g(H3L_Y);
		az = H3L_read_accel_g(H3L_Z);

		if (result == FR_OK) {
			sprintf(highg_str_buf, "%d\t%f\t%f\t%f\n", xTaskGetTickCount(), ax, ay, az);
			f_puts(highg_str_buf, &f_highg_log);
			if ((counter % 50) == 0) {
				f_sync(&f_highg_log);
			}
		}

		vTaskDelay(50);
		counter += 1;
	}
}

static void load_and_parse_command() {
	static char command[6] = {0};
	static char buff[20] = {0};
	static uint8_t block[S25FL_SECTOR_SIZE];
	static FIL t_file;
	FRESULT res;

	if (scanf("%5s", command) == 0) return;

	if (strcmp(command, "read") == 0) {
		uint32_t addr;
		if (scanf("%u", &addr) == 0) {
			fprintf(stderr, "Needs argument <address>\n");
			return;
		}

		S25FL_read(addr, block, 512);

		int i;
		for (i = 0; i < 512; i++) {
			fprintf(stderr, "%02x ", block[i]);
		}
		fprintf(stderr, "\n");
	} else if (strcmp(command, "writw") == 0) {
		uint32_t addr;
		if (scanf("%u", &addr) == 0) {
			fprintf(stderr, "Needs argument <address>\n");
			return;
		}

		fprintf(stderr, "Please enter data in hex octets, ending with anything none\n");

		int i = 0;
		memset(block, 0xff, 512);
		for (;;) {
			uint32_t byte;
			scanf("%9s", buff);
			if (sscanf(buff, "%x", &byte) == 0) {
				break;
			}
			block[i++] = byte;
			if (i >= 512) {
				break;
			}
		}

		fprintf(stderr, "printing back\n");
		for (i = 0; i < 512; i++) {
			fprintf(stderr, "%02x ", block[i]);
		}
		fprintf(stderr, "\n");

		fprintf(stderr, "writing to block\n");
		S25FL_write(addr, block, 512);
	} else if (strcmp(command, "erase") == 0) {
		uint32_t addr;
		if (scanf("%u", &addr) == 0) {
			fprintf(stderr, "Need argument <address>\n");
			return;
		}

		S25FL_erase_4k(addr);
		fprintf(stderr, "Erasure complete\n");
	} else if (strcmp(command, "erasa") == 0) {
		S25FL_erase_bulk();
		fprintf(stderr, "Everything erased\n");
	} else if (strcmp(command, "writs") == 0) {
			uint32_t addr;
			if (scanf("%u", &addr) == 0) {
				fprintf(stderr, "Needs argument <sector>\n");
				return;
			}

			fprintf(stderr, "Please enter data in hex octets, ending with anything none\n");

			int i = 0;
			memset(block, 0xff, S25FL_SECTOR_SIZE);
			for (;;) {
				uint32_t byte;
				scanf("%9s", buff);
				if (sscanf(buff, "%x", &byte) == 0) {
					break;
				}
				block[i++] = byte;
				if (i >= S25FL_SECTOR_SIZE) {
					break;
				}
			}

			fprintf(stderr, "writing to block\n");
			S25FL_write_sector(block, addr);
	} else if (strcmp(command, "reads") == 0) {
		uint32_t addr;
		if (scanf("%u", &addr) == 0) {
			fprintf(stderr, "Needs argument <address>\n");
			return;
		}

		S25FL_read_sector(block, addr);

		int i;
		for (i = 0; i < S25FL_SECTOR_SIZE; i++) {
			fprintf(stderr, "%02x ", block[i]);
		}
		fprintf(stderr, "\n");
	} else if (strcmp(command, "stats") == 0) {
		static uint32_t* stat_registers[] = {&S25FL_read_sector_count, &S25FL_write_sector_count, &S25FL_erase_sector_count};
		static const char* stat_names[] = {"read sectors", "write sectors", "erase sectors (4k)"};

		int i;
		for (i = 0; i < sizeof(stat_registers) / sizeof(stat_registers[0]); i++) {
			fprintf(stderr, "Stat %s = %d\n", stat_names[i], *stat_registers[i]);
		}
	} else if (strcmp(command, "ls") == 0) {
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
		if (scanf("%19s", buff) == 0) {
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
		if (scanf("%19s", buff) == 0) {
			fprintf(stderr, "Need <filename>\n");
			return;
		}
		res = f_unlink(buff);
		if (res != FR_OK) {
			fprintf(stderr, "Failed to unlink file %s with error %d\n", buff, res);
		}
	} else if (strcmp(command, "appd") == 0) {
		if (scanf("%19s", buff) == 0) {
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

		if (scanf("%19s", buff) == 0) {
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
	} else {
		fprintf(stderr, "Invalid command %s\n", command);
	}

}

static void vUSBUARTController(void* pvParameters) {
	for(;;) {
//		fprintf(stderr, "Read %d\n", vcom_read_cnt());
//		fprintf(stderr, "Reads %s\n", buf);
//		vTaskDelay(100);
		load_and_parse_command();
		while (true) {
			char c = getchar();
			if (c == '\r' || c == '\n') break;
		}
	}
}

static FATFS root_fs;
static void vBootSystem(void* pvParameters) {
	int result;
	LOG_INFO("Wait for voltage stabilization");
	vTaskDelay(1000);

	if (1){
		int sdcard_retry_limit = SDCARD_START_RETRY_LIMIT;
		while (sdcard_retry_limit > 0) {
			LOG_INFO("Attempting to mount FAT on SDCARD");
			result = f_mount(&root_fs, "0:", 1);
			if (result == FR_OK) {
				break;
			}
			LOG_WARN("SDCard Mount error code %d", result);
			if (result == FR_NO_FILESYSTEM) {
				LOG_WARN("No file system. Making new.");
				result = f_mkfs("0:", 1, 0);
				if (result != FR_OK) {
					LOG_ERROR("File system creation failure %d", result);
					exit_error(ERROR_CODE_SDCARD_MKFS_FAILED);
				}
			}
			Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, !Chip_GPIO_GetPinState(LPC_GPIO, 0, 20));
			vTaskDelay(200);
			sdcard_retry_limit --;
		}
		if (sdcard_retry_limit == 0) {
			LOG_ERROR("SDCard Mount failed");
			exit_error(ERROR_CODE_SDCARD_MOUNT_FAILED);
		}

		Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, false);

		result = logging_init_persistent();
		if (result != 0) {
			exit_error(ERROR_CODE_SDCARD_LOGGING_INIT_FAILED);
		}
	}

	LOG_INFO("Starting real tasks");

	xTaskCreate(vFlushLogs, (signed char *) "vFlushLogs",
				256, NULL, (tskIDLE_PRIORITY + 2), NULL);

	xTaskCreate(vLEDTask1, (signed char *) "vTaskLed1",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	/* LED2 toggle thread */
	xTaskCreate(vLEDTask2, (signed char *) "vTaskLed2",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	/* LED0 toggle thread */
	xTaskCreate(vLEDTask0, (signed char *) "vTaskLed0",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	xTaskCreate(vUSBUARTController, (signed char*) "USBUART", 512, NULL, (tskIDLE_PRIORITY + 1UL), NULL);

	xTaskCreate(vBaro, (signed char*) "Baro", 256, NULL, SENSOR_PRIORITY, NULL);

	xTaskCreate(vIMU, (signed char*) "IMU", 512, NULL, SENSOR_PRIORITY, NULL);

	xTaskCreate(vHighG, (signed char*) "HighG", 256, NULL, SENSOR_PRIORITY, NULL);

	LOG_INFO("Initialization Complete. Clock speed is %d", SystemCoreClock);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, false);
	vTaskDelete(NULL);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */

int main(void)
{
	prvSetupHardware();

	debug_uart_init();
	LOG_INFO("Initializing hardware");
	hardware_init();

	Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, true);

	/* LED1 toggle thread */

	LOG_INFO("Starting tasks");

	xTaskCreate(vBootSystem, NULL, 256, NULL, (tskIDLE_PRIORITY + 2), NULL);

	LOG_INFO("Tasks created; Starting scheduler");
	/* Start the scheduler */
	vTaskStartScheduler();

	exit_error(ERROR_CODE_MAIN_SCHEDULER_FALL_THRU);
	/* Should never arrive here */
	return 1;
}
