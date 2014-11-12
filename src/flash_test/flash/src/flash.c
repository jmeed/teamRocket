/*
===============================================================================
 Name        : flash.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "chip.h"
#include "board.h"
#include "S25FL.h"
#include <cr_section_macros.h>


int main(void) {
	// Initialize stuff
    SystemCoreClockUpdate();
    Board_Init();
    S25FL_init(LPC_SSP0, S25FL_P_256, S25FL_E_64);

    // Write some data
    uint8_t write_data[8] = {0x77, 0x55, 0x33, 0x11, 0x00, 0x99, 0x88, 0x22};
    uint32_t address = 0x00000100;
    uint32_t write_length = 8;
    S25FL_write(address, (uint8_t*)write_data, write_length);

    // Pause - for some reason this is still needed, but shouldn't be
    volatile int j, dummy = 0;
    for (j = 0; j < 50000; ++j)
    	++dummy;

    // Read our new data
    uint8_t read_data[8];
    uint32_t read_length = 8;
    S25FL_read(address, (uint8_t*)read_data, read_length);

    // Print the data
    for (j = 0; j < read_length; ++j)
    	printf("%d ", read_data[j]);
    printf("\n");

    // Busy loop
    volatile static int i = 0 ;
    while(1) {
        i++ ;
    }
    return 0 ;
}
