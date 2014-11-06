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
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 2, LedState);
		LedState = (bool) !LedState;
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
		logging_flush_persistent();
	}
}

static void vBaro(void* pvParameters) {
	LPS_init(I2C0);
	LPS_enable();
	while (true) {
		LOG_INFO("LPS Temp = %f", LPS_read_data(LPS_TEMPERATURE));
		LOG_INFO("LPS Alti = %f", LPS_read_data(LPS_ALTITUDE));
		vTaskDelay(500);
	}
}

xTaskHandle boot_handle;
static FATFS root_fs;
static void vBootSystem(void* pvParameters) {
	int result;
	LOG_INFO("Wait for voltage stabilization");
	vTaskDelay(1000);
	LOG_INFO("Attempting to mount FAT on SDCARD");
	result = f_mount(&root_fs, "0:", 1);
	if (result != FR_OK) {
		exit_error(ERROR_CODE_SDCARD_MOUNT_FAILED);
	}

	result = logging_init_persistent();
	if (result != 0) {
		exit_error(ERROR_CODE_SDCARD_LOGGING_INIT_FAILED);
	}

	LOG_INFO("Starting real tasks");

	xTaskCreate(vFlushLogs, (signed char *) "vFlushLogs",
				256, NULL, (tskIDLE_PRIORITY), NULL);

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

	xTaskCreate(vBaro, (signed char*) "Baro", 256, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
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
