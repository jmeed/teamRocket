#include "L3G.h"

uint8_t L3G::read_reg(uint8_t reg_addr) {
	read_i2c_register(slave_address | 0x01, slave_address, reg_addr);
	// uint8_t buf[1];
	// int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, reg_addr, buf, 1);
	// return buf[0];
}

void L3G::write_reg(uint8_t reg_addr, uint8_t data) {
	write_i2c_register(slave_address, reg_addr, data);
	// uint8_t buf[2] = {reg_addr, data};
	// return Chip_I2C_MasterSend(i2c_id, slave_address, buf, 2);
}

bool L3G::init() {
	return detect_device();
}

float L3G::read_data(uint8_t dimension) {
	switch (dimension) {
		case L3G_TEMPERATURE:
			return read_temperature_C();
		default:
			return read_spin_rate_dps(dimension);
	}
}

// Unused so far
void L3G::set_mode(void* mode) {
}

// Unused so far
uint8_t L3G::get_status(uint8_t status) {
	return 0;
}

void L3G::enable() {
	// 0x0F = 0b00001111
	// Normal power mode, all axes enabled
	write_reg(L3G_CTRL_REG1, 0x0F);

	// 0x20 = 0b00100000
	// 2000 dps mode
	write_reg(L3G_CTRL_REG4, 0x20);
}

/*static void L3G::vector_cross(const vector *a, const vector *b, vector *out) {
    out->x = a->y*b->z - a->z*b->y;
    out->y = a->z*b->x - a->x*b->z;
    out->z = a->x*b->y - a->y*b->x;
}*/

/*static float L3G::vector_dot(const vector *a, const vector *b) {
    return a->x*b->x+a->y*b->y+a->z*b->z;
}*/

/*static void L3G::vector_normalize(vector *a) {
    float mag = sqrt(vector_dot(a,a));
    a->x /= mag;
    a->y /= mag;
    a->z /= mag;
}*/

int16_t read_spin_rate_raw(uint8_t dimension) {
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

float read_spin_rate_dps(uint8_t dimension) {
	return 0.0f + (float)read_spin_rate_raw(dimension) / 16.384f;
}

uint8_t read_temperature_raw() {
	uint8_t t;
	return read_reg(L3G_OUT_TEMP);
}

float read_temperature_C() {
	return 42.5f + (float)read_temperature_raw() / 480.0f;
}

bool L3G::detect_device() {
	// Try each possible address and return if reading L3G_WHO_AM_I returns the expected response
	slave_address = L3G_SA0_LOW_ADDRESS;
	if (read_reg(L3G_WHO_AM_I) == L3GD20_WHO_ID_1 || read_reg(L3G_WHO_AM_I) == L3GD20_WHO_ID_2)
		return true;
	slave_address = L3G_SA0_HIGH_ADDRESS;
	if (read_reg(L3G_WHO_AM_I) == L3GD20_WHO_ID_1 || read_reg(L3G_WHO_AM_I) == L3GD20_WHO_ID_2) {
		slave_address = L3G_SA0_LOW_ADDRESS;
		return true;
	}

	return false;
}
