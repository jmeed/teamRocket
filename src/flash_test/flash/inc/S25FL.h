#ifndef S25FL_H
#define S25FL_H

#include "spi.h"
#include "stdio.h"

// === Commands
// Read Device Identification
#define S25FL_READ_ID		0x90
#define S25FL_RDID 			0x9F
#define S25FL_RSFDP			0x5A
#define S25FL_RES			0xAB
// Register Access
#define S25FL_RDSR1			0x05
#define S25FL_RDSR2			0x07
#define S25FL_RDCR			0x35
#define S25FL_WRR			0x01
#define S25FL_WRDI			0x04
#define S25FL_WREN			0x06
#define S25FL_CLSR			0x30
#define S25FL_ABRD			0x14
#define S25FL_ABWR			0x15
#define S25FL_BRRD			0x16
#define S25FL_BRWR			0x17
#define S25FL_BRAC			0xB9
#define S25FL_DLPRD			0x41
#define S25FL_PNVDLR		0x43
#define S25FL_WVDLR			0x4A
// Read Flash Array
#define S25FL_READ 			0x03
#define S25FL_4READ			0x13
#define S25FL_FAST_READ		0x0B
#define S25FL_4FAST_READ	0x0C
#define S25FL_DOR 			0x3B
#define S25FL_4DOR			0x3C
#define S25FL_QOR			0x6B
#define S25FL_4QOR			0x6C
#define S25FL_DIOR			0xBB
#define S25FL_4DIOR			0xBC
#define S25FL_QIOR			0xEB
#define S25FL_4QIOR			0xEC
// Program Flash Array
#define S25FL_PP 			0x02
#define S25FL_4PP			0x12
#define S25FL_QPP			0x32
//#define S25FL_QPP			0x38
#define S25FL_4QPP			0x34
#define S25FL_PGSP			0x85
#define S25FL_PGRS			0x8A
// Erase Flash Array
#define S25FL_P4E			0x20
#define S25FL_4P4E			0x21
#define S25FL_BE			0x60
//#define S25FL_BE			0xC7
#define S25FL_SE 			0xD8
#define S25FL_4SE 			0xDC
#define S25FL_ERSP 			0x75
#define S25FL_ERRS			0x7A
// One Time Program Array
#define S25FL_OTPP 			0x42
#define S25FL_OTPR			0x48
// Advanced Sector Protection
#define S25FL_DYBRD			0xE0
#define S25FL_DYBWR			0xE1
#define S25FL_PPBRD			0xE2
#define S25FL_PPBP			0xE3
#define S25FL_PPBE			0xE4
#define S25FL_ASPRD			0x2B
#define S25FL_ASPP			0x2F
#define S25FL_PLBRD			0xA7
#define S25FL_PLBWR			0xA6
#define S25FL_PASSRD		0xE7
#define S25FL_PASSP			0xE8
#define S25FL_PASSU			0xE9
// Reset
#define S25FL_RESET			0xF0
#define S25FL_MBR			0xFF

#define S25FL_DUMMY_CYCLES	5000	// mentioned, but # not defined by spec

// === SPI
#define S25FL_SS_PORT	0
#define S25FL_SS_PIN	2
LPC_SSP_T* S25FL_ssp_id;
uint8_t* S25FL_rx_buf;
uint8_t* S25FL_tx_buf;
uint32_t S25FL_rx_len;	// number of expected return bytes
uint32_t S25FL_tx_len;	// number of command+address bytes

// === Configuration types
enum Page_Size {
	S25FL_P_256,	// 0x00
	S25FL_P_512		// 0x01
};
enum Page_Size S25FL_p_size;
enum Erase_Size {
	S25FL_E_64,		// 0x00
	S25FL_E_256		// 0x01
};
enum Erase_Size S25FL_e_size;

// =======================================================================
// === Configuration & Status functions
// =======================================================================

// Initializes the device and SPI channel
void S25FL_init(LPC_SSP_T* id_in, enum Page_Size p_sz_in, enum Erase_Size e_sz_in);

// Writes new values to the status registers and configuration register
void S25FL_write_registers(uint8_t status_1, uint8_t config, uint8_t status_2);

// Reads the contents of specified register
uint8_t S25FL_read_register(uint8_t reg);

// Set page size to either 256 bytes or 512 bytes
void S25FL_set_page_size(enum Page_Size page_size);

// Set block erase size to either 64 kB or 256 kB
void S25FL_set_erase_size(enum Erase_Size erase_size);

// Restores device to initial power up state, except for folatile FREEZE
// bit and the PPB Lock bit
void S25FL_reset();

// =======================================================================
// === Write functions
// =======================================================================

// Set slave select (active low)
// Not a user function
void S25FL_ss_set();

// Clear slave select (active low)
// Not a user function
void S25FL_ss_clear();

// Write the buffer array to flash, starting at address (3-bytes), for 
// length bytes
// Writes will wrap around in memory
// This is a user function
void S25FL_write(uint32_t address, uint8_t* buffer, uint32_t length);

// Before writing more data, the Work In Progress bit in Read Status
// Register 1 must be checked
// This function loops until WIP bit is cleared
// The user will not have to call this themselves
void S25FL_write_wait();

// The write enable instruction must be sent every time before any write,
// page program, or erase command
// The user will not have to call this themselves
void S25FL_write_enable();

// This function disables writing, blocking any write, page program, or
// erase command
void S25FL_write_disable();

// Suspend any current programming operations
// While suspended, commands RDSR1 and RDSR2 are allowed
// void S25FL_write_suspend();

// Resume any current programming operations
// void S25FL_write_resume();

// =======================================================================
// === Read functions
// =======================================================================

// Read flash data from address into buffer, for length bytes
// This is a user function
void S25FL_read(uint32_t address, uint8_t* buffer, uint32_t length);

// =======================================================================
// === Erase functions
// =======================================================================

// Erases (sets all bits to 1) a 4-kB sector at address (3-bytes)
// This function only works if sectors are configured to 64-kB
void S25FL_erase_4k(uint32_t address);

// Erases (sets all bits to 1) a 64-kB or 256-kB sector (depends on 
// configuration) at address (3-bytes)
void S25FL_erase_sector(uint32_t address);

// Erases (sets all bits to 1) the entire flash memory array
void S25FL_erase_bulk();

// Suspend any current erase operations
// While suspended, commands RDSR1 and RDSR2 are allowed
// void S25FL_erase_suspend();

// Resume any current erase operations
// void S25FL_erase_resume();

#endif /* S25FL_H */
