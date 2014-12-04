#ifndef VOLATILE_FLIGHT_DATA_H
#define VOLATILE_FLIGHT_DATA_H

/*****************************************************************************
 * Global variables for flight record
 ****************************************************************************/

float max_spd;
float max_alt;
float max_acc;
float descent_rate;

float alt_arr[5];
float time_arr[5];

extern static bool volt_active;
extern static bool gps_activated;
extern static bool baro_running;
extern static bool imu_running;
extern static bool highg_running;

#endif /* VOLATILE_FLIGHT_DATA_H */