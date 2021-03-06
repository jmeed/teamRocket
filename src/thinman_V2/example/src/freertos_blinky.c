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
#include <math.h>
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
#include "drivers/firing_board.h"
#include "drivers/i2c_uart.h"
#include "sensors/LPS.h"
#include "sensors/LSM.h"
#include "sensors/H3L.h"
#include "tasks/bluetooth_command.h"
#include "tasks/volatile_flight_data.h"

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

	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 10, (IOCON_FUNC1 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // NEOPixel
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 10);

	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 17, (IOCON_FUNC0 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // Bluetooth wakeup


	// I2C on-board
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4,
		(IOCON_FUNC1 | IOCON_FASTI2C_EN) | IOCON_DIGMODE_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5,
		(IOCON_FUNC1 | IOCON_FASTI2C_EN) | IOCON_DIGMODE_EN);

	// Interboard
	Chip_SYSCTL_PeriphReset(RESET_I2C1);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 24, (IOCON_FUNC2 | IOCON_FASTI2C_EN | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);  // SDA
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 7, (IOCON_FUNC3 | IOCON_FASTI2C_EN | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);   // SCL

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 20);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 2);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 2, 2);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 17);
	Chip_GPIO_SetPinOutHigh(LPC_GPIO, 0, 17);
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
#define OFFBOARD_I2C I2C1
#define SENSOR_PRIORITY (tskIDLE_PRIORITY + 2)
static void i2c_onboard_init(void) {
	i2c_setup_master(ONBOARD_I2C);
	Chip_I2C_SetClockRate(ONBOARD_I2C, 100000);
}

static void i2c_offboard_init(void) {
	i2c_setup_master(OFFBOARD_I2C);
	Chip_I2C_SetClockRate(OFFBOARD_I2C, 100000);
}


static void hardware_init(void) {
	// Setup UART clocks
	setup_pinmux();
	spi_init();
	spi_setup_device(SPI_DEVICE_1, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE0, true);
	SDCardInit();
	i2c_init();
	i2c_onboard_init();
	i2c_offboard_init();
	neopixel_init();
	firing_board_init();
}

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
}

/* LED1 toggle thread */

