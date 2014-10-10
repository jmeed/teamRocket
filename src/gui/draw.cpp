#include "draw.h"

void draw_connected_wings(HWND hwnd, LPARAM lparam, int x_start, int y_start) {
	int grp_x_len = 170;
	int grp_y_len = 160;
	int x_off = 10;
	int y_off = 30;
	draw_groupbox(hwnd, lparam, "Connected Wings", x_start, y_start, grp_x_len, grp_y_len, H_GBOX_WINGS);
	draw_checkbox(hwnd, lparam, "Secondary Staging", x_start+x_off, y_start+y_off*1, CK_X_LEN, CK_Y_LEN, H_CK_SECONDARY_STAGING_WING);
	draw_checkbox(hwnd, lparam, "Radio", x_start+x_off, y_start+y_off*2, CK_X_LEN, CK_Y_LEN, H_CK_RADIO_DOWNLINK_WING);
	draw_checkbox(hwnd, lparam, "GPS", x_start+x_off, y_start+y_off*3, CK_X_LEN, CK_Y_LEN, H_CK_GPS_LOCATOR_WING);
	draw_checkbox(hwnd, lparam, "Cellular", x_start+x_off, y_start+y_off*4, CK_X_LEN, CK_Y_LEN, H_CK_CELLULAR_LOCATOR_WING);
}

void draw_secondary_staging(HWND hwnd, LPARAM lparam, int x_start, int y_start) {
	int grp_x_len = 460;
	int grp_y_len = 160;
	int x_off = 10;
	int y_off = 30;
	int stc_x_len = 70;
	int stc_y_len = 20;
	int edt_x_len = 60;
	int edt_y_len = 20;
	draw_groupbox(hwnd, lparam, "Secondary Staging", x_start, y_start, grp_x_len, grp_y_len, H_GBOX_SECONDARY_STAGING);
	// Channel 0
	draw_static(hwnd, lparam, "Channel 0:", x_start+x_off, y_start+y_off*1, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL, x_start+90, y_start+y_off*1, edt_x_len, edt_y_len, H_TXT_CH0_ALTITUDE);
	//draw_static(hwnd, lparam, "m", x_start+160, y_start+y_off*1, 15, stc_y_len, H_STATIC);
	draw_checkbox(hwnd, lparam, "Altitude (m)", x_start+160, y_start+y_off*1, 90, CK_Y_LEN, H_CK_CH0_ALTITUDE);
	draw_checkbox(hwnd, lparam, "Time (s)", x_start+260, y_start+y_off*1, 90, CK_Y_LEN, H_CK_CH0_TIME);
	draw_checkbox(hwnd, lparam, "Freefall", x_start+360, y_start+y_off*1, 90, CK_Y_LEN, H_CK_CH0_FREEFALL);
	// Channel 1
	draw_static(hwnd, lparam, "Channel 1:", x_start+x_off, y_start+y_off*2, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL, x_start+90, y_start+y_off*2, edt_x_len, edt_y_len, H_TXT_CH1_ALTITUDE);
	//draw_static(hwnd, lparam, "m", x_start+160, y_start+y_off*2, 15, stc_y_len, H_STATIC);
	draw_checkbox(hwnd, lparam, "Altitude (m)", x_start+160, y_start+y_off*2, 90, CK_Y_LEN, H_CK_CH1_ALTITUDE);
	draw_checkbox(hwnd, lparam, "Time (s)", x_start+260, y_start+y_off*2, 90, CK_Y_LEN, H_CK_CH1_TIME);
	draw_checkbox(hwnd, lparam, "Freefall", x_start+360, y_start+y_off*2, 90, CK_Y_LEN, H_CK_CH1_FREEFALL);
	// Channel 2
	draw_static(hwnd, lparam, "Channel 2:", x_start+x_off, y_start+y_off*3, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL, x_start+90, y_start+y_off*3, edt_x_len, edt_y_len, H_TXT_CH2_ALTITUDE);
	//draw_static(hwnd, lparam, "m", x_start+160, y_start+y_off*3, 15, stc_y_len, H_STATIC);
	draw_checkbox(hwnd, lparam, "Altitude (m)", x_start+160, y_start+y_off*3, 90, CK_Y_LEN, H_CK_CH2_ALTITUDE);
	draw_checkbox(hwnd, lparam, "Time (s)", x_start+260, y_start+y_off*3, 90, CK_Y_LEN, H_CK_CH2_TIME);
	draw_checkbox(hwnd, lparam, "Freefall", x_start+360, y_start+y_off*3, 90, CK_Y_LEN, H_CK_CH2_FREEFALL);
	// Channel 3
	draw_static(hwnd, lparam, "Channel 3:", x_start+x_off, y_start+y_off*4, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL, x_start+90, y_start+y_off*4, edt_x_len, edt_y_len, H_TXT_CH3_ALTITUDE);
	//draw_static(hwnd, lparam, "m", x_start+160, y_start+y_off*4, 15, stc_y_len, H_STATIC);
	draw_checkbox(hwnd, lparam, "Altitude (m)", x_start+160, y_start+y_off*4, 90, CK_Y_LEN, H_CK_CH3_ALTITUDE);
	draw_checkbox(hwnd, lparam, "Time (s)", x_start+260, y_start+y_off*4, 90, CK_Y_LEN, H_CK_CH3_TIME);
	draw_checkbox(hwnd, lparam, "Freefall", x_start+360, y_start+y_off*4, 90, CK_Y_LEN, H_CK_CH3_FREEFALL);
}

