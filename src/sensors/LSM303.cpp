#include "LSM303.h"

uint8_t LSM303::read_reg(uint8_t reg_addr) {
	read_i2c_register(slave_address | 0x01, slave_address, reg_addr);
	// uint8_t buf[1];
	// int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, reg_addr, buf, 1);
	// return buf[0];
}

void LSM303::write_reg(uint8_t reg_addr, uint8_t data) {
	write_i2c_register(slave_address, reg_addr, data);
	// uint8_t buf[2] = {reg_addr, data};
	// return Chip_I2C_MasterSend(i2c_id, slave_address, buf, 2);
}

bool LSM303::init() {
	return detect_device();
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
			return 0.0f
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
	int n = write_reg(LSM303_CTRL_REG2, 0x20);
	
	// 0x57 = 0b01010111
	// ADDR = 0101 (50 Hz ODR), AZEN = AYEN = AXEN = 1 (all axes enabled)
	n = write_reg(LSM303_CTRL_REG1, 0x57);

	// Magnetometer

	// 0x64 = 0b01100100
	// M_RES = 11 (high resolution mode), M_ODR = 001 (6.25 Hz ODR)
	n = write_reg(LSM303_CTRL_REG5, 0x64)

	// 0x20 = 0b00100000
	// MFS = 01 (+/- 4 gauss full scale)
	n = write_reg(LSM303_CTRL_REG6, 0x20);

	// 0x00 = 00000000
	// MLP = 0 (low power mode off), MD = 00 (continuous-conversion mode)
	n = write_reg(LSM303_CTRL_REG7, 0x00);
}

/*void LSM303::set_timeout(unsigned int timeout) {
	io_timeout = timeout;
}*/

/*unsigned int LSM303::get_timeout() {
	return io_timeout;
}*/

/*bool LSM303::timeout_occured() {
	bool tmp = did_timeout;
	did_timeout = false;
	return tmp;
}*/

int16_t LSM303::read_accel_raw(uint8_t dimension) {
	uint8_t *a_l, *a_h;
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
	int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, reg_addr_l, a_l, 1);
	n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, reg_addr_h, a_h, 1);
	return (int16_t)(*a_h << 8 | *a_l);
}

float LSM303::read_accel_g(uint8_t dimension) {
	return 0.0f + (float)read_accel_raw(dimension) / 2048.0f;
}

int16_t LSM303::read_mag_raw(uint8_t dimension) {
	uint8_t *m_l, *m_h;
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
	int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, reg_addr_l, m_l, 1);
	n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, reg_addr_h, m_h, 1);
	return (int16_t)(*m_h << 8 | *m_l);
}

int16_t LSM303::read_temperature_raw() {
	uint8_t *t_l, *t_h;
	int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, LSM303_OUT_TEMP_L, t_l, 1);
	n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, LSM303_OUT_TEMP_H, t_h, 1);
	return (int16_t)(*t_h << 8 | *t_l);
}

// float LSM303::read_temperature_C() {
// 	return 42.5f + (float)read_temperature_raw() / 480.0f;
// }

// float LSM303::read_mag_heading() {
// 	return heading(vector<int>{1, 0, 0});
// }

// template <typename T> float LSM303::heading(vector<T> from){
//     vector<int32_t> temp_m = {
//     	(int32_t)read_mag_raw(MAG_X),
//     	(int32_t)read_mag_raw(MAG_Y),
//     	(int32_t)read_mag_raw(MAG_Z)
//     }

//     // subtract offset (average of min and max) from magnetometer readings
//     temp_m.x -= ((int32_t)m_min.x + m_max.x) / 2;
//     temp_m.y -= ((int32_t)m_min.y + m_max.y) / 2;
//     temp_m.z -= ((int32_t)m_min.z + m_max.z) / 2;

//     // compute E and N
//     vector<float> E;
//     vector<float> N;
//     vector_cross(&temp_m, &a, &E);
//     vector_normalize(&E);
//     vector_cross(&a, &E, &N);
//     vector_normalize(&N);

//     // compute heading
//     float heading = atan2(vector_dot(&E, &from), vector_dot(&N, &from)) * 180 / M_PI;
//     if (heading < 0) heading += 360;
//     return heading;
// }

// template <typename Ta, typename Tb, typename To> void LSM303::vector_cross(const vector<Ta> *a,const vector<Tb> *b, vector<To> *out){
//     out->x = (a->y * b->z) - (a->z * b->y);
//     out->y = (a->z * b->x) - (a->x * b->z);
//     out->z = (a->x * b->y) - (a->y * b->x);
// }

// template <typename Ta, typename Tb> float LSM303::vector_dot(const vector<Ta> *a, const vector<Tb> *b){
//     return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
// }

// void LSM303::vector_normalize(vector<float> *a){
//     float mag = sqrt(vector_dot(a, a));
//     a->x /= mag;
//     a->y /= mag;
//     a->z /= mag;
// }

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
