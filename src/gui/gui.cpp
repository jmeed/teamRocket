#include "draw.h"
#include "rocketio.h"

static char const *AppTitle = TEXT("Team Rocket Flight Recorder/Controller");

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
	WNDCLASS wc;
	HWND hwnd;
	MSG msg;

	wc.style = CS_HREDRAW | CS_VREDRAW; 
	wc.lpfnWndProc = WindowProc; 
	wc.cbClsExtra = 0; 
	wc.cbWndExtra = 0; 
	wc.hInstance = hInst; 
	wc.hIcon = LoadIcon(NULL,IDI_WINLOGO); 
	wc.hCursor = LoadCursor(NULL,IDC_ARROW); 
	wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 230));
	wc.lpszMenuName = NULL; 
	wc.lpszClassName = AppTitle;

	if (!RegisterClass(&wc))
		return 0;

	hwnd = CreateWindow(AppTitle, AppTitle,
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, 760, 480,
		NULL, NULL, hInst, NULL);

	if (!hwnd)
		return 0;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 1;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	BOOL status;
	switch (msg) {
		case WM_CREATE: {
			draw_connected_wings(hwnd, lparam, 20, 20);
			draw_secondary_staging(hwnd, lparam, 200, 20);
			draw_previous_flight(hwnd, lparam, 20, 190);
			draw_set(hwnd, lparam, 580, 220);
			draw_save(hwnd, lparam, 580, 300);
			draw_flight_history(hwnd, lparam, 580, 380);
			break;
		} case WM_COMMAND: {
			//check_boxes(hwnd);
			switch(LOWORD(wparam)) {
				case H_BTN_SET: {
					write_config(hwnd);
					break;
				} case H_BTN_SAVE: {
					write_flight(hwnd);
					break;
				} case H_BTN_FLIGHT_HISTORY: {
					read_flight(hwnd);
					break;
				} case H_CK_SECONDARY_STAGING_WING: {
					status = IsDlgButtonChecked(hwnd, H_CK_SECONDARY_STAGING_WING);
					if (status) {
						CheckDlgButton(hwnd, H_CK_SECONDARY_STAGING_WING, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_SECONDARY_STAGING_WING, BST_CHECKED);
					}
					break;
				} case H_CK_RADIO_DOWNLINK_WING: {
					status = IsDlgButtonChecked(hwnd, H_CK_RADIO_DOWNLINK_WING);
					if (status) {
						CheckDlgButton(hwnd, H_CK_RADIO_DOWNLINK_WING, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_RADIO_DOWNLINK_WING, BST_CHECKED);
					}
					break;
				} case H_CK_GPS_LOCATOR_WING: {
					status = IsDlgButtonChecked(hwnd, H_CK_GPS_LOCATOR_WING);
					if (status) {
						CheckDlgButton(hwnd, H_CK_GPS_LOCATOR_WING, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_GPS_LOCATOR_WING, BST_CHECKED);
					}
					break;
				} case H_CK_CELLULAR_LOCATOR_WING: {
					status = IsDlgButtonChecked(hwnd, H_CK_CELLULAR_LOCATOR_WING);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CELLULAR_LOCATOR_WING, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CELLULAR_LOCATOR_WING, BST_CHECKED);
					}
					break;
				} case H_CK_CH0_ALTITUDE: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH0_ALTITUDE);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH0_ALTITUDE, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH0_ALTITUDE, BST_CHECKED);
					}
					break;
				} case H_CK_CH0_TIME: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH0_TIME);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH0_TIME, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH0_TIME, BST_CHECKED);
					}
					break;
				} case H_CK_CH0_FREEFALL: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH0_FREEFALL);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH0_FREEFALL, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH0_FREEFALL, BST_CHECKED);
					}
					break;
				} case H_CK_CH1_ALTITUDE: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH1_ALTITUDE);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH1_ALTITUDE, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH1_ALTITUDE, BST_CHECKED);
					}
					break;
				} case H_CK_CH1_TIME: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH1_TIME);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH1_TIME, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH1_TIME, BST_CHECKED);
					}
					break;
				} case H_CK_CH1_FREEFALL: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH1_FREEFALL);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH1_FREEFALL, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH1_FREEFALL, BST_CHECKED);
					}
					break;
				} case H_CK_CH2_ALTITUDE: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH2_ALTITUDE);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH2_ALTITUDE, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH2_ALTITUDE, BST_CHECKED);
					}
					break;
				} case H_CK_CH2_TIME: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH2_TIME);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH2_TIME, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH2_TIME, BST_CHECKED);
					}
					break;
				} case H_CK_CH2_FREEFALL: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH2_FREEFALL);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH2_FREEFALL, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH2_FREEFALL, BST_CHECKED);
					}
					break;
				} case H_CK_CH3_ALTITUDE: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH3_ALTITUDE);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH3_ALTITUDE, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH3_ALTITUDE, BST_CHECKED);
					}
					break;
				} case H_CK_CH3_TIME: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH3_TIME);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH3_TIME, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH3_TIME, BST_CHECKED);
					}
					break;
				} case H_CK_CH3_FREEFALL: {
					status = IsDlgButtonChecked(hwnd, H_CK_CH3_FREEFALL);
					if (status) {
						CheckDlgButton(hwnd, H_CK_CH3_FREEFALL, BST_UNCHECKED);
					}
					else {
						CheckDlgButton(hwnd, H_CK_CH3_FREEFALL, BST_CHECKED);
					}
					break;
				}
			}
			break;
		} case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		} default: {
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
	}

	return 0;
}
