#include "LPS.h"

int8_t LPS::read_reg(uint8_t reg_addr) {
	uint8_t buf[1];
	int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, reg_addr, buf, 1);
	return buf[0];
}

int LPS::write_reg(uint8_t reg_addr, uint8_t data) {
	uint8_t buf[2] = {reg_addr, data};
	return Chip_I2C_MasterSend(i2c_id, slave_address, buf, 2);
}

bool LPS::init(I2C_ID_T in) {
	i2c_id = in;
	return detect_device();
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

// Unused so far
void LPS::set_mode(void* mode) {
}

// Unused so far
uint8_t LPS::get_status(uint8_t status) {
	return 0;
}

// Device specific members

void LPS::enable() {
	// 0xE0 = 0b11100000
	// PD = 1 (active mode); ODR = 110 (12.5 Hz pressure & temperature output data rate)
	write_reg(LPS_CTRL_REG1, 0xE0);
}

int32_t LPS::read_pressure_raw() {
	// assert MSB to enable register address auto-increment
	uint8_t *p_xl, *p_l, *p_h;
	int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, LPS_OUT_PRESS_XL, p_xl, 1);
	n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, LPS_OUT_PRESS_L, p_l, 1);
	n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, LPS_OUT_PRESS_H, p_h, 1);
	return (int32_t)(int8_t)*p_h << 16 | (uint16_t)*p_l << 8 | p_xl;
}

float LPS::read_pressure_millibars() {
	return (float)read_pressure_raw() / 4096.0f;
}

int16_t LPS::read_temperature_raw() {
	uint8_t *t_l, *t_h;
	int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, LPS_OUT_TEMP_L, t_l, 1);
	n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, LPS_OUT_TEMP_H, t_h, 1);
	return (int16_t)(*t_h << 8 | *t_l);
}

float LPS::read_temperature_C() {
	return 42.5f + (float)read_temperature_raw() / 480.0f;
}

static float LPS::pressure_to_altitude_m(float pressure_mbar, float altimeter_setting_mbar = 1013.25) {
	return (1 - pow(pressure_mbar / altimeter_setting_mbar, 0.190263f)) * 44330.8f;
}

bool LPS::detect_device() {
	/*uint8_t *id;
	int n = Chip_I2C_MasterCmdRead(i2c_id, slave_address, WHO_AM_I, buf, 1);
	if (*id == 0xBB) {
		return true;
	} else {
		return false;
	}*/
	slave_address = LPS_SA0_HIGH_ADDRESS;
	if (read_reg(LPS_WHO_AM_I) == LPS331AP_WHO_ID)
		return true;
	slave_address = SA0_LOW_ADDRESS;
	if (read_reg(LPS_WHO_AM_I) == LPS331AP_WHO_ID)
		return true;

	return false;
}
