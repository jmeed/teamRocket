#ifndef ROCKETIO_H
#define ROCKETIO_H

#include "gui.h"
#include <windows.h>
#include <fstream>

#define ALTITUDE_CODE	100000
#define TIME_CODE		100001
#define FREEFALL_CODE	100002

void write_config(HWND hwnd);
void write_flight(HWND hwnd);
void read_flight(HWND hwnd);

#endif /* ROCKETIO_H */
