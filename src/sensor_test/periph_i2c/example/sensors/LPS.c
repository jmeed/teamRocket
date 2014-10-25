#include "LPS.h"

int LPS_init(I2C_ID_T id_in) {
	LPS_i2c_id = id_in;
	LPS_slave_address = LPS_SA0_HIGH_ADDRESS;
	return 1;
	//return detect_device();
}

float LPS_read_data(uint8_t dimension) {
	float p_mbar = 0.0f;
	switch (dimension) {
		case LPS_ALTITUDE:
			p_mbar = LPS_read_pressure_millibars();
			return LPS_pressure_to_altitude_m(p_mbar, 1013.25f);
		case LPS_TEMPERATURE:
			return LPS_read_temperature_C();
		default:
			return 0.0f;
	}
}

// Device specific members

void LPS_enable() {
	// 0xE0 = 0b11100000
	// PD = 1 (active mode); ODR = 110 (12.5 Hz pressure & temperature output data rate)
	LPS_write_reg(LPS_CTRL_REG1, 0xE0);
}

int32_t LPS_read_pressure_raw() {
	uint8_t p_xl, p_l, p_h;
	p_xl = LPS_read_reg(LPS_OUT_PRESS_XL);
	p_l = LPS_read_reg(LPS_OUT_PRESS_L);
	p_h = LPS_read_reg(LPS_OUT_PRESS_H);
	return (int32_t)(p_h << 16 | (uint16_t)(p_l << 8 | p_xl));
}

float LPS_read_pressure_millibars() {
	return (float)LPS_read_pressure_raw() / 4096.0f;
}

int16_t LPS_read_temperature_raw() {
	uint8_t t_l, t_h;
	t_l = LPS_read_reg(LPS_OUT_TEMP_L);
	t_h = LPS_read_reg(LPS_OUT_TEMP_H);
	return (int16_t)(t_h << 8 | t_l);
}

float LPS_read_temperature_C() {
	return 42.5f + (float)LPS_read_temperature_raw() / 480.0f;
}

float LPS_pressure_to_altitude_m(float pressure_mbar, float altimeter_setting_mbar) {
	return (1 - pow(pressure_mbar / altimeter_setting_mbar, 0.190263f)) * 44330.8f;
}

// bool LPS_detect_device() {
// 	slave_address = LPS_SA0_LOW_ADDRESS;
// 	if (read_reg(LPS_WHO_AM_I) == LPS331AP_WHO_ID)
// 		return true;
// 	slave_address = LPS_SA0_HIGH_ADDRESS;
// 	if (read_reg(LPS_WHO_AM_I) == LPS331AP_WHO_ID) {
// 		slave_address = LPS_SA0_LOW_ADDRESS;
// 		return true;
// 	}

// 	return false;
// }

uint8_t LPS_read_reg(uint8_t reg_addr) {
	// Write the register we want to read
	// - Make a transmit buffer
	uint8_t tx_size = 1;
	uint8_t tx_buf[tx_size];
	// - Set the register address
	tx_buf[0] = reg_addr;
	// - Write the register value
	Chip_I2C_MasterSend(LPS_i2c_id, LPS_slave_address >> 1, tx_buf, tx_size);

	// Read the register value
	// - Make a receive buffer
	uint8_t rx_size = 1;
	uint8_t rx_buf[rx_size];
	// - Read the register value
	Chip_I2C_MasterRead(LPS_i2c_id, (LPS_slave_address | 0x01) >> 1, rx_buf, rx_size);

	return rx_buf[0];
}

void LPS_write_reg(uint8_t reg_addr, uint8_t data) {
	// Write the register and then the data
	// - Make a transmit buffer
	uint8_t tx_size = 2;
	uint8_t tx_buf[tx_size];
	// - Set the register address
	tx_buf[0] = reg_addr;
	tx_buf[1] = data;
	// - Write the data
	Chip_I2C_MasterSend(LPS_i2c_id, LPS_slave_address >> 1, tx_buf, tx_size);
}
