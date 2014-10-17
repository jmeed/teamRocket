#ifndef DRAW_H
#define DRAW_H

#include "gui.h"
#include <windows.h>

#define CK_X_LEN	150
#define CK_Y_LEN	20
#define BTN_X_LEN	150
#define BTN_Y_LEN	35

void draw_connected_wings(HWND hwnd, LPARAM lparam, int x_start, int y_start);
void draw_secondary_staging(HWND hwnd, LPARAM lparam, int x_start, int y_start);
void draw_previous_flight(HWND hwnd, LPARAM lparam, int x_start, int y_start);
void draw_set(HWND hwnd, LPARAM lparam, int x_start, int y_start);
void draw_save(HWND hwnd, LPARAM lparam, int x_start, int y_start);
void draw_flight_history(HWND hwnd, LPARAM lparam, int x_start, int y_start);
void draw_static(HWND hwnd, LPARAM lparam, char const* text, int x_start, int y_start, int x_length, int y_length, int handle);
void draw_groupbox(HWND hwnd, LPARAM lparam, char const* text, int x_start, int y_start, int x_length, int y_length, int handle);
void draw_checkbox(HWND hwnd, LPARAM lparam, char const* text, int x_start, int y_start, int x_length, int y_length, int handle);
void draw_pushbutton(HWND hwnd, LPARAM lparam, char const* text, int x_start, int y_start, int x_length, int y_length, int handle);
void draw_edit(HWND hwnd, LPARAM lparam, unsigned style, int x_start, int y_start, int x_length, int y_length, int handle);

#endif /* DRAW_H */
