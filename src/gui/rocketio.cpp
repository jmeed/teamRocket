#include "rocketio.h"

using namespace std;

void write_config(HWND hwnd) {
	BOOL status;
	LPTSTR buf = new char[16];
	unsigned maxRead = 16;
	int len;
	// Open File
	ofstream configFile;
	configFile.open("config.txt");
	// Connected Wings
	status = IsDlgButtonChecked(hwnd, H_CK_SECONDARY_STAGING_WING);
	configFile << status << "\n";
	status = IsDlgButtonChecked(hwnd, H_CK_RADIO_DOWNLINK_WING);
	configFile << status << "\n";
	status = IsDlgButtonChecked(hwnd, H_CK_GPS_LOCATOR_WING);
	configFile << status << "\n";
	status = IsDlgButtonChecked(hwnd, H_CK_CELLULAR_LOCATOR_WING);
	configFile << status << "\n";
	// Secondary Staging
	// === Channel 0
	GetWindowText(GetDlgItem(hwnd, H_TXT_CH0_ALTITUDE), buf, maxRead);
	status = IsDlgButtonChecked(hwnd, H_CK_CH0_ALTITUDE);
	if (status) {
		len = strlen(buf);
		for (int i = 0; i < len; ++i)
			configFile << buf[i];
		configFile << "\n" << "a" << "\n"; 
	}
	status = IsDlgButtonChecked(hwnd, H_CK_CH0_TIME);
	if (status) {
		len = strlen(buf);
		for (int i = 0; i < len; ++i)
			configFile << buf[i];
		configFile << "\n" << "t" << "\n";
	}
	status = IsDlgButtonChecked(hwnd, H_CK_CH0_FREEFALL);
	if (status) {
		configFile << "0" << "\n";
		configFile << "f" << "\n";
	}
	// === Channel 1
	GetWindowText(GetDlgItem(hwnd, H_TXT_CH1_ALTITUDE), buf, maxRead);
	status = IsDlgButtonChecked(hwnd, H_CK_CH1_ALTITUDE);
	if (status) {
		len = strlen(buf);
		for (int i = 0; i < len; ++i)
			configFile << buf[i];
		configFile << "\n" << "a" << "\n"; 
	}
	status = IsDlgButtonChecked(hwnd, H_CK_CH1_TIME);
	if (status) {
		len = strlen(buf);
		for (int i = 0; i < len; ++i)
			configFile << buf[i];
		configFile << "\n" << "t" << "\n";
	}
	status = IsDlgButtonChecked(hwnd, H_CK_CH1_FREEFALL);
	if (status) {
		configFile << "0" << "\n";
		configFile << "f" << "\n";
	}
	// === Channel 2
	GetWindowText(GetDlgItem(hwnd, H_TXT_CH2_ALTITUDE), buf, maxRead);
	status = IsDlgButtonChecked(hwnd, H_CK_CH2_ALTITUDE);
	if (status) {
		len = strlen(buf);
		for (int i = 0; i < len; ++i)
			configFile << buf[i];
		configFile << "\n" << "a" << "\n"; 
	}
	status = IsDlgButtonChecked(hwnd, H_CK_CH2_TIME);
	if (status) {
		len = strlen(buf);
		for (int i = 0; i < len; ++i)
			configFile << buf[i];
		configFile << "\n" << "t" << "\n";
	}
	status = IsDlgButtonChecked(hwnd, H_CK_CH2_FREEFALL);
	if (status) {
		configFile << "0" << "\n";
		configFile << "f" << "\n";
	}
	// === Channel 3
	GetWindowText(GetDlgItem(hwnd, H_TXT_CH3_ALTITUDE), buf, maxRead);
	status = IsDlgButtonChecked(hwnd, H_CK_CH3_ALTITUDE);
	if (status) {
		len = strlen(buf);
		for (int i = 0; i < len; ++i)
			configFile << buf[i];
		configFile << "\n" << "a" << "\n"; 
	}
	status = IsDlgButtonChecked(hwnd, H_CK_CH3_TIME);
	if (status) {
		len = strlen(buf);
		for (int i = 0; i < len; ++i)
			configFile << buf[i];
		configFile << "\n" << "t" << "\n";
	}
	status = IsDlgButtonChecked(hwnd, H_CK_CH3_FREEFALL);
	if (status) {
		configFile << "0" << "\n";
		configFile << "f" << "\n";
	}

	configFile.close();
}

void write_flight(HWND hwnd) {

}

void read_flight(HWND hwnd) {

}
