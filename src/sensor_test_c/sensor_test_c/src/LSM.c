#include "LSM.h"

int LSM_init(I2C_ID_T id_in,
		enum LSM_gyro_scale g_sc,
		enum LSM_accel_scale a_sc,
		enum LSM_mag_scale m_sc,
		enum LSM_gyro_odr g_odr,
		enum LSM_accel_odr a_odr,
		enum LSM_mag_odr m_odr) {
	
	// Set scale class variables
	LSM_xlg_address = LSM_XLG_SA0_HIGH_ADDRESS;
	LSM_mag_address = LSM_MAG_SA0_HIGH_ADDRESS;
	LSM_i2c_id = id_in;
	LSM_g_scale = g_sc;
	LSM_a_scale = a_sc;
	LSM_m_scale = m_sc;
	
	// Test if our devices respond
	uint8_t xlg_test = LSM_read_reg_xlg(LSM_WHO_AM_I_XLG);
	uint8_t m_test = LSM_read_reg_mag(LSM_WHO_AM_I_M);
	// Return false if we didn't communicate successfully
	if (!((xlg_test == LSM_XLG_WHO_ID) && (m_test == LSM_M_WHO_ID)))
		return 0;

	// Gyro initialization
	LSM_init_gyro(); 				// Turn on
	LSM_set_gyro_odr(g_odr);		// Set data rate and bandwidth
	LSM_set_gyro_scale(LSM_g_scale);	// Set range
	
	// Accel initialization
	LSM_init_accel();
	LSM_set_accel_odr(a_odr);
	LSM_set_accel_scale(LSM_a_scale);

	// Mag initialization
	LSM_init_mag();
	LSM_set_mag_odr(m_odr);
	LSM_set_mag_scale(LSM_m_scale);
	
	// Now calculate the resolution of each sensor
	LSM_calc_g_res(); // Calculate DPS / ADC tick, stored in g_res variable
	LSM_calc_a_res(); // Calculate g / ADC tick, stored in a_res variable
	LSM_calc_m_res(); // Calculate Gs / ADC tick, stored in m_res variable
	
	// We communicated successfully
	return 1;
}

int16_t LSM_read_gyro_raw(uint8_t dimension) {
	int8_t out_l, out_h;
	uint8_t reg_addr_l, reg_addr_h;
	switch (dimension) {
		case LSM_GYRO_X:
			reg_addr_l = LSM_OUT_X_L_G;
			reg_addr_h = LSM_OUT_X_H_G;
			break;
		case LSM_GYRO_Y:
			reg_addr_l = LSM_OUT_Y_L_G;
			reg_addr_h = LSM_OUT_Y_H_G;
			break;
		case LSM_GYRO_Z:
			reg_addr_l = LSM_OUT_Z_L_G;
			reg_addr_h = LSM_OUT_Z_H_G;
			break;
		default:
			reg_addr_l = LSM_OUT_X_L_G;
			reg_addr_h = LSM_OUT_X_H_G;
			break;
	}
	out_l = LSM_read_reg_xlg(reg_addr_l);
	out_h = LSM_read_reg_xlg(reg_addr_h);
	return (int16_t)(out_h << 8 | out_l);
}

float LSM_read_gyro_dps(uint8_t dimension) {
	return LSM_g_res * (float)LSM_read_gyro_raw(dimension);
}

int16_t LSM_read_accel_raw(uint8_t dimension) {
	int8_t out_l, out_h;
	uint8_t reg_addr_l, reg_addr_h;
	switch (dimension) {
		case LSM_ACCEL_X:
			reg_addr_l = LSM_OUT_X_L_XL;
			reg_addr_h = LSM_OUT_X_H_XL;
			break;
		case LSM_ACCEL_Y:
			reg_addr_l = LSM_OUT_Y_L_XL;
			reg_addr_h = LSM_OUT_Y_H_XL;
			break;
		case LSM_ACCEL_Z:
			reg_addr_l = LSM_OUT_Z_L_XL;
			reg_addr_h = LSM_OUT_Z_H_XL;
			break;
		default:
			reg_addr_l = LSM_OUT_X_L_XL;
			reg_addr_h = LSM_OUT_X_H_XL;
			break;
	}
	out_l = LSM_read_reg_xlg(reg_addr_l);
	out_h = LSM_read_reg_xlg(reg_addr_h);
	return (int16_t)(out_h << 8 | out_l);
}

