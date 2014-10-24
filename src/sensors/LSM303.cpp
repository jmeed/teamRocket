#include "LSM303.h"

uint8_t LSM303::read_reg(uint8_t reg_addr) {
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

void LSM303::write_reg(uint8_t reg_addr, uint8_t data) {
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

bool LSM303::init(I2C_ID_T id_in) {
	i2c_id = id_in;
	slave_address = LSM303_SA0_HIGH_ADDRESS;
	return true;
	//return detect_device();
}

float LSM303::read_data(uint8_t dimension) {
	switch (dimension) {
		case LSM303_ACCEL_X:
		case LSM303_ACCEL_Y:
		case LSM303_ACCEL_Z:
			return read_accel_g(dimension);
		case LSM303_MAG_X:
		case LSM303_MAG_Y:
		case LSM303_MAG_Z:
			return read_mag_raw(dimension);
		case LSM303_MAG_HEADING:
			return 0.0f;
			//return read_mag_heading();
		case LSM303_TEMPERATURE:
			return read_temperature_C();
		default:
			return 0.0f;
	}
}

void LSM303::set_mode(void* mode) {
}

uint8_t LSM303::get_status(uint8_t status) {
	return 0;
}

void LSM303::enable() {
	// Accelerometer

	// 0x20 = 0b00100000
	// AFS = 0b100 (+/- 16 g full scale)
	write_reg(LSM303_CTRL_REG2, 0x20);
	
	// 0x57 = 0b01010111
	// ADDR = 0101 (50 Hz ODR), AZEN = AYEN = AXEN = 1 (all axes enabled)
	write_reg(LSM303_CTRL_REG1, 0x57);

	// Magnetometer

	// 0x64 = 0b01100100
	// M_RES = 11 (high resolution mode), M_ODR = 001 (6.25 Hz ODR)
	write_reg(LSM303_CTRL_REG5, 0x64);

	// 0x20 = 0b00100000
	// MFS = 01 (+/- 4 gauss full scale)
	write_reg(LSM303_CTRL_REG6, 0x20);

	// 0x00 = 00000000
	// MLP = 0 (low power mode off), MD = 00 (continuous-conversion mode)
	write_reg(LSM303_CTRL_REG7, 0x00);
}

int16_t LSM303::read_accel_raw(uint8_t dimension) {
	uint8_t a_l, a_h;
	uint8_t reg_addr_l, reg_addr_h;
	switch (dimension) {
		case LSM303_ACCEL_X:
			reg_addr_l = LSM303_OUT_X_L_A;
			reg_addr_h = LSM303_OUT_X_H_A;
			break;
		case LSM303_ACCEL_Y:
			reg_addr_l = LSM303_OUT_Y_L_A;
			reg_addr_h = LSM303_OUT_Y_H_A;
			break;
		case LSM303_ACCEL_Z:
			reg_addr_l = LSM303_OUT_Z_L_A;
			reg_addr_h = LSM303_OUT_Z_H_A;
			break;
		default:
			reg_addr_l = LSM303_OUT_X_L_A;
			reg_addr_h = LSM303_OUT_X_H_A;
			break;
	}
	a_l = read_reg(reg_addr_l);
	a_h = read_reg(reg_addr_h);
	return (int16_t)(a_h << 8 | a_l);
}

float LSM303::read_accel_g(uint8_t dimension) {
	return 0.0f + (float)read_accel_raw(dimension) / 2048.0f;
}

int16_t LSM303::read_mag_raw(uint8_t dimension) {
	uint8_t m_l, m_h;
	uint8_t reg_addr_l, reg_addr_h;
	switch (dimension) {
		case LSM303_MAG_X:
			reg_addr_l = LSM303_OUT_X_L_M;
			reg_addr_h = LSM303_OUT_X_H_M;
			break;
		case LSM303_MAG_Y:
			reg_addr_l = LSM303_OUT_Y_L_M;
			reg_addr_h = LSM303_OUT_Y_H_M;
			break;
		case LSM303_MAG_Z:
			reg_addr_l = LSM303_OUT_Z_L_M;
			reg_addr_h = LSM303_OUT_Z_H_M;
			break;
		default:
			reg_addr_l = LSM303_OUT_X_L_M;
			reg_addr_h = LSM303_OUT_X_H_M;
			break;
	}
	m_l = read_reg(reg_addr_l);
	m_h = read_reg(reg_addr_h);
	return (int16_t)(m_h << 8 | m_l);
}

int16_t LSM303::read_temperature_raw() {
	uint8_t t_l, t_h;
	t_l = read_reg(LSM303_OUT_TEMP_L);
	t_h = read_reg(LSM303_OUT_TEMP_H);
	return (int16_t)(t_h << 8 | t_l);
}

bool LSM303::detect_device() {
	slave_address = LSM303_SA0_LOW_ADDRESS;
	if (read_reg(LSM303_WHO_AM_I) == LSM303D_WHO_ID)
		return true;
	slave_address = LSM303_SA0_HIGH_ADDRESS;
	if (read_reg(LSM303_WHO_AM_I) == LSM303D_WHO_ID) {
		slave_address = LSM303_SA0_LOW_ADDRESS;
		return true;
	}

	return false;
}
