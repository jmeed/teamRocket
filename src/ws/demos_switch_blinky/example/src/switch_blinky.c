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
#include "light_ws2812_cortex.h"
#include <stdio.h>


/**
 * @brief    main routine for blinky example
 * @return    Function should not exit.
 */
int main(void)
{
	uint8_t black[8*3];
	uint8_t white[8*3];
	uint8_t grey[8*3];
	volatile int i;
	for(i = 0; i < 8*3;i++){
		black[i]=200+i*2;
		white[i]=0+i*2;
		grey[i]=100+i*2;
	}
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, ws2812_port, ws2812_set);
	while(1){

		ws2812_sendarray(black, 24);
		for (i = 0; i < 10000000; i++);
		ws2812_sendarray(white, 24);

		for (i = 0; i < 10000000; i++);
		ws2812_sendarray(grey, 24);

		for (i = 0; i < 10000000; i++);
	}
	return 0;
}
