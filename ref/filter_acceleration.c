uint8_t g_count = 0;
uint8_t g_hist_len = 0;
float gx_hist[128];
float gy_hist[128];
float gz_hist[128];
static void vIMU(void* pvParameters) {
	static FIL f_imu_log;
	static char imu_str_buf[0x40];
	static float ax_hist[5];
	static float ay_hist[5];
	static float az_hist[5];

	LOG_INFO("Initializing IMU");
	xSemaphoreTake(mutex_i2c, portMAX_DELAY);
	LSM_init(I2C0, G_SCALE_245DPS, A_SCALE_8G, M_SCALE_4GS, G_ODR_952, A_ODR_952, M_ODR_80);
	xSemaphoreGive(mutex_i2c);
	LOG_INFO("IMU initialized");

	int result;
	int counter = 0;
	strcpy(imu_str_buf, "IMU.TAB");
	{
		int rename_number = 1;
		while(true) {
			if (f_stat(imu_str_buf, NULL) == FR_OK) {
				sprintf(imu_str_buf, "IMU%d.TAB", rename_number);
				rename_number ++;
				continue;
			}
			break;
		}
	}
	LOG_INFO("Imu output is %s", imu_str_buf);
	result = f_open(&f_imu_log, imu_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
	for (;;) {
		float ax, ay, az, gx, gy, gz, mx, my, mz;
		xSemaphoreTake(mutex_i2c, portMAX_DELAY);
		ax = LSM_read_accel_g(LSM_ACCEL_X);
		ay = LSM_read_accel_g(LSM_ACCEL_Y);
		az = LSM_read_accel_g(LSM_ACCEL_Z);
		gx = LSM_read_accel_g(LSM_GYRO_X);
		gy = LSM_read_accel_g(LSM_GYRO_Y);
		gz = LSM_read_accel_g(LSM_GYRO_Z);
		mx = LSM_read_accel_g(LSM_MAG_X);
		my = LSM_read_accel_g(LSM_MAG_Y);
		mz = LSM_read_accel_g(LSM_MAG_Z);
		xSemaphoreGive(mutex_i2c);

		// Remove faulty 0 values
		filter_acceleration(&ax, &ay, &az, ax_hist, ay_hist, az_hist);
		// Store gyro values for XBee's use
		gx_hist[g_count] = gx;
		gy_hist[g_count] = gy;
		gz_hist[g_count] = gz;
		++g_count;

		if (result == FR_OK) {
			sprintf(imu_str_buf, "%d\t%f\t%f\t%f\t%f\t", xTaskGetTickCount(), ax, ay, az, gx, gy);
			f_puts(imu_str_buf, &f_imu_log);
			sprintf(imu_str_buf, "%f\t%f\t%f\t%f\t%f\n", gz, mx, my, mz);
			f_puts(imu_str_buf, &f_imu_log);
			if ((counter % 50) == 0) {
				f_sync(&f_imu_log);
			}
		}

		vTaskDelay(50);
		counter += 1;
	}
}

void filter_acceleration(float *ax, float *ay, float *az, float *ax_hist, float *ay_hist, float *az_hist) {
	float noise_threshold = 0.08;
	int i;
	int buf_size = 5;

	// If x-accel is below threshold, look for recent value
	if (*ax < noise_threshold && *ax > -noise_threshold) {
		for (i = 0; i < buf_size; ++i) {
			if (ax_hist[i] > noise_threshold || ax_hist[i] < -noise_threshold) {
				*ax = ax_hist[i];
				break;
			}
		}
	}
	// If y-accel is below threshold, look for recent value
	if (*ay < noise_threshold && *ay > -noise_threshold) {
		for (i = 0; i < buf_size; ++i) {
			if (ay_hist[i] > noise_threshold || ay_hist[i] < -noise_threshold) {
				*ay = ay_hist[i];
				break;
			}
		}
	}
	// If z-accel is below threshold, look for recent value
	if (*az < noise_threshold && *az > -noise_threshold) {
		for (i = 0; i < buf_size; ++i) {
			if (az_hist[i] > noise_threshold || az_hist[i] < -noise_threshold) {
				*az = az_hist[i];
				break;
			}
		}
	}

	// Shift the buffer back
	for (i = buf_size - 1; i >= 0; --i) {
		ax_hist[i] = ax_hist[i-1];
		ay_hist[i] = ay_hist[i-1];
		az_hist[i] = az_hist[i-1];
	}
	// Place new values
	ax_hist[0] = *ax;
	ay_hist[0] = *ay;
	az_hist[0] = *az;
}
