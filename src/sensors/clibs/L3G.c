#include "L3G.h"

bool L3G_init(I2C_ID_T id_in) {
	L3G_i2c_id = id_in;
	L3G_slave_address = L3G_SA0_HIGH_ADDRESS;
	return true;
	//return detect_device();
}

float L3G_read_data(uint8_t dimension) {
	switch (dimension) {
		case L3G_TEMPERATURE:
			return read_temperature_C();
		default:
			return read_spin_rate_dps(dimension);
	}
}

void L3G_enable() {
	// 0x0F = 0b00001111
	// Normal power mode, all axes enabled
	write_reg(L3G_CTRL_REG1, 0x0F);

	// 0x20 = 0b00100000
	// 2000 dps mode
	write_reg(L3G_CTRL_REG4, 0x20);
}

int16_t L3G_read_spin_rate_raw(uint8_t dimension) {
	uint8_t r_l, r_h;
	uint8_t reg_addr_l, reg_addr_h;
	switch (dimension) {
		case L3G_SPIN_RATE_X:
			reg_addr_l = L3G_OUT_X_L;
			reg_addr_h = L3G_OUT_X_H;
			break;
		case L3G_SPIN_RATE_Y:
			reg_addr_l = L3G_OUT_Y_L;
			reg_addr_h = L3G_OUT_Y_H;
			break;
		case L3G_SPIN_RATE_Z:
			reg_addr_l = L3G_OUT_Z_L;
			reg_addr_h = L3G_OUT_Z_H;
			break;
		default:
			reg_addr_l = L3G_OUT_X_L;
			reg_addr_h = L3G_OUT_X_H;
			break;
	}
	r_l = read_reg(reg_addr_l);
	r_h = read_reg(reg_addr_h);
	return (int16_t)(r_h << 8 | r_l);
}

float L3G_read_spin_rate_dps(uint8_t dimension) {
	return (float)read_spin_rate_raw(dimension) * (2000.0f / 32768.0f);
}

uint8_t L3G_read_temperature_raw() {
	return read_reg(L3G_OUT_TEMP);
}

float L3G_read_temperature_C() {
	return 42.5f + (float)read_temperature_raw() / 480.0f;
}

// bool L3G_detect_device() {
// 	// Try each possible address and return if reading L3G_WHO_AM_I returns the expected response
// 	slave_address = L3G_SA0_LOW_ADDRESS;
// 	if (read_reg(L3G_WHO_AM_I) == L3GD20_WHO_ID_1 || read_reg(L3G_WHO_AM_I) == L3GD20_WHO_ID_2)
// 		return true;
// 	slave_address = L3G_SA0_HIGH_ADDRESS;
// 	if (read_reg(L3G_WHO_AM_I) == L3GD20_WHO_ID_1 || read_reg(L3G_WHO_AM_I) == L3GD20_WHO_ID_2) {
// 		return true;
// 	}

// 	return false;
// }

uint8_t L3G_read_reg(uint8_t reg_addr) {
	// Write the register we want to read
	// - Make a transmit buffer
	uint8_t tx_size = 1;
	uint8_t tx_buf[tx_size];
	// - Set the register address
	tx_buf[0] = reg_addr;
	// - Write the register value
	Chip_I2C_MasterSend(L3G_i2c_id, L3G_slave_address >> 1, tx_buf, tx_size);

	// Read the register value
	// - Make a receive buffer
	uint8_t rx_size = 1;
	uint8_t rx_buf[rx_size];
	// - Read the register value
	Chip_I2C_MasterRead(L3G_i2c_id, (L3G_slave_address | 0x01) >> 1, rx_buf, rx_size);

	return rx_buf[0];
}

void L3G_write_reg(uint8_t reg_addr, uint8_t data) {
	// Write the register and then the data
	// - Make a transmit buffer
	uint8_t tx_size = 2;
	uint8_t tx_buf[tx_size];
	// - Set the register address
	tx_buf[0] = reg_addr;
	tx_buf[1] = data;
	// - Write the data
	Chip_I2C_MasterSend(L3G_i2c_id, L3G_slave_address >> 1, tx_buf, tx_size);
}