void draw_previous_flight(HWND hwnd, LPARAM lparam, int x_start, int y_start) {
	int grp_x_len = 530;
	int grp_y_len = 250;
	int x_off = 10;
	int y_off = 30;
	int stc_x_len = 130;
	int stc_y_len = 20;
	int edt_x_len = 60;
	int edt_y_len = 20;
	draw_groupbox(hwnd, lparam, "Previous Flight", x_start, y_start, grp_x_len, grp_y_len, H_GBOX_FLIGHT_DISPLAY);
	// Altitude
	draw_static(hwnd, lparam, "Launch Altitude:", x_start+x_off, y_start+y_off*1, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL | ES_READONLY, x_start+150, y_start+y_off*1, edt_x_len, edt_y_len, H_TXT_LAUNCH_ALTITUDE);
	draw_static(hwnd, lparam, "MAX Altitude:", x_start+x_off, y_start+y_off*2, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL | ES_READONLY, x_start+150, y_start+y_off*2, edt_x_len, edt_y_len, H_TXT_MAX_ALTITUDE);
	draw_static(hwnd, lparam, "Delta Altitude:", x_start+x_off, y_start+y_off*3, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL | ES_READONLY, x_start+150, y_start+y_off*3, edt_x_len, edt_y_len, H_TXT_DELTA_ALTITUDE);
	// Acceleration
	draw_static(hwnd, lparam, "Max Acceleration", x_start+x_off, y_start+y_off*4, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL | ES_READONLY, x_start+150, y_start+y_off*4, edt_x_len, edt_y_len, H_TXT_MAX_ACCELERATION);
	draw_static(hwnd, lparam, "Avg Acceleration", x_start+x_off, y_start+y_off*5, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL | ES_READONLY, x_start+150, y_start+y_off*5, edt_x_len, edt_y_len, H_TXT_AVG_ACCELERATION);
	// Spin rate
	draw_static(hwnd, lparam, "Max Spin Rate", x_start+x_off, y_start+y_off*6, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL | ES_READONLY, x_start+150, y_start+y_off*6, edt_x_len, edt_y_len, H_TXT_MAX_SPIN_RATE);
	draw_static(hwnd, lparam, "Max Spin Rate", x_start+x_off, y_start+y_off*7, stc_x_len, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL | ES_READONLY, x_start+150, y_start+y_off*7, edt_x_len, edt_y_len, H_TXT_AVG_SPIN_RATE);
	// Channel 0 deployment
	draw_static(hwnd, lparam, "Channel 0:", x_start+230, y_start+y_off*1, 70, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, x_start+310, y_start+y_off*1, 100, edt_y_len, H_TXT_CH0_DEPLOYED_STATUS);
	draw_static(hwnd, lparam, "at", x_start+420, y_start+y_off*1, 15, edt_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, x_start+445, y_start+y_off*1, edt_x_len, edt_y_len, H_TXT_CH0_DEPLOYED_ALTITUDE);
	draw_static(hwnd, lparam, "m", x_start+505, y_start+y_off*1, 15, edt_y_len, H_STATIC);
	// Channel 1 deployment
	draw_static(hwnd, lparam, "Channel 1:", x_start+230, y_start+y_off*2, 70, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, x_start+310, y_start+y_off*2, 100, edt_y_len, H_TXT_CH1_DEPLOYED_STATUS);
	draw_static(hwnd, lparam, "at", x_start+420, y_start+y_off*2, 15, edt_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, x_start+445, y_start+y_off*2, edt_x_len, edt_y_len, H_TXT_CH1_DEPLOYED_ALTITUDE);
	draw_static(hwnd, lparam, "m", x_start+505, y_start+y_off*2, 15, edt_y_len, H_STATIC);
	// Channel 2 deployment
	draw_static(hwnd, lparam, "Channel 2:", x_start+230, y_start+y_off*3, 70, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, x_start+310, y_start+y_off*3, 100, edt_y_len, H_TXT_CH2_DEPLOYED_STATUS);
	draw_static(hwnd, lparam, "at", x_start+420, y_start+y_off*3, 15, edt_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, x_start+445, y_start+y_off*3, edt_x_len, edt_y_len, H_TXT_CH2_DEPLOYED_ALTITUDE);
	draw_static(hwnd, lparam, "m", x_start+505, y_start+y_off*3, 15, edt_y_len, H_STATIC);
	// Channel 3 deployment
	draw_static(hwnd, lparam, "Channel 3:", x_start+230, y_start+y_off*4, 70, stc_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, x_start+310, y_start+y_off*4, 100, edt_y_len, H_TXT_CH3_DEPLOYED_STATUS);
	draw_static(hwnd, lparam, "at", x_start+420, y_start+y_off*4, 15, edt_y_len, H_STATIC);
	draw_edit(hwnd, lparam, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY, x_start+445, y_start+y_off*4, edt_x_len, edt_y_len, H_TXT_CH3_DEPLOYED_ALTITUDE);
	draw_static(hwnd, lparam, "m", x_start+505, y_start+y_off*4, 15, edt_y_len, H_STATIC);
}

