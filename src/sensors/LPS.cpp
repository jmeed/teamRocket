#include "LPS.h"

bool LPS::init(I2C_ID_T id_in) {
	i2c_id = id_in;
	slave_address = LPS_SA0_HIGH_ADDRESS;
	return true;
	//return detect_device();
}

float LPS::read_data(uint8_t dimension) {
	switch (dimension) {
		case LPS_ALTITUDE:
			float p_mbar = read_pressure_millibars();
			return pressure_to_altitude_m(p_mbar);
		case LPS_TEMPERATURE:
			return read_temperature_C();
		default:
			return 0.0f;
	}
}

// Device specific members

void LPS::enable() {
	// 0xE0 = 0b11100000
	// PD = 1 (active mode); ODR = 110 (12.5 Hz pressure & temperature output data rate)
	write_reg(LPS_CTRL_REG1, 0xE0);
}

int32_t LPS::read_pressure_raw() {
	uint8_t p_xl, p_l, p_h;
	p_xl = read_reg(LPS_OUT_PRESS_XL);
	p_l = read_reg(LPS_OUT_PRESS_L);
	p_h = read_reg(LPS_OUT_PRESS_H);
	return (int32_t)(int8_t)p_h << 16 | (uint16_t)p_l << 8 | p_xl;
}

float LPS::read_pressure_millibars() {
	return (float)read_pressure_raw() / 4096.0f;
}

int16_t LPS::read_temperature_raw() {
	uint8_t t_l, t_h;
	t_l = read_reg(LPS_OUT_TEMP_L);
	t_h = read_reg(LPS_OUT_TEMP_H);
	return (int16_t)(t_h << 8 | t_l);
}

float LPS::read_temperature_C() {
	return 42.5f + (float)read_temperature_raw() / 480.0f;
}

static float LPS::pressure_to_altitude_m(float pressure_mbar, float altimeter_setting_mbar = 1013.25) {
	return (1 - pow(pressure_mbar / altimeter_setting_mbar, 0.190263f)) * 44330.8f;
}

bool LPS::detect_device() {
	slave_address = LPS_SA0_LOW_ADDRESS;
	if (read_reg(LPS_WHO_AM_I) == LPS331AP_WHO_ID)
		return true;
	slave_address = LPS_SA0_HIGH_ADDRESS;
	if (read_reg(LPS_WHO_AM_I) == LPS331AP_WHO_ID) {
		slave_address = LPS_SA0_LOW_ADDRESS;
		return true;
	}

	return false;
}

uint8_t LPS::read_reg(uint8_t reg_addr) {
	// Write the register we want to read
	// - Make a transmit buffer
	uint8_t tx_size = 1;
	uint8_t tx_buf[tx_size];
	// - Set the register address
	tx_buf[0] = reg_addr;
	// - Write the register value
	Chip_I2C_MasterSend(i2c_id, slave_address >> 1, tx_buf, tx_size);

	// Read the register value
	// - Make a receive buffer
	uint8_t rx_size = 1;
	uint8_t rx_buf[rx_size];
	// - Read the register value
	Chip_I2C_MasterRead(i2c_id, (slave_address | 0x01) >> 1, rx_buf, rx_size);

	return rx_buf[0];
}

void LPS::write_reg(uint8_t reg_addr, uint8_t data) {
	// Write the register and then the data
	// - Make a transmit buffer
	uint8_t tx_size = 2;
	uint8_t tx_buf[tx_size];
	// - Set the register address
	tx_buf[0] = reg_addr;
	tx_buf[1] = data;
	// - Write the data
	Chip_I2C_MasterSend(i2c_id, slave_address >> 1, tx_buf, tx_size);
}