float LSM_read_accel_g(uint8_t dimension) {
	return LSM_a_res * (float)LSM_read_accel_raw(dimension);
}

int16_t LSM_read_mag_raw(uint8_t dimension) {
	int8_t out_l, out_h;
	uint8_t reg_addr_l, reg_addr_h;
	switch (dimension) {
		case LSM_MAG_X:
			reg_addr_l = LSM_OUT_X_L_M;
			reg_addr_h = LSM_OUT_X_H_M;
			break;
		case LSM_MAG_Y:
			reg_addr_l = LSM_OUT_Y_L_M;
			reg_addr_h = LSM_OUT_Y_H_M;
			break;
		case LSM_MAG_Z:
			reg_addr_l = LSM_OUT_Z_L_M;
			reg_addr_h = LSM_OUT_Z_H_M;
			break;
		default:
			reg_addr_l = LSM_OUT_X_L_M;
			reg_addr_h = LSM_OUT_X_H_M;
			break;
	}
	out_l = LSM_read_reg_mag(reg_addr_l);
	out_h = LSM_read_reg_mag(reg_addr_h);
	return (int16_t)(out_h << 8 | out_l);
}

float LSM_read_mag_gs(uint8_t dimension) {
	return LSM_m_res * (float)LSM_read_mag_raw(dimension);
}

int16_t LSM_read_temperature_raw() {
	int8_t out_l, out_h;
	out_l = LSM_read_reg_xlg(LSM_OUT_TEMP_L);
	out_h = LSM_read_reg_xlg(LSM_OUT_TEMP_H);
	return ((int16_t)(out_h << 12 | out_l << 4)) >> 4; // 12 bit signed integer
}

float LSM_read_temperature_C() {
	return (float)LSM_read_temperature_raw();
}

void LSM_set_gyro_scale(enum LSM_gyro_scale g_sc) {
	// Get current reg value
	uint8_t ctrl = (uint8_t)LSM_read_reg_xlg(LSM_CTRL_REG1_G);
	// Mask out scale bits
	ctrl &= 0xFF ^ (0x3 << 3);
	// Set scale bits
	ctrl |= (g_sc << 3);
	// Write back to reg
	LSM_write_reg_xlg(LSM_CTRL_REG1_G, ctrl);
	// Update class variable
	LSM_g_scale = g_sc;
	LSM_calc_g_res();
}

void LSM_set_accel_scale(enum LSM_accel_scale a_sc) {
	// Get current reg value
	uint8_t ctrl = (uint8_t)LSM_read_reg_xlg(LSM_CTRL_REG6_XL);
	// Mask out scale bits
	ctrl &= 0xFF ^ (0x3 << 3);
	// Set scale bits
	ctrl |= (a_sc << 3);
	// Write back to reg
	LSM_write_reg_xlg(LSM_CTRL_REG6_XL, ctrl);
	// Update class variable
	LSM_a_scale = a_sc;
	LSM_calc_a_res();
}

void LSM_set_mag_scale(enum LSM_mag_scale m_sc) {
	// Get current reg value
	uint8_t ctrl = (uint8_t)LSM_read_reg_mag(LSM_CTRL_REG2_M);
	// Mask out scale bits
	ctrl &= 0xFF ^ (0x3 << 5);
	// Set scale bits
	ctrl |= (m_sc << 5);
	// Write back to reg
	LSM_write_reg_mag(LSM_CTRL_REG2_M, ctrl);
	// Update class variable
	LSM_m_scale = m_sc;
	LSM_calc_m_res();
}

void LSM_set_gyro_odr(enum LSM_gyro_odr g_odr) {
	// Get current reg value
	uint8_t ctrl = (uint8_t)LSM_read_reg_xlg(LSM_CTRL_REG1_G);
	// Mask out ODR bits
	ctrl &= 0xFF ^ (0x7 << 5);
	// Set ODR bits
	ctrl |= (g_odr << 5);
	// Write back to reg
	LSM_write_reg_xlg(LSM_CTRL_REG1_G, ctrl);
}

void LSM_set_accel_odr(enum LSM_accel_odr a_odr) {
	// Get current reg value
	uint8_t ctrl = (uint8_t)LSM_read_reg_xlg(LSM_CTRL_REG6_XL);
	// Mask out ODR bits
	ctrl &= 0xFF ^ (0x7 << 5);
	// Set ODR bits
	ctrl |= (a_odr << 5);
	// Write back to reg
	LSM_write_reg_xlg(LSM_CTRL_REG6_XL, ctrl);
}

