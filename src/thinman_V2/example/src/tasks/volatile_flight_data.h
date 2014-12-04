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

#endif /* VOLATILE_FLIGHT_DATA_H */