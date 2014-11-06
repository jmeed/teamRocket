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

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 20);
}

static void debug_uart_init(void) {
	Chip_Clock_SetUSARTNBaseClockRate((115200 * 256), false);
	uart0_init();
	uart0_setup(115200, 1);
}

static void hardware_init(void) {
	// Setup UART clocks

	setup_pinmux();
	spi_init();
	spi_setup_device(SPI_DEVICE_1, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE0, true);
	SDCardInit();
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
		printf("Test float %.4f\r\n", 1.2424125);
		vTaskDelay(configTICK_RATE_HZ * 2);
	}
}

/* LED2 toggle thread */
static void vLEDTask2(void *pvParameters) {
	bool LedState = false;
	while (1) {
		Board_LED_Set(2, LedState);
		LedState = (bool) !LedState;
		printf("Hello World that works!\r\n");

		vTaskDelay(configTICK_RATE_HZ);
	}
}

xTaskHandle setup_handle;
static FATFS fs;
static DIR dir;
static FILINFO fno;
static FIL f;
static void vSetupSDCard(void* pvParameters) {
	int result;
	vTaskDelay(1000);
	LOG_INFO("Attempting to mount FAT on SDCARD");
//	 exit_error(SDCardStartup());
	result = f_mount(&fs, "0:", 1);
//	exit_error(result);
	LOG_INFO("Mount result is %d", result);
	if (result == 0) {
		result = f_open(&f, "0:test.txt", FA_WRITE | FA_OPEN_ALWAYS);
		if (result != FR_OK) {
			exit_error(result);
		}
		result = f_lseek(&f, f_size(&f)); // Seek to append
		if (result != FR_OK) {
			exit_error(result);
		}
		UINT stuff;
		result = f_write(&f, "TEST ABC\r\n", 10, &stuff);
//		exit_error(stuff);
		if (result != FR_OK) {
			exit_error(result);
		}
		result = f_sync(&f);
		if (result != FR_OK) {
			exit_error(result+40);
		}
		f_close(&f);
		exit_error(31);
//		result = f_opendir(&dir, "/");
//		LOG_INFO("Readdir result is %d", result);
//		for(;;) {
//			result = f_readdir(&dir, &fno);
//			if (result != 0 || fno.fname[0] == 0) break;
//			LOG_INFO("File name is %s", fno.fname);
//			if (!(fno.fattrib & AM_DIR)) {
//				morsePlay(fno.fname);
//			}
//		}
	}
//	result = SDCardStartup();
//	LOG_DEBUG("SDCard startup result is %d\n\r", result);
//	if (result == 0) {
//		static uint8_t read_buffer[512];
//		LOG_INFO("Trying to read sector 0");
//		result = SDCardSendCommand(17, 0, 0xff, read_buffer, 512);
//		LOG_DEBUG("SDCARD read result is %d", result);
//
//		LOG_DEBUG("Printing result");
//		{
//			int i;
//			for (i = 0; i < 512; i++) {
//				printf("%02x ", read_buffer[i]);
//			}
//			printf("\n\r");
//		}
//	}
	vTaskSuspend(setup_handle);
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

	xTaskCreate(vSetupSDCard, NULL, 256, NULL, (tskIDLE_PRIORITY + 2), &setup_handle);

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

	LOG_INFO("Tasks created; Starting scheduler");
	/* Start the scheduler */
	vTaskStartScheduler();

	exit_error(ERROR_CODE_MAIN_SCHEDULER_FALL_THRU);
	/* Should never arrive here */
	return 1;
}
