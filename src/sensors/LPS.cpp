#include "LPS.h"

int8_t LPS::read_reg(uint8_t addr) {

}

void LPS::write_reg(uint8_t addr, int8_t data) {

}

bool LPS::init() {
	return detect_device();
}

int32_t LPS::read_data(uint8_t dimension) {
	switch (dimension) {
		case ALTITUDE:
			float p_mbar = read_pressure_millibars();
			return pressure_to_altitude_m(p_mbar);
		case TEMPERATURE:
			return read_temperature_C();
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

LPS::LPS() {
	address = SA0_HIGH_ADDRESS;
}

void LPS::enable() {
	// 0xE0 = 0b11100000
	// PD = 1 (active mode); ODR = 110 (12.5 Hz pressure & temperature output data rate)
	write_reg(CTRL_REG1, 0xE0);
}

int32_t LPS::read_pressure_raw() {

}

float LPS::read_pressure_millibars() {
	return (float)read_pressure_raw() / 4096.0f;
}

int16_t LPS::read_temperature_raw() {

}

float LPS::read_temperature_C() {
	return 42.5f + (float)read_temperature_raw() / 480.0f;
}

static float LPS::pressure_to_altitude_m(float pressure_mbar, float altimeter_setting_mbar = 1013.25) {
	return (1 - pow(pressure_mbar / altimeter_setting_mbar, 0.190263f)) * 44330.8f;
}

bool LPS::detect_device() {


	if (id == 0xBB) {
		return true;
	} else {
		return false;
	}
}
