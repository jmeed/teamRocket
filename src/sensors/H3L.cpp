#include "H3L.h"

H3L::H3L() {
}

bool H3L::init(I2C_ID_T id_in, accel_scale a_sc, accel_odr a_odr) {
	// Set class variables
	slave_address = H3L_SA0_HIGH_ADDRESS;
	i2c_id = id_in;
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
	Chip_I2C_MasterRead(i2c_id, (slave_address | 0x01 >> 1), rx_buf, rx_size);

	return rx_buf[0];
}

void H3L::write_reg(uint8_t reg_addr, uint8_t data) {
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