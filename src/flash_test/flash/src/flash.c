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
    SystemCoreClockUpdate();
    Board_Init();

    S25FL_init(LPC_SSP0, S25FL_P_256, S25FL_E_64);

    S25FL_tx_buf[0] = S25FL_WREN;
    spi_transceive(1, 0);

	volatile int j, dummy = 0;
	for (j = 0; j < S25FL_DUMMY_CYCLES; ++j)
		++dummy;

	S25FL_tx_buf[0] = S25FL_PP;
	S25FL_tx_buf[1] = 0x00;
	S25FL_tx_buf[2] = 0x01;
	S25FL_tx_buf[3] = 0x00;
	S25FL_tx_buf[4] = 0x77;
	S25FL_tx_buf[5] = 0x66;
	S25FL_tx_buf[6] = 0x55;
	S25FL_tx_buf[7] = 0x44;
	spi_transceive(8, 0);

	for (j = 0; j < S25FL_DUMMY_CYCLES; ++j)
		++dummy;

	S25FL_tx_buf[0] = S25FL_READ;
	S25FL_tx_buf[1] = 0x00;
	S25FL_tx_buf[2] = 0x01;
	S25FL_tx_buf[3] = 0x00;
	spi_transceive(4, 4);

	for (j = 0; j < S25FL_DUMMY_CYCLES; ++j)
			++dummy;

	for (j = 0; j < 4; ++j)
		printf("%d ", S25FL_rx_buf[j+4]);

//    uint8_t id = S25FL_read_register(S25FL_RDID);
//    printf("id = %d\n", id);

//    S25FL_tx_buf[0] = S25FL_RDID;
//    spi_transceive(1, 8);

//    spi_transceive(0, 8);
//    for (j = 0; j < 8; ++j)
//    	printf("%d ", S25FL_rx_buf[j]);

//    uint8_t write_data[8] = {0x77, 0x55, 0x33, 0x11, 0x00, 0x99, 0x88, 0x22};
//    uint32_t address = 0x00000100;
//    uint32_t write_length = 8;
//    S25FL_write(address, (uint8_t*)write_data, write_length);
//
//	volatile int j, dummy = 0;
//	for (j = 0; j < 50000; ++j)
//		++dummy;
//
//    uint8_t read_data[8];
//    uint32_t read_length = 8;
//    S25FL_read(address, (uint8_t*)read_data, read_length);
//
//    for (j = 0; j < read_length; ++j)
//    	printf("%d ", read_data[j]);
//    printf("\n");

    volatile static int i = 0 ;
    while(1) {
        i++ ;
    }
    return 0 ;
}
