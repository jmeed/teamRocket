/*
 * @brief Blinky example using sysTick
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

#include "board.h"
#include <stdio.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#if defined(BOARD_MANLEY_11U68)
/* This is B2 (button 2).  Press this to blink just LD2. */
#define BLINK_SWITCH_PORT   0
#define BLINK_SWITCH_BIT    20

/* This is B3 (button 3).  Press this to cycle through all LEDs.. */
#define CYCLE_SWITCH_PORT   0
#define CYCLE_SWITCH_BIT    21

#elif defined(BOARD_NXP_LPCXPRESSO_11U68)
/* This is SW1.  Press this to blink just the RED LED. */
#define BLINK_SWITCH_PORT   0
#define BLINK_SWITCH_BIT    1

/* This is SW2.  Press this to cycle through all LEDs.. */
#define CYCLE_SWITCH_PORT   0
#define CYCLE_SWITCH_BIT    16

#else
#error "Pins not configured for this example"
#endif /* defined (BOARD_MANLEY_11U68) */

#define TICKRATE_HZ1 (10)	/* 10 ticks per second */

static bool ledCnt;
/* RTC interrupt flags */
static bool rtcWake, rtcAlarm;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief    RTC Interrupt Handler
 * @return    None
 */
void RTC_IRQHandler(void)
{
	uint32_t rtcStatus;

	/* Get RTC status register */
	rtcStatus = Chip_RTC_GetStatus(LPC_RTC);

	/* Check RTC 1KHz match interrupt */
	if (rtcStatus & RTC_CTRL_WAKE1KHZ) {
		/* RTC high resultiuon wakeup interrupt */
		rtcWake = true;
		Chip_RTC_SetWake(LPC_RTC, 150);
		if (ledCnt == false) {
			Board_LED_Toggle(0);
		}
	}

	/* Check RTC 1Khz match interrupt */
	if (rtcStatus & RTC_CTRL_ALARM1HZ) {
		/* Alarm */
		rtcAlarm = true;
	}

	/* Clear only latched RTC status */
	Chip_RTC_EnableOptions(LPC_RTC,
						   (rtcStatus & (RTC_CTRL_WAKE1KHZ | RTC_CTRL_ALARM1HZ)));
}

/**
 * @brief    Handle interrupt from SysTick timer
 * @return    Nothing
 */
void SysTick_Handler(void)
{
	static unsigned char cnt = 0;
	static unsigned char quickCnt = 0;

	quickCnt++;
	if (quickCnt > 100) {
		quickCnt = 0;

		if (ledCnt == true) {
			switch (cnt) {
			case 0:
				Board_LED_Set(0, true);
				Board_LED_Set(1, false);
				Board_LED_Set(2, false);
				break;

			case 1:
				Board_LED_Set(1, true);
				Board_LED_Set(0, false);
				Board_LED_Set(2, false);
				break;

			case 2:
				Board_LED_Set(2, true);
				Board_LED_Set(1, false);
				Board_LED_Set(1, false);
				break;

			default:
				cnt = 0;
				break;
			}

			cnt++;
			if (cnt > 2) {
				cnt = 0;
			}
		}
	}
}

/**
 * @brief    main routine for blinky example
 * @return    Function should not exit.
 */
int main(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, BLINK_SWITCH_PORT, BLINK_SWITCH_BIT);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, CYCLE_SWITCH_PORT, CYCLE_SWITCH_BIT);
	ledCnt = true;

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ1 / 30);

	/* Enable the RTC oscillator, oscillator rate can be determined by
	   calling Chip_Clock_GetRTCOscRate()    */
	Chip_Clock_EnableRTCOsc();

	/* Initialize RTC driver (enables RTC clocking) */
	Chip_RTC_Init(LPC_RTC);

	/* Enable RTC as a peripheral wakeup event */
	Chip_SYSCTL_EnablePeriphWakeup(SYSCTL_WAKEUP_RTCINT);

	/* RTC reset */
	Chip_RTC_Reset(LPC_RTC);

	/* Start RTC at a count of 0 when RTC is disabled. If the RTC is enabled, you
	   need to disable it before setting the initial RTC count. */
	Chip_RTC_Disable(LPC_RTC);
	Chip_RTC_SetCount(LPC_RTC, 0);

	/* Set a long alarm time so the interrupt won't trigger */
	Chip_RTC_SetAlarm(LPC_RTC, 1000);

	/* Enable RTC and high resolution timer - this can be done in a single
	   call with Chip_RTC_EnableOptions(LPC_RTC, (RTC_CTRL_RTC1KHZ_EN | RTC_CTRL_RTC_EN)); */
	Chip_RTC_Enable1KHZ(LPC_RTC);
	Chip_RTC_Enable(LPC_RTC);

	/* Clear latched RTC interrupt statuses */
	Chip_RTC_ClearStatus(LPC_RTC, (RTC_CTRL_OFD | RTC_CTRL_ALARM1HZ | RTC_CTRL_WAKE1KHZ));

	/* Enable RTC interrupt */
	NVIC_EnableIRQ(RTC_IRQn);

	/* Enable RTC alarm interrupt */
	Chip_RTC_EnableWakeup(LPC_RTC, (RTC_CTRL_ALARMDPD_EN | RTC_CTRL_WAKEDPD_EN));

	while (1) {
		rtcWake = rtcAlarm = false;

		/* If blink switch is pressed... */
		if ((Chip_GPIO_GetPinState(LPC_GPIO, BLINK_SWITCH_PORT, BLINK_SWITCH_BIT)) == false) {

			/* If currently cycling... */
			if (ledCnt != false) {
				/* Turn off all LEDs. */
				Board_LED_Set(0, false);
				Board_LED_Set(1, false);
				Board_LED_Set(2, false);
				/* Set RTC to wake in one second. */
				Chip_RTC_SetWake(LPC_RTC, 1000);
			}
			/* Stop cycling... */
			ledCnt = false;
		}

		/* If cycle switch is pressed... */
		if ((Chip_GPIO_GetPinState(LPC_GPIO, CYCLE_SWITCH_PORT, CYCLE_SWITCH_BIT)) == false) {
			/* Start cycling... */
			ledCnt = true;
		}

		__WFI();
	}

	return 0;
}