void LSM_set_mag_odr(enum LSM_mag_odr m_odr) {
	// Get current reg value
	uint8_t ctrl = (uint8_t)LSM_read_reg_mag(LSM_CTRL_REG1_M);
	// Mask out ODR bits
	ctrl &= 0xFF ^ (0x7 << 2);
	// Set ODR bits
	ctrl |= (m_odr << 2);
	// Write back to reg
	LSM_write_reg_mag(LSM_CTRL_REG1_M, ctrl);
}

void LSM_set_accel_abw(enum LSM_accel_abw a_abw) {
	// Get current reg value
	uint8_t ctrl = (uint8_t)LSM_read_reg_xlg(LSM_CTRL_REG6_XL);
	// Mask out ABW bits
	ctrl &= 0xFF ^ 0x3;
	// Set ABW bits
	ctrl |= a_abw;
	// Write back to reg
	LSM_write_reg_xlg(LSM_CTRL_REG6_XL, ctrl);
}

void LSM_configure_gyro_int(uint8_t int1_cfg, uint16_t int1_ths_x,
		uint16_t int1_ths_y, uint16_t int1_ths_z, uint8_t duration) {
	// Write directly to each register
	LSM_write_reg_xlg(LSM_INT_GEN_CFG_G, int1_cfg);
	LSM_write_reg_xlg(LSM_INT_GEN_THS_XH_G, (int1_ths_x & 0xFF00) >> 8);
	LSM_write_reg_xlg(LSM_INT_GEN_THS_XL_G, (int1_ths_x & 0xFF));
	LSM_write_reg_xlg(LSM_INT_GEN_THS_YH_G, (int1_ths_y & 0xFF00) >> 8);
	LSM_write_reg_xlg(LSM_INT_GEN_THS_YL_G, (int1_ths_y & 0xFF));
	LSM_write_reg_xlg(LSM_INT_GEN_THS_ZH_G, (int1_ths_z & 0xFF00) >> 8);
	LSM_write_reg_xlg(LSM_INT_GEN_THS_ZL_G, (int1_ths_z & 0xFF));
	if (duration)
		LSM_write_reg_xlg(LSM_INT_GEN_DUR_G, 0x80 | duration);
	else
		LSM_write_reg_xlg(LSM_INT_GEN_DUR_G, 0x00);
}

void LSM_configure_accel_int(uint8_t int1_cfg, uint8_t int1_ths_x,
		uint8_t int1_ths_y, uint8_t int1_ths_z, uint8_t duration) {
	// Write directly to each register
	LSM_write_reg_xlg(LSM_INT_GEN_CFG_XL, int1_cfg);
	LSM_write_reg_xlg(LSM_INT_GEN_THS_X_XL, (int1_ths_x & 0xFF));
	LSM_write_reg_xlg(LSM_INT_GEN_THS_Y_XL, (int1_ths_y & 0xFF));
	LSM_write_reg_xlg(LSM_INT_GEN_THS_Z_XL, (int1_ths_z & 0xFF));
	if (duration)
		LSM_write_reg_xlg(LSM_INT_GEN_DUR_XL, 0x80 | duration);
	else
		LSM_write_reg_xlg(LSM_INT_GEN_DUR_XL, 0x00);
}

void LSM_configure_mag_int(uint8_t int1_cfg, uint16_t int1_ths) {
	// Write directly to each register
	LSM_write_reg_mag(LSM_INT_CFG_M, int1_cfg);
	LSM_write_reg_mag(LSM_INT_THS_H_M, (int1_ths & 0xFF00) >> 8);
	LSM_write_reg_mag(LSM_INT_THS_L_M, (int1_ths & 0xFF));
}