void draw_set(HWND hwnd, LPARAM lparam, int x_start, int y_start) {
	draw_pushbutton(hwnd, lparam, "SET", x_start, y_start, BTN_X_LEN, BTN_Y_LEN, H_BTN_SET);
}

void draw_save(HWND hwnd, LPARAM lparam, int x_start, int y_start) {
	draw_pushbutton(hwnd, lparam, "SAVE", x_start, y_start, BTN_X_LEN, BTN_Y_LEN, H_BTN_SAVE);
}

void draw_flight_history(HWND hwnd, LPARAM lparam, int x_start, int y_start) {
	draw_pushbutton(hwnd, lparam, "FLIGHT HISTORY", x_start, y_start, BTN_X_LEN, BTN_Y_LEN, H_BTN_FLIGHT_HISTORY);
}

void draw_static(HWND hwnd, LPARAM lparam, char const* text, int x_start, int y_start, int x_length, int y_length, int handle) {
	CreateWindow(
		TEXT("static"), TEXT(text),
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		x_start, y_start, x_length, y_length,
		hwnd, (HMENU)handle, ((LPCREATESTRUCT)lparam)->hInstance, NULL);
}

void draw_groupbox(HWND hwnd, LPARAM lparam, char const* text, int x_start, int y_start, int x_length, int y_length, int handle) {
	CreateWindow(
		TEXT("button"), TEXT(text),
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		x_start, y_start, x_length, y_length,
		hwnd, (HMENU)handle, ((LPCREATESTRUCT)lparam)->hInstance, NULL);
}

void draw_checkbox(HWND hwnd, LPARAM lparam, char const* text, int x_start, int y_start, int x_length, int y_length, int handle) {
	CreateWindow(
		TEXT("button"), TEXT(text),
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		x_start, y_start, x_length, y_length,
		hwnd, (HMENU)handle, ((LPCREATESTRUCT)lparam)->hInstance, NULL);
}

void draw_pushbutton(HWND hwnd, LPARAM lparam, char const* text, int x_start, int y_start, int x_length, int y_length, int handle) {
	CreateWindow(
		TEXT("button"), TEXT(text),
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		x_start, y_start, x_length, y_length,
		hwnd, (HMENU)handle, ((LPCREATESTRUCT)lparam)->hInstance, NULL);
}

void draw_edit(HWND hwnd, LPARAM lparam, unsigned style, int x_start, int y_start, int x_length, int y_length, int handle) {
	CreateWindow(
		TEXT("edit"), TEXT(""),
		style,
		x_start, y_start, x_length, y_length,
		hwnd, (HMENU)handle, ((LPCREATESTRUCT)lparam)->hInstance, NULL);
}