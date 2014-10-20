
/*
********************************************************************
* File : initialize.c
* Description: The scope is to initialize Z8 Encore! XP® peripherals so 
* that it functions as a NiMH battery cell_control.
* This program initializes - 
* 								GPIO ports A, B, C
* 								Timer 0 and timer 1
* 								Internal Crystal oscillator
* 								ADC
* required for the cell_control function.
**********************************************************************
*/

#include <stdio.h>

#include <ez8.h>
#include "read_sensors.h"
#include "spi_routines.h"

extern void interrupt isr_timer0(void);
extern void interrupt isr_uart0_rx(void);

void initialize(void)
{
   unsigned int RAM_Location;	
   int i;							
   
// Initialize GPIO ports 
						            // ----------------------
   PAADDR = ALT_FUN;                // Port A alternate function -
   PACTL  = 0xb0;                   // All port A pins are LEDs, UART, or no-connect.  Updated to output timer1
   PAADDR = DATA_DIR;             	// Port A data direction -
  PACTL  = 0x50;                 	// set LEDs to be outputs (0)   
//   PACTL  = 0x59;                 	// set LEDs to be outputs (0)   
	PAADDR = 0x04; 				 	// High drive enable subregister
    PACTL  = 0x00;					// Normally C3
	PAADDR = 0x03; 				 	// Open-drain mode
   PACTL  = 0x00;					// Not open drain for any of the pins
   PAADDR = ALT_FUN;				// Leave in alt fun register mode for easier access
   PAOUT = 0x02;

			// ----------------------------------------------------------
									// Port B Initialization
   PBADDR = ALT_FUN;                // Port B alternate function -
   PBCTL  = 0x3F;                   // analogs
   PBADDR = AFS1;					// analog function set, 
   PBCTL  = 0x3F;					// 1 for each analog
									// High drive enable defaults low.
   PBADDR = DATA_DIR;             	// Port B data direction -
 	PBCTL  = 0xfF;					// All inputs or no-connect
	PBOUT = 0x00;					// Turn LEDs off initially.
   PBADDR = 0X00;
									// ----------------------------------------------------------
									// Port C Initialization
   PCADDR = ALT_FUN;                // Port C alternate function -
   PCCTL  = 0x71;                   // - Selecting Alternate Function for PC3
   PCADDR = AFS1;					//	
   PCCTL  = 0x71;					// Select LED Drive over Comparator o/p for PC3
   PCADDR = 0x03;					//	Open-drain enable register	
   PCCTL  = 0x70;					// Select LED Drive over Comparator o/p for PC3
   LEDEN  = 0x00;					// Enable LED Drive for PC3
   LEDLVLH = 0x10;                  // 
   LEDLVLL = 0x30;					// 7 mA current sink for PC3
   PCADDR = DATA_DIR;               // Port C data direction - 
   PCCTL  = 0x81;                   // .
										// Stop Mode Recovery Source not initialized for Port c
									// Pull Up not initialized for Port c
									// AFS2 not applicable for Port c
   PCOUT  = 0x38; 					// Data Output Register - LED OFF
   PCADDR = 0X00;

									// ----------------------------------------------------------
									// Initialize timer 0
//   SET_VECTOR(TIMER0, isr_timer0);  // Set interrupt service routine ISR for timer0 (Already in main)
//  SET_VECTOR(TIMER1, isr_timer1);  // Set interrupt service routine ISR for timer1
//	SET_VECTOR(UART0_RX, isr_uart0_rx);  // Set interrupt service routine ISR for UART
  T0H	= 0x00;		                // Timer High	
   T0L  = 0x00;		                // Timer Low

 // T0RH = 0x6a;	                    // Makes 200 Hz with prescale of 1
 //  T0RL = 0x72;	                    // 
	T0RH = 0x69;	                    // Makes 200 Hz with prescale of 1
	T0RL = 0x49;	                    // 
	T0CTL0 = 0x60;					// Timer control register 0
	T0CTL1 = 0x81;	                // Timer control regiester 1
   									// Timer Control#00010001b
					                // 				 | | ||_|_______ continuous mode of operation
                                    // 				 | |_|__________ divide by 4 Prescale
									//				 | enable
                                    // Set Timer0 as Level2 (Nominal) priority interrupt   
	T1H	= 0x00;		                // Timer High	
   	T1L  = 0x00;		            // Timer Low
//   T1RH = 0x0A;	                    // Reload reg Makes 1.5 ms with prescale of 16
//   T1RL = 0x62;	                    // Reload reg   
   T1RH = 0x07;	                    // Reload reg Makes 1.5 ms with prescale of 16
   T1RL = 0xcc;	                    // Reload reg   
   T1CTL0 = 0x00;					// Timer control register 0 
   T1CTL1 = 0x01;	                // Timer control register 1 Make polarity low except pulse.
   									// prescale 32

//IRQ0ENH |= 0x31;                 // IRQ0 Enable High for Timer1, Timer0, Uart0_rx and ADC, respectively.
IRQ0ENH |= 0x30;                 // no ADC
IRQ0ENL &= 0x00;	                // IRQ0 Enable Low

--------------------------------------------------  
									// Initialize Crystal Oscillator
//   OSCCTL  = OSC_UNLOCK_SEQ1;		// Unlock sequence for OSCTL write
//   OSCCTL  = OSC_UNLOCK_SEQ2;		//
//   OSCCTL  = 0xB8;					// Internal 5.5MHz, 
   									// external oscillator is disabled
									// IPO is enabled, WDT oscillator enabled
									// Failure detection and recovery of primary oscillator is enabled
									// Crystal or external RC oscillator as system clock
									// WDT failure detection enabled.
                                    // --------------------------------------------------  
									// ADC initialization
//	ADCCTL0 = 0x20;					// External Vref, Single Shot, Analog channel 0, not enabled
//	ADCCTL1 = 0x00;					// External Vref, Alarms disabled, unbuffered-single ended inputs 

//	ADCCTL0 = 0x20;					// Internal Vref, Single Shot, Analog channel 0, not enabled
//	ADCCTL1 = 0x80;					// Internal Vref 2.0volts, Alarms disabled, unbuffered 
									// Find Offset value for ADC

                                    // --------------------------------------------------                                      // Initialize UART for data collection, test only.
    U0BRH = 0x00;                   // Set Baudrate; test only.  
	U0BRL = 0x03;	                // Set Baudrate = 115200; test only. F_clk = 5.5296 MHz
//    U0BRL = 0x01;	                // Set Baudrate = 345600; test only. F_clk = 5.5296 MHz
    U0CTL0 = 0xC0;                  // Transmit enable, Receive Enable, 
                                    // No Parity, 1 Stop.
	U0CTL1 = 0x00;					// No multiprocessor bits, enable int on rcv data
					                // ----------------------------------------
}

// Initialize ADC compensation


/*
******************************************************************************
**  End of File
******************************************************************************
*/