/*void LSM_calibrate(float g_bias[3], float a_bias[3]) {
  uint8_t data[6] = {0, 0, 0, 0, 0, 0};
  int32_t gyro_bias[3] = {0, 0, 0}, accel_bias[3] = {0, 0, 0};
  uint16_t samples, ii;
  
  // First get gyro bias
  byte c = gReadByte(CTRL_REG5_G);
  gWriteByte(CTRL_REG5_G, c | 0x40);         // Enable gyro FIFO  
  delay(20);                                 // Wait for change to take effect
  gWriteByte(FIFO_CTRL_REG_G, 0x20 | 0x1F);  // Enable gyro FIFO stream mode and set watermark at 32 samples
  delay(1000);  // delay 1000 milliseconds to collect FIFO samples
  
  samples = (gReadByte(FIFO_SRC_REG_G) & 0x1F); // Read number of stored samples

  for(ii = 0; ii < samples ; ii++) {            // Read the gyro data stored in the FIFO
    int16_t gyro_temp[3] = {0, 0, 0};
    gReadBytes(OUT_X_L_G,  &data[0], 6);
    gyro_temp[0] = (int16_t) (((int16_t)data[1] << 8) | data[0]); // Form signed 16-bit integer for each sample in FIFO
    gyro_temp[1] = (int16_t) (((int16_t)data[3] << 8) | data[2]);
    gyro_temp[2] = (int16_t) (((int16_t)data[5] << 8) | data[4]);

    gyro_bias[0] += (int32_t) gyro_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
    gyro_bias[1] += (int32_t) gyro_temp[1]; 
    gyro_bias[2] += (int32_t) gyro_temp[2]; 
  }  

  gyro_bias[0] /= samples; // average the data
  gyro_bias[1] /= samples; 
  gyro_bias[2] /= samples; 
  
  g_bias[0] = (float)gyro_bias[0]*gRes;  // Properly scale the data to get deg/s
  g_bias[1] = (float)gyro_bias[1]*gRes;
  g_bias[2] = (float)gyro_bias[2]*gRes;
  
  c = gReadByte(CTRL_REG5_G);
  gWriteByte(CTRL_REG5_G, c & ~0x40);  // Disable gyro FIFO  
  delay(20);
  gWriteByte(FIFO_CTRL_REG_G, 0x00);   // Enable gyro bypass mode
  

  //  Now get the accelerometer biases
  c = xmReadByte(CTRL_REG0_XM);
  xmWriteByte(CTRL_REG0_XM, c | 0x40);      // Enable accelerometer FIFO  
  delay(20);                                // Wait for change to take effect
  xmWriteByte(FIFO_CTRL_REG, 0x20 | 0x1F);  // Enable accelerometer FIFO stream mode and set watermark at 32 samples
  delay(1000);  // delay 1000 milliseconds to collect FIFO samples

  samples = (xmReadByte(FIFO_SRC_REG) & 0x1F); // Read number of stored accelerometer samples

   for(ii = 0; ii < samples ; ii++) {          // Read the accelerometer data stored in the FIFO
    int16_t accel_temp[3] = {0, 0, 0};
    xmReadBytes(OUT_X_L_A, &data[0], 6);
    accel_temp[0] = (int16_t) (((int16_t)data[1] << 8) | data[0]);// Form signed 16-bit integer for each sample in FIFO
    accel_temp[1] = (int16_t) (((int16_t)data[3] << 8) | data[2]);
    accel_temp[2] = (int16_t) (((int16_t)data[5] << 8) | data[4]);  

    accel_bias[0] += (int32_t) accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
    accel_bias[1] += (int32_t) accel_temp[1]; 
    accel_bias[2] += (int32_t) accel_temp[2]; 
  }  

  accel_bias[0] /= samples; // average the data
  accel_bias[1] /= samples; 
  accel_bias[2] /= samples; 

  if(accel_bias[2] > 0L) {accel_bias[2] -= (int32_t) (1.0/aRes);}  // Remove gravity from the z-axis accelerometer bias calculation
  else {accel_bias[2] += (int32_t) (1.0/aRes);}
 
  
  a_bias[0] = (float)accel_bias[0]*aRes; // Properly scale data to get gs
  a_bias[1] = (float)accel_bias[1]*aRes;
  a_bias[2] = (float)accel_bias[2]*aRes;

  c = xmReadByte(CTRL_REG0_XM);
  xmWriteByte(CTRL_REG0_XM, c & ~0x40);    // Disable accelerometer FIFO  
  delay(20);
  xmWriteByte(FIFO_CTRL_REG, 0x00);       // Enable accelerometer bypass mode
}*/

void LSM_init_gyro() {
	// See header file for command descriptions
	LSM_write_reg_xlg(LSM_CTRL_REG1_G, 0x38);
	LSM_write_reg_xlg(LSM_CTRL_REG2_G, 0x00);
	LSM_write_reg_xlg(LSM_CTRL_REG3_G, 0x00);
	LSM_write_reg_xlg(LSM_CTRL_REG4, 0x38);
}

void LSM_init_accel() {
	// See header file for command descriptions
	LSM_write_reg_xlg(LSM_CTRL_REG5_XL, 0xF8);
	LSM_write_reg_xlg(LSM_CTRL_REG6_XL, 0xD8);
	LSM_write_reg_xlg(LSM_CTRL_REG7_XL, 0x00);
}

