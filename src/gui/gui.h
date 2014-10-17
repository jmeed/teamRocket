#ifndef GUI_H
#define GUI_H

#include <windows.h>

#define H_STATIC	99

// Connected Wings
#define H_GBOX_WINGS				100
#define H_CK_SECONDARY_STAGING_WING	1000
#define H_CK_RADIO_DOWNLINK_WING	1001
#define H_CK_GPS_LOCATOR_WING		1002
#define H_CK_CELLULAR_LOCATOR_WING	1003

// Secondary Staging Configuration
#define H_GBOX_SECONDARY_STAGING	101
// === Channel 0
#define H_CK_CH0_ALTITUDE			1004
#define H_CK_CH0_TIME				10000
#define H_CK_CH0_FREEFALL			1005
#define H_TXT_CH0_ALTITUDE 			1006
// === Channel 1
#define H_CK_CH1_ALTITUDE 			1007
#define H_CK_CH1_TIME				10001
#define H_CK_CH1_FREEFALL 			1008
#define H_TXT_CH1_ALTITUDE  		1009
// === Channel 2
#define H_CK_CH2_ALTITUDE 			1010
#define H_CK_CH2_TIME				10002
#define H_CK_CH2_FREEFALL			1011
#define H_TXT_CH2_ALTITUDE 			1012
// === Channel 3
#define H_CK_CH3_ALTITUDE 			1013
#define H_CK_CH3_TIME				10003
#define H_CK_CH3_FREEFALL 			1014
#define H_TXT_CH3_ALTITUDE 			1015

// Set Configuration
#define H_GBOX_SET 	102
#define H_BTN_SET	1016

// Previous Flight Display
#define H_GBOX_FLIGHT_DISPLAY	103
//#define H_GBOX_FLIGHT_HISTORY	104
#define H_GBOX_SAVE				105
// === Altitude
#define H_TXT_LAUNCH_ALTITUDE 	1017
#define H_TXT_MAX_ALTITUDE		1018
#define H_TXT_DELTA_ALTITUDE	1019
// === Acceleration
#define H_TXT_MAX_ACCELERATION	1020
#define H_TXT_AVG_ACCELERATION	1021
// === Spin rate
#define H_TXT_MAX_SPIN_RATE	1022
#define H_TXT_AVG_SPIN_RATE	1023
// === Channel 0
#define H_TXT_CH0_DEPLOYED_STATUS	1024
#define H_TXT_CH0_DEPLOYED_ALTITUDE	1025
// === Channel 1
#define H_TXT_CH1_DEPLOYED_STATUS	1026
#define H_TXT_CH1_DEPLOYED_ALTITUDE	1027
// === Channel 2
#define H_TXT_CH2_DEPLOYED_STATUS	1028
#define H_TXT_CH2_DEPLOYED_ALTITUDE	1029
// === Channel 3
#define H_TXT_CH3_DEPLOYED_STATUS	1030
#define H_TXT_CH3_DEPLOYED_ALTITUDE	1031
// === Other Flight History
#define H_BTN_FLIGHT_HISTORY	1032
// === Save
#define H_BTN_SAVE	1033

#endif /* GUI_H */