bool volt_active;
bool gps_activated = false;
bool baro_running = false;
bool imu_running = false;
bool highg_running = false;
const uint32_t neopixel_number_translation[] = {
	0x1f0000, // R
	0x001f00, // G
	0x00001f, // B
	0x000f0f, // C
	0x0f000f, // M
	0x0f0f00, // Y
};
static void vLEDTask1(void *pvParameters) {
	uint32_t counter = 0;
	uint32_t current_counter = 0;
	while (1) {
		{
			if (current_counter == 0) {
				current_counter = xTaskGetTickCount() / 1000;
				neopixel_set_color(0, 0);
			} else {
				neopixel_set_color(0, NEOPIXEL_COLOR_FROM_RGB(neopixel_number_translation[current_counter % 6]));
				current_counter /= 6;
			}
		}

		uint32_t sec_color = 0;
		if (gps_activated) {
			sec_color += 0x1f0000;
		}
		if (volt_active) {
			sec_color += 0x001f00;
		}
		if (baro_running) {
			sec_color += 0x00000a;
		}
		if (imu_running) {
			sec_color += 0x00000a;
		}
		if (highg_running) {
			sec_color += 0x00000a;
		}
		neopixel_set_color(1, NEOPIXEL_COLOR_FROM_RGB(sec_color));

		vTaskDelay(1000);
		counter += 1;
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
		vTaskSuspend(NULL);
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
	baro_running = true;
	while (true) {
		float temp, alt;
		temp = LPS_read_data(LPS_TEMPERATURE);
		alt = LPS_read_data(LPS_ALTITUDE);
		if (result == FR_OK) {

			// Update last 5 altitude measurements
			alt_arr[4] = alt_arr[3];
			alt_arr[3] = alt_arr[2];
			alt_arr[2] = alt_arr[1];
			alt_arr[1] = alt_arr[0];
			alt_arr[0] = alt;

			// Average last 5 measurements as **simple** filter
			float avg_alt = (alt_arr[0] + alt_arr[1] + alt_arr[2] + alt_arr[3] + alt_arr[4]) / 5;

			// Store max altitude if found
			if(avg_alt > max_alt) {
				max_alt = avg_alt;
			}

			//update last 5 times for calculating speed
			time_arr[4] = time_arr[3];
			time_arr[3] = time_arr[2];
			time_arr[2] = time_arr[1];
			time_arr[1] = time_arr[0];
			time_arr[0] = xTaskGetTickCount() / 1000.0;

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

typedef struct {
	float ax;
	float ay;
	float az;
	float gx;
	float gy;
	float gz;
	float mx;
	float my;
	float mz;
} imu_measurements_t;


static imu_measurements_t imu_measurements;
static void vIMU(void* pvParameters) {
	static FIL f_imu_log;
	static char imu_str_buf[0x40];
	LOG_INFO("Initializing IMU");
	if (LSM_init(ONBOARD_I2C, G_SCALE_500DPS, A_SCALE_16G, M_SCALE_4GS, G_ODR_952, A_ODR_952, M_ODR_80)) {
		LOG_INFO("IMU initialized");
	} else {
		LOG_ERROR("IMU failed to initialize");
		vTaskSuspend(NULL);
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
	imu_running = true;
	for (;;) {
		while (!(LSM_read_reg_xlg(LSM_STATUS_REG1_XL) & 1));
		imu_measurements.ax = LSM_read_accel_g(LSM_ACCEL_X);
		imu_measurements.ay = LSM_read_accel_g(LSM_ACCEL_Y);
		imu_measurements.az = LSM_read_accel_g(LSM_ACCEL_Z);
		while (!(LSM_read_reg_xlg(LSM_STATUS_REG1_XL) & 2));
		imu_measurements.gx = LSM_read_gyro_dps(LSM_GYRO_X);
		imu_measurements.gy = LSM_read_gyro_dps(LSM_GYRO_Y);
		imu_measurements.gz = LSM_read_gyro_dps(LSM_GYRO_Z);
		while (!(LSM_read_reg_mag(LSM_STATUS_REG_M) & 8));
		imu_measurements.mx = LSM_read_mag_gs(LSM_MAG_X);
		imu_measurements.my = LSM_read_mag_gs(LSM_MAG_Y);
		imu_measurements.mz = LSM_read_mag_gs(LSM_MAG_Z);

		if (result == FR_OK) {
			//Find max acceleration in positive x direction.  "this side up" on board is +x
			if(imu_measurements.ax > max_acc) {
				max_acc = imu_measurements.ax;
			}
			sprintf(imu_str_buf, "%d\t%f\t%f\t%f\t%f\t", xTaskGetTickCount(), imu_measurements.ax, imu_measurements.ay, imu_measurements.az, imu_measurements.gx);
			f_puts(imu_str_buf, &f_imu_log);
			sprintf(imu_str_buf, "%f\t%f\t%f\t%f\t%f\n", imu_measurements.gy, imu_measurements.gz, imu_measurements.mx, imu_measurements.my, imu_measurements.mz);
			f_puts(imu_str_buf, &f_imu_log);
			if ((counter % 100) == 0) {
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
		vTaskSuspend(NULL);
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
	highg_running = true;
	for (;;) {
		float ax, ay, az;
		while (!(H3L_read_reg(H3L_STATUS_REG) & 1));
		ax = H3L_read_accel_g(H3L_X);
		while (!(H3L_read_reg(H3L_STATUS_REG) & 2));
		ay = H3L_read_accel_g(H3L_Y);
		while (!(H3L_read_reg(H3L_STATUS_REG) & 4));
		az = H3L_read_accel_g(H3L_Z);

		if (fabs(ay) > 5.0) {
			if (!firing_board_fire_channel(2)) {
				LOG_DEBUG("Failed to fire 2");
			} else {
				LOG_DEBUG("Firing 2");
			}
		}

		if (result == FR_OK) {
			//Find max acceleration in positive x direction.  "this side up" on board is +x
			if( ax > max_acc ) {
				max_acc = ax;
			}
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

static void vGPS(void* pv) {
	static FIL f_volts;
	static char highg_str_buf[0x40];
	LOG_INFO("Initializing GPS board");

	int result;
	strcpy(highg_str_buf, "GPS.TAB");
	{
		int rename_number = 1;
		while(true) {
			if (f_stat(highg_str_buf, NULL) == FR_OK) {
				sprintf(highg_str_buf, "GPS%d.TAB", rename_number);
				rename_number ++;
				continue;
			}
			break;
		}
	}
	LOG_INFO("GPS output is %s", highg_str_buf);
	result = f_open(&f_volts, highg_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
	if (result != FR_OK) {
		LOG_ERROR("Failed to open gps log file");
		vTaskSuspend(NULL);
	}

	TickType_t xLastWakeTime = xTaskGetTickCount();
	bool line_broken = true;
	uint32_t counter = 0;
	uint32_t line_counter = 0;

    LOG_INFO("Initializing Telem wing");
	for(;;) {
		if (i2c_uart_init()) {

			i2c_uart_set_gpio_direction(0xff); // Set all GPIO to output to reduce noise susceptibility
			i2c_uart_write_gpio(1 << 3);
			LOG_INFO("Telemetry wing initialized");
			size_t line_position = 0;
            static char line_buffer[200];
            bool is_gpgga = false;
            bool is_firing = true;
            line_buffer[sizeof(line_buffer) - 1] = 0;
			for(;;) {
				int c;
				if (i2c_uart_transmit_error) {
					LOG_ERROR("Telemetry wing dropped out");
					gps_activated = false;
					break;
				}
				while(1) {
					c = i2c_uart_readc(I2C_UART_CHANB);
					if (c < 0) {
						break;
					}


					if (line_position < sizeof(line_buffer) - 1) {
						line_buffer[line_position] = c;
					}
					f_putc(c, &f_volts);
					line_position ++;

					if (line_position == 6) {
						line_buffer[6] = 0;
						if (strcmp(line_buffer, "$GPGGA") == 0) {
							is_gpgga = true;
						}
					}

					if (c == '\n') {
						if ((line_counter % 20) == 0) {
							f_sync(&f_volts);
						}
						line_counter ++;
						if (is_gpgga) {
							line_buffer[line_position] = 0;
							i2c_uart_send_string(I2C_UART_CHANA, line_buffer);
						}

						line_broken = true;
						line_position = 0;
						is_gpgga = false;
					} else {
						line_broken = false;
					}
					gps_activated = true;
				}
				while (1) {
					c = i2c_uart_readc(I2C_UART_CHANA);
					if (c < 0) {
						break;
					}

					if (is_firing) {
						is_firing = false;
						if (c >= '1' && c <= '4') {
							if (firing_board_fire_channel(c - '0')) {
								i2c_uart_send_string(I2C_UART_CHANA, "Firing\n");
							} else {
								i2c_uart_send_string(I2C_UART_CHANA, "Failed to fire\n");
							}
						} else {
							i2c_uart_send_string(I2C_UART_CHANA, "Bad channel\n");
						}
					}
					if (c == 'F') {
						is_firing = true;
					}
				}
				if ((counter % 10) == 0) {
					static char imu_out_buf[40];
					sprintf(imu_out_buf, "S,IMUACC,%.2f,%.2f,%.2f\n", imu_measurements.ax, imu_measurements.ay, imu_measurements.az);
					i2c_uart_send_string(I2C_UART_CHANA, imu_out_buf);
				}
				vTaskDelayUntil(&xLastWakeTime, 10);
				counter ++;
			}
		}
		vTaskDelay(500); // Wait for device to connect
	}
}

static void vVolts(void* pv) {
	static FIL f_volts;
	static char highg_str_buf[0x40];

	int result;
	int counter = 0;
	strcpy(highg_str_buf, "VOLTS.TAB");
	{
		int rename_number = 1;
		while(true) {
			if (f_stat(highg_str_buf, NULL) == FR_OK) {
				sprintf(highg_str_buf, "VOLTS%d.TAB", rename_number);
				rename_number ++;
				continue;
			}
			break;
		}
	}
	LOG_INFO("Volts output is %s", highg_str_buf);
	result = f_open(&f_volts, highg_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
	if (result != FR_OK) {
		LOG_ERROR("Failed to open volts log file");
		vTaskSuspend(NULL);
	}


	LOG_INFO("Init Firing Board ");
	for (;;) {
		if (firing_board_setup(OFFBOARD_I2C)) {
			LOG_INFO("Firing board initialized");

			for (;;) {
				float vext, vbus;
				vext = firing_board_read_volt(VOLTAGE_EXTERNAL_BAT);
				vbus = firing_board_read_volt(VOLTAGE_BUS);

				if (firing_board_transmit_error) {
					LOG_ERROR("Firing board dropped out");
					volt_active = false;
					break;
				}

				if (result == FR_OK) {
					sprintf(highg_str_buf, "%d\t%f\t%f\n", xTaskGetTickCount(), vext, vbus);
					f_puts(highg_str_buf, &f_volts);
					if ((counter % 5) == 0) {
						f_sync(&f_volts);
					}
				}

				volt_active = true;

				vTaskDelay(500);
				counter += 1;
			}
		}
		vTaskDelay(500); // Wait for firing board connect
	}
}

static TaskHandle_t monitor_tasks[10];
static uint32_t monitor_task_write_ptr;

void vTaskDepthRecorder(void* pv) {
	int i;
	for (;;) {
		for (i = 0; i < sizeof(monitor_tasks) / sizeof(*monitor_tasks); i++) {
			if (monitor_tasks[i]) {
				LOG_DEBUG("Task %s: watermark %d", pcTaskGetTaskName(monitor_tasks[i]), uxTaskGetStackHighWaterMark(monitor_tasks[i]));
			}
		}
		vTaskDelay(2000);
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
//	xTaskCreate(vTaskDepthRecorder, "DepthRec", 156, NULL, (tskIDLE_PRIORITY + 2), &monitor_tasks[monitor_task_write_ptr++]);

	xTaskCreate(vFlushLogs, "vFlushLogs",
				150, NULL, (tskIDLE_PRIORITY + 2), &monitor_tasks[monitor_task_write_ptr++]);

	xTaskCreate(vLEDTask1, "vTaskLed1",
				128, NULL, (tskIDLE_PRIORITY + 1UL),
				&monitor_tasks[monitor_task_write_ptr++]);

	xTaskCreate(task_bluetooth_commands, "USBUART", 256, NULL, (tskIDLE_PRIORITY + 1UL), &monitor_tasks[monitor_task_write_ptr++]);

	xTaskCreate(vBaro, "Baro", 256, NULL, SENSOR_PRIORITY, &monitor_tasks[monitor_task_write_ptr++]);

	xTaskCreate(vIMU, "IMU", 256, NULL, SENSOR_PRIORITY, &monitor_tasks[monitor_task_write_ptr++]);

	xTaskCreate(vHighG, "HighG", 256, NULL, SENSOR_PRIORITY, &monitor_tasks[monitor_task_write_ptr++]);
	xTaskCreate(vVolts, "Volts", 256, NULL, (tskIDLE_PRIORITY + 1UL), &monitor_tasks[monitor_task_write_ptr++]);
	xTaskCreate(vGPS, "GPS", 256, NULL, (tskIDLE_PRIORITY + 1UL), &monitor_tasks[monitor_task_write_ptr++]);

	LOG_INFO("Initialization Complete. Clock speed is %d", SystemCoreClock);
	LOG_INFO("Free memory %d", xPortGetFreeHeapSize());

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

	/* Initialize Globals */
	max_spd = 0;
	max_acc = 0;
	max_alt = 0;
	descent_rate = 0;
	
	alt_arr[0] = 0;
	alt_arr[1] = 0;
	alt_arr[2] = 0;
	alt_arr[3] = 0;
	alt_arr[4] = 0;

	time_arr[0] = 0;
	time_arr[1] = 0;
	time_arr[2] = 0;
	time_arr[3] = 0;
	time_arr[4] = 0;

	prvSetupHardware();

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 20);
	debug_uart_init();
	LOG_INFO("Initializing hardware");
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, true);
	hardware_init();
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, true);


	/* LED1 toggle thread */

	LOG_INFO("Starting tasks");

	xTaskCreate(vBootSystem, "Boot", 256, NULL, (tskIDLE_PRIORITY + 2), &monitor_tasks[monitor_task_write_ptr++]);

	LOG_INFO("Tasks created; Starting scheduler");
	/* Start the scheduler */
	vTaskStartScheduler();

	exit_error(ERROR_CODE_MAIN_SCHEDULER_FALL_THRU);
	/* Should never arrive here */
	return 1;
}