void LSM_init_mag() {
	// See header file for command descriptions
	LSM_write_reg_mag(LSM_CTRL_REG1_M, 0xC0);
	LSM_write_reg_mag(LSM_CTRL_REG2_M, 0x00);
	LSM_write_reg_mag(LSM_CTRL_REG3_M, 0x00);
	LSM_write_reg_mag(LSM_CTRL_REG4_M, 0x0C);
	LSM_write_reg_mag(LSM_CTRL_REG5_M, 0x00);
}

void LSM_calc_g_res() {
	switch (LSM_g_scale) {
		case G_SCALE_245DPS:
			LSM_g_res = 245.0f / 32768.0f;
			break;
		case G_SCALE_500DPS:
			LSM_g_res = 500.0f / 32768.0f;
			break;
		case G_SCALE_2000DPS:
			LSM_g_res = 2000.0f / 32768.0f;
			break;
		default:
			LSM_g_res = 1.0f;
	}
}

void LSM_calc_a_res() {
	switch (LSM_a_scale) {
		case A_SCALE_2G:
			LSM_a_res = 2.0f / 32768.0f;
			break;
		case A_SCALE_4G:
			LSM_a_res = 4.0f / 32768.0f;
			break;
		case A_SCALE_8G:
			LSM_a_res = 8.0f / 32768.0f;
			break;
		default:
			LSM_a_res = 1.0f;
	}
}

void LSM_calc_m_res() {
	switch (LSM_m_scale) {
		case M_SCALE_4GS:
			LSM_m_res = 4.0f / 32768.0f;
			break;
		case M_SCALE_8GS:
			LSM_m_res = 8.0f / 32768.0f;
			break;
		case M_SCALE_12GS:
			LSM_m_res = 12.0f / 32768.0f;
			break;
		case M_SCALE_16GS:
			LSM_m_res = 16.0f / 32768.0f;
			break;
		default:
			LSM_m_res = 1.0f;
	}
}

int8_t LSM_read_reg_xlg(uint8_t reg_addr) {
	// Write the register we want to read
	// - Make a transmit buffer
	uint8_t tx_size = 1;
	uint8_t tx_buf[tx_size];
	// - Set the register address
	tx_buf[0] = reg_addr;
	// - Write the register value
	Chip_I2C_MasterSend(LSM_i2c_id, LSM_xlg_address, tx_buf, tx_size);

	// Read the register value
	// - Make a receive buffer
	uint8_t rx_size = 1;
	uint8_t rx_buf[rx_size];
	// - Read the register value
	Chip_I2C_MasterRead(LSM_i2c_id, LSM_xlg_address, rx_buf, rx_size);

	return rx_buf[0];
}

void LSM_write_reg_xlg(uint8_t reg_addr, uint8_t data) {
	// Write the register and then the data
	// - Make a transmit buffer
	uint8_t tx_size = 2;
	uint8_t tx_buf[tx_size];
	// - Set the register address
	tx_buf[0] = reg_addr;
	tx_buf[1] = data;
	// - Write the data
	Chip_I2C_MasterSend(LSM_i2c_id, LSM_xlg_address, tx_buf, tx_size);
}

int8_t LSM_read_reg_mag(uint8_t reg_addr) {
	// Write the register we want to read
	// - Make a transmit buffer
	uint8_t tx_size = 1;
	uint8_t tx_buf[tx_size];
	// - Set the register address
	tx_buf[0] = reg_addr;
	// - Write the register value
	Chip_I2C_MasterSend(LSM_i2c_id, LSM_mag_address, tx_buf, tx_size);

	// Read the register value
	// - Make a receive buffer
	uint8_t rx_size = 1;
	uint8_t rx_buf[rx_size];
	// - Read the register value
	Chip_I2C_MasterRead(LSM_i2c_id, LSM_mag_address, rx_buf, rx_size);

	return rx_buf[0];
}

void LSM_write_reg_mag(uint8_t reg_addr, uint8_t data) {
	// Write the register and then the data
	// - Make a transmit buffer
	uint8_t tx_size = 2;
	uint8_t tx_buf[tx_size];
	// - Set the register address
	tx_buf[0] = reg_addr;
	tx_buf[1] = data;
	// - Write the data
	Chip_I2C_MasterSend(LSM_i2c_id, LSM_mag_address, tx_buf, tx_size);
}
