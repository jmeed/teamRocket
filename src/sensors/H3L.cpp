#include "H3L.h"

H3L::H3L(uint8_t slave_address, I2C_ID_T id) {

}

bool H3L::init(accel_scale a_sc) {

}

int16_t H3L::read_accel_raw(uint8_t dimension) {

}

float H3L::read_accel_g(uint8_t dimension) {
	return a_res * (float)read_accel_raw(dimension);
}

void H3L::set_accel_scale(accel_scale a_sc) {

}

void H3L::set_accel_odr(accel_odr a_odr) {

}

void H3L::configure_int_1() {

}

void H3L::configure_int_2() {

}

void H3L::init_accel() {

}

void H3L::calc_a_res() {

}

int8_t H3L::read_reg(uint8_t reg_addr) {
	int8_t buf[1];
	int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, reg_addr, buf, 1);
	return buf[0];
}

void H3L::write_reg(uint8_t) {
	uint8_t buf[2] = {reg_addr, data};
	int n = Chip_I2C_MasterSend(i2c_id, slave_address, buf, 2);
}