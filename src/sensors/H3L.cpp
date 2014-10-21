#include "H3L.h"

H3L::H3L(uint8_t slave_addr) {
	slave_address = slave_addr;
}

bool H3L::init(accel_scale a_sc, accel_odr a_odr) {
	// Set class variable
	a_scale = a_sc;

	// Test if device responds, return false if not
	if (!(read_reg(H3L_WHO_AM_I) == H3L_WHO_ID))
		return false;

	// Accel initialization
	init_accel();
	set_accel_odr(a_odr);
	set_accel_scale(a_sc);
	calc_a_res();

	// We communicated successfully
	return true;
}

int16_t H3L::read_accel_raw(uint8_t dimension) {
	uint8_t reg_addr_l, reg_addr_h, accel_l, accel_h;
	switch (dimension) {
		case H3L_X:
			reg_addr_l = H3L_OUT_X_L;
			reg_addr_h = H3L_OUT_X_H;
			break;
		case H3L_Y:
			reg_addr_l = H3L_OUT_Y_L;
			reg_addr_h = H3L_OUT_Y_H;
			break;
		case H3L_Z:
			reg_addr_l = H3L_OUT_Z_L;
			reg_addr_h = H3L_OUT_Z_H;
			break;
		default:
			reg_addr_l = H3L_OUT_X_L;
			reg_addr_h = H3L_OUT_X_H;
	}
	accel_l = read_reg(reg_addr_l);
	accel_h = read_reg(reg_addr_h);
	return (int16_t)(accel_h << 8 | accel_l);
}

float H3L::read_accel_g(uint8_t dimension) {
	return a_res * (float)read_accel_raw(dimension);
}

void H3L::set_accel_scale(accel_scale a_sc) {
	// Get current reg value
	uint8_t ctrl = read_reg(H3L_CTRL_REG4);
	// Mask out scale bits
	ctrl &= 0xFF ^ (0x03 << 4);
	// Set scale bits
	ctrl |= (a_sc << 4);
	// Write back to reg
	write_reg(H3L_CTRL_REG4, ctrl);
}

void H3L::set_accel_odr(accel_odr a_odr) {
	// Get current reg value
	uint8_t ctrl = read_reg(H3L_CTRL_REG1);
	// Mask out ODR bits
	ctrl &= 0xFF ^ (0x03 << 3);
	// Set ODR bits
	ctrl |= (a_odr << 3);
	// Write back to reg
	write_reg(H3L_CTRL_REG1, ctrl);
}

void H3L::configure_int_1(uint8_t int1_cfg, uint8_t int1_ths, uint8_t duration) {
	write_reg(H3L_INT1_CFG, 0xBF & int1_cfg); // 6th bit always 0
	write_reg(H3L_INT1_THS, 0x7F & int1_ths); // 7th bit always 0
	write_reg(H3L_INT1_DURATION, 0x7F & duration); // 7th bit always 0
}

void H3L::configure_int_2(uint8_t int2_cfg, uint8_t int2_ths, uint8_t duration) {
	write_reg(H3L_INT2_CFG, 0xBF & int2_cfg); // 6th bit always 0
	write_reg(H3L_INT2_THS, 0x7F & int2_ths); // 7th bit always 0
	write_reg(H3L_INT2_DURATION, 0x7F & duration); // 7th bit always 0
}

void H3L::init_accel() {
	write_reg(H3L_CTRL_REG1, 0x3F);
	write_reg(H3L_CTRL_REG2, 0x00);
	write_reg(H3L_CTRL_REG3, 0x00);
	write_reg(H3L_CTRL_REG4, 0x30);
	write_reg(H3L_CTRL_REG5, 0x00);
}

void H3L::calc_a_res() {
	switch (a_scale) {
		case H3L_SCALE_100G:
			a_res = 100.0f / 32768.0f;
			break;
		case H3L_SCALE_200G:
			a_res = 200.0f / 32768.0f;
			break;
		case H3L_SCALE_400G:
			a_res = 400.0f / 32768.0f;
			break;
		default:
			a_res = 1.0f;
	}
}

int8_t H3L::read_reg(uint8_t reg_addr) {
	read_i2c_register(slave_address | 0x01, slave_address, reg_addr);
	// int8_t buf[1];
	// int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, reg_addr, buf, 1);
	// return buf[0];
}

void H3L::write_reg(uint8_t reg_addr, uint8_t data) {
	write_i2c_register(slave_address, reg_addr, data);
	// uint8_t buf[2] = {reg_addr, data};
	// int n = Chip_I2C_MasterSend(i2c_id, slave_address, buf, 2);
}