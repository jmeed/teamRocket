#include "draw.h"

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
	wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 230));//(HBRUSH)COLOR_WINDOWFRAME; 
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
			switch(LOWORD(wparam)) {
				/*case H_BTN_SET: {
					SetWindowText(hwnd, TEXT("You pressed SET"));
					break;
				} case H_BTN_SAVE: {
					SetWindowText(hwnd, TEXT("You pressed SAVE"));
					break;
				}*/
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
