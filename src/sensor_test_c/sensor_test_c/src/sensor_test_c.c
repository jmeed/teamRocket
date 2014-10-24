/*
 * @brief I2C example
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

#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "L3G.h"
#include "LPS.h"
#include "LSM303.h"

#include "H3L.h"
#include "LSM.h"
// FIXME - nneds check
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define DEFAULT_I2C          I2C0

#define I2C_EEPROM_BUS       DEFAULT_I2C
#define I2C_IOX_BUS          DEFAULT_I2C

#define SPEED_100KHZ         100000
#define SPEED_400KHZ         400000
#define I2C_DEFAULT_SPEED    SPEED_100KHZ
#define I2C_FASTPLUS_BIT     0

#if (I2C_DEFAULT_SPEED > SPEED_400KHZ)
#undef  I2C_FASTPLUS_BIT
#define I2C_FASTPLUS_BIT IOCON_FASTI2C_EN
#endif

static int mode_poll;	/* Poll/Interrupt mode flag */
static I2C_ID_T i2cDev = DEFAULT_I2C;	/* Currently active I2C device */

static volatile uint32_t tick_cnt;

static void Init_I2C_PinMux(void) {
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
#if defined(BOARD_MANLEY_11U68)
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5, IOCON_FUNC1 | I2C_FASTPLUS_BIT);

#elif defined(BOARD_NXP_LPCXPRESSO_11U68)
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4,
		(IOCON_FUNC1 | I2C_FASTPLUS_BIT) | IOCON_DIGMODE_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5,
		(IOCON_FUNC1 | I2C_FASTPLUS_BIT) | IOCON_DIGMODE_EN);

#else
#warning "No I2C Pin Muxing defined for this example"
#endif
}

/* State machine handler for I2C0 and I2C1 */
static void i2c_state_handling(I2C_ID_T id) {
	if (Chip_I2C_IsMasterActive(id)) {
		Chip_I2C_MasterStateHandler(id);
	}
	else {
		Chip_I2C_SlaveStateHandler(id);
	}
}

/* Set I2C mode to polling/interrupt */
static void i2c_set_mode(I2C_ID_T id, int polling) {
	if (!polling) {
		mode_poll &= ~(1 << id);
		Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandler);
		NVIC_EnableIRQ(I2C0_IRQn);
	}
	else {
		mode_poll |= 1 << id;
		NVIC_DisableIRQ(I2C0_IRQn);
		Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandlerPolling);
	}
}

/* Initialize the I2C bus */
static void i2c_app_init(I2C_ID_T id, int speed) {
	Init_I2C_PinMux();

	/* Initialize I2C */
	Chip_I2C_Init(id);
	Chip_I2C_SetClockRate(id, speed);

	/* Set default mode to interrupt */
	i2c_set_mode(id, 0);
}

/**
 * @brief	I2C Interrupt Handler
 * @return	None
 */
void I2C0_IRQHandler(void)
{
	i2c_state_handling(I2C0);
}

/**
 * @brief	Main program body
 * @return	int
 */
int main(void) {
	SystemCoreClockUpdate();
	Board_Init();
	i2c_app_init(I2C0, I2C_DEFAULT_SPEED);
	volatile int i;

	volatile float baro_alt = 0.0f;
	volatile float gyro_T = 0.0f, imu_T = 0.0f, baro_T = 0.0f;
	volatile float acc_x = 0.0f, acc_y = 0.0f, acc_z = 0.0f;
	volatile float mag_x = 0.0f, mag_y = 0.0f, mag_z = 0.0f;
	volatile float gyro_x = 0.0f, gyro_y = 0.0f, gyro_z = 0.0f;
	LPS_init(I2C0);
	L3G_init(I2C0);
	LSM303_init(I2C0);
	LPS_enable();
	L3G_enable();
	LSM303_enable();
	while (1) {
		baro_alt = LPS_read_data(LPS_ALTITUDE);
		gyro_T = L3G_read_data(L3G_TEMPERATURE);
//		gyro_x = L3G_read_data(L3G_SPIN_RATE_X);
//		gyro_y = L3G_read_data(L3G_SPIN_RATE_Y);
//		gyro_z = L3G_read_data(L3G_SPIN_RATE_Z);
		imu_T = LSM303_read_data(LSM303_TEMPERATURE);
		baro_T = LPS_read_data(LPS_TEMPERATURE);
//		acc_x = LSM303_read_data(LSM303_ACCEL_X);
//		acc_y = LSM303_read_data(LSM303_ACCEL_Y);
//		acc_z = LSM303_read_data(LSM303_ACCEL_Z);
//		mag_x = LSM303_read_data(LSM303_MAG_X);
//		mag_y = LSM303_read_data(LSM303_MAG_Y);
//		mag_z = LSM303_read_data(LSM303_MAG_Z);

		//DEBUGOUT("Temps: baro = %.2f, gyro = %.2f, imu = %.2f\n", baro_T, gyro_T, imu_T);
		DEBUGOUT("Baro: alt = %.2f, temp = %.2f\n", baro_alt, baro_T);
		//DEBUGOUT("Gyro: x = %.2f, y = %.2f, z = %.2f, temp = %.2f\n", gyro_x, gyro_y, gyro_z, gyro_T);
		//DEBUGOUT("MAG/XL: ax = %.2f, ay = %.2f, az = %.2f, mx = %.2f, my = %.2f, mz = %.2f, temp = %.2f\n", acc_x, acc_y, acc_z, mag_x, mag_y, mag_z, imu_T);
		for (i = 0; i < 500000; ++i);
	}


	while(1);

	Chip_I2C_DeInit(I2C0);

	return 0;
}
