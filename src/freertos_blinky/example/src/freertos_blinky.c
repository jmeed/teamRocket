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
#include "sensors/LPS.h"
#include "sensors/LSM.h"

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
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, (IOCON_FUNC3 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // MISO
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, (IOCON_FUNC4 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 16, (IOCON_FUNC0 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // SSEL


	// I2C on-board
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4,
		(IOCON_FUNC1 | IOCON_FASTI2C_EN) | IOCON_DIGMODE_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5,
		(IOCON_FUNC1 | IOCON_FASTI2C_EN) | IOCON_DIGMODE_EN);

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 20);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 2);
}

static void debug_uart_init(void) {
	logging_init();
	Chip_Clock_SetUSARTNBaseClockRate((115200 * 256), false);
	uart0_init();
	uart0_setup(115200, 1);
}

#define ONBOARD_I2C I2C0
static void i2c_onboard_init(void) {
	Chip_I2C_Init(ONBOARD_I2C);
	Chip_I2C_SetClockRate(ONBOARD_I2C, 100000);
//	mode_poll &= ~(1 << id);
	Chip_I2C_SetMasterEventHandler(ONBOARD_I2C, Chip_I2C_EventHandler);
	NVIC_EnableIRQ(I2C0_IRQn);
}

void I2C0_IRQHandler(void)
{
	if (Chip_I2C_IsMasterActive(ONBOARD_I2C)) {
		Chip_I2C_MasterStateHandler(ONBOARD_I2C);
	}
	else {
		Chip_I2C_SlaveStateHandler(ONBOARD_I2C);
	}
}

static void hardware_init(void) {
	// Setup UART clocks

	setup_pinmux();
	spi_init();
	spi_setup_device(SPI_DEVICE_1, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE0, true);
	SDCardInit();
	i2c_onboard_init();
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
	while (1) {
		Board_LED_Set(1, LedState);
		LedState = (bool) !LedState;
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 2, !Chip_GPIO_GetPinState(LPC_GPIO, 0, 2));
		LOG_INFO("Test float %.4f\r\n", 1.2424125);
		vTaskDelay(configTICK_RATE_HZ * 2);
	}
}

/* LED2 toggle thread */
static void vLEDTask2(void *pvParameters) {
	bool LedState = false;
	while (1) {
		Board_LED_Set(2, LedState);
		LedState = (bool) !LedState;
		LOG_INFO("Hello World that works!\r\n");

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

static xSemaphoreHandle mutex_i2c;
static void vIMU(void* pvParameters);

static void vBaro(void* pvParameters) {
	static FIL f_baro_log;
	static char baro_str_buf[0x20];
	int result;
	int counter = 0;
	LPS_init(I2C0);
	LPS_enable();
	strcpy(baro_str_buf, "0:BARO.TAB");
	{
		int rename_number = 1;
		while(true) {
			if (f_stat(baro_str_buf, NULL) == FR_OK) {
				sprintf(baro_str_buf, "0:BARO%d.TAB", rename_number);
				rename_number ++;
				continue;
			}
			break;
		}
	}
	LOG_INFO("Baro output is %s", baro_str_buf);
	result = f_open(&f_baro_log, baro_str_buf, FA_WRITE | FA_CREATE_ALWAYS);

	// DEBUG
	xTaskCreate(vIMU, (signed char*) "IMU", 512, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
	// ENDBEBUG
	while (true) {
		float temp, alt;
		xSemaphoreTake(mutex_i2c, portMAX_DELAY);
		temp = LPS_read_data(LPS_TEMPERATURE);
		alt = LPS_read_data(LPS_ALTITUDE);
		xSemaphoreGive(mutex_i2c);
		if (result == FR_OK) {
			sprintf(baro_str_buf, "%d\t%f\t%f\n", xTaskGetTickCount(), temp, alt);
			f_puts(baro_str_buf, &f_baro_log);
			if ((counter % 50) == 0) {
				f_sync(&f_baro_log);
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
	xSemaphoreTake(mutex_i2c, portMAX_DELAY);
	LSM_init(I2C0, G_SCALE_245DPS, A_SCALE_8G, M_SCALE_4GS, G_ODR_952, A_ODR_952, M_ODR_80);
	xSemaphoreGive(mutex_i2c);
	LOG_INFO("IMU initialized");

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
	for (;;) {
		float ax, ay, az, gx, gy, gz, mx, my, mz;
		xSemaphoreTake(mutex_i2c, portMAX_DELAY);
		ax = LSM_read_accel_g(LSM_ACCEL_X);
		ay = LSM_read_accel_g(LSM_ACCEL_Y);
		az = LSM_read_accel_g(LSM_ACCEL_Z);
//		gx = LSM_read_accel_g(LSM_GYRO_X);
//		gy = LSM_read_accel_g(LSM_GYRO_Y);
//		gz = LSM_read_accel_g(LSM_GYRO_Z);
		mx = LSM_read_accel_g(LSM_MAG_X);
		my = LSM_read_accel_g(LSM_MAG_Y);
		mz = LSM_read_accel_g(LSM_MAG_Z);
		xSemaphoreGive(mutex_i2c);

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

xTaskHandle boot_handle;
static FATFS root_fs;
static void vBootSystem(void* pvParameters) {
	int result;
	LOG_INFO("Wait for voltage stabilization");
	vTaskDelay(1000);

	{
		int sdcard_retry_limit = SDCARD_START_RETRY_LIMIT;
		while (sdcard_retry_limit > 0) {
			LOG_INFO("Attempting to mount FAT on SDCARD");
			result = f_mount(&root_fs, "0:", 1);
			if (result == FR_OK) {
				break;
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
	}

	result = logging_init_persistent();
	if (result != 0) {
		exit_error(ERROR_CODE_SDCARD_LOGGING_INIT_FAILED);
	}

	LOG_INFO("Starting real tasks");

	xTaskCreate(vFlushLogs, (signed char *) "vFlushLogs",
				256, NULL, (tskIDLE_PRIORITY + 2), NULL);

	xTaskCreate(vLEDTask1, (signed char *) "vTaskLed1",
				256, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	/* LED2 toggle thread */
	xTaskCreate(vLEDTask2, (signed char *) "vTaskLed2",
				256, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	/* LED0 toggle thread */
	xTaskCreate(vLEDTask0, (signed char *) "vTaskLed0",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	mutex_i2c = xSemaphoreCreateMutex();

	xTaskCreate(vBaro, (signed char*) "Baro", 256, NULL, (tskIDLE_PRIORITY + 1UL), NULL);

	LOG_INFO("Initialization Complete. Clock speed is %d", SystemCoreClock);
	vTaskSuspend(boot_handle);
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

	/* LED1 toggle thread */

	LOG_INFO("Starting tasks");

	xTaskCreate(vBootSystem, NULL, 256, NULL, (tskIDLE_PRIORITY + 2), &boot_handle);

	LOG_INFO("Tasks created; Starting scheduler");
	/* Start the scheduler */
	vTaskStartScheduler();

	exit_error(ERROR_CODE_MAIN_SCHEDULER_FALL_THRU);
	/* Should never arrive here */
	return 1;
}
