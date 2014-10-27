
/*
********************************************************************
* File : main.c
**********************************************************************
*/
// Rev log, Oct 16
/*
-Made writing previous buffer to flash a function
-Called old buffer write to flash upon detecting liftoff
-Added extra byte shift to skip past calibration index for cal download
-Made liftoff flight_mode flag and prelaunch flight_mode flag higher than other flight_modes to simplify conditional for buffer write

-Nov 6 07	Fix Bug:  Stopped writing to buffer after liftoff detect, until next buffer swap.  Fixed by adding "Start Buffer Write"
to manage buffer swap and to liftoff detect.
-Feb 10	08:  Modified to look for an extra byte before the calibration coefficients so that they will line up with int sizes
- 5/1  deployed_flag reset to zero whenever in prelaunch flight_mode.  last_press initially set to 1.5 atm to prevent premature sep
5/8-11:  Cleaned up A/D register data grab, and SPI bit shifting.  Replaced printf with itoa and putch calls.
Added pressure-to-altitude function, but had to comment it out due to space.
Added full deployment controller.
Added flight event register.
6/5  Changed location of deployment program to page 3.
8/15/08  Deleted turning on beeper at end of recording.
11/10/08  Changed the continuity check to unbuffered mode.
*/

#include <stdio.h>
#include <ez8.h>
//#include <math.h>
#include "spi_routines.h"
#include "main.h"
#define FLIGHT_START 		256		//Starting address
#define FLIGHT_PAGES 		768		//Pages in 3 sectors
#define HR_PAGES	 		550		//Number of pages to use for high-rate portion
#define WAIT 				0x0A
#define PRELAUNCH 			0x0F
#define LIFTOFF 			0x0E
#define BEEP 				0x0B
#define DOWNLOAD 			0x0D

void interrupt void isr_timer0(void);
void interrupt void isr_uart0_rx(void);
//void interrupt void isr_ADC(void);
void Download_Data(void);
void Switch_Beep(int);
void initialize(void);
//int Calc_Altitude(int);
void Print_Line(void);
void Erase_Flight(void);
void Wait_For_Flash(void);
char Flight_Is_Full(void);
void Prog_Other_Buffer(void);
void Start_Buffer_Write(void);
char* itoa(int val);
void putstring(char*);
int Calc_Altitude(long);


/* Global Variables */
unsigned int  major_cycle = 0, seconds = 0, msec = 0, new_step=1;// margin_time= 0, save_margin_time=0;
unsigned int t_p_vals[3] = {1,150,250}, liftoff_press, old_liftoff_press, filtered_press, filtered_press_times4;
signed long velocity = 0;
char port_mask[3] = {4,8,1};  //Apogee,  Main, 3rd
unsigned char output_start_cycle [3] ={255,255,255},  full_cycle;  //Not-fired initial condition

unsigned int FER = 0, output_mask = 0; /* Flight Event mask.  16 bits with reserved bits so that ASCII conversion is typable.*/
unsigned char program_data[12], enable_program;
signed int ADC_counts, last_press = 10000, cycle, old_accel_offset = 0;		//Cut out intermediate variables.  
signed int sensor_values[6];		//calibrated, and in engineering units
									//sensor_values[0]:  X accel in % Gs
									//sensor_values[1]:  Y accel in % Gs
									//sensor_values[2]:  Continuity voltage in millivolts
									//sensor_values[3]:  Sensor ref voltage in millivolts
									//sensor_values[4]:  Temperature in deci-degrees F
									//sensor_values[5]:  Pressure in millibars
unsigned int meas_index = 0,  buff_meas_index = 0, processed_index = 0, page_index=0;
unsigned char flight_mode = PRELAUNCH, buffer_index=0, flight_index=1, next_channel_index = 1, ready_for_liftoff=0;

int adc_settings0[3] = {0x83, 0x88, 0xc7};
int adc_settings1[3] = {0x85, 0x85, 0x05};
//int other_chan0[4] = {0x84, 0x80, 0x8E, 0xC7};	// 30k limit continuity, half v, temp, press, 30k limit
//int other_chan1[4] = {0x80, 0x81, 0x81, 0x05};	// 30k limit continuity, half v, temp, press, 30k limit
int other_chan0[4] = {0x84, 0x80, 0x8E, 0x87};	// 100k limit continuity, half v, temp, press, 
int other_chan1[4] = {0x80, 0x81, 0x81, 0x85};	// 100k limit  continuity, half v, temp, press, 100k limit

void main()
{

	unsigned int  button_time = 5, output_cycle[3] = {0,0,0}, continuity;
	signed int  filt_accel_times_16 = 0, accel_offset = 0;
	unsigned char batt_low = 0, button_pushed=1, all_sensor_index = 0;
	signed long temp1;
	signed int slope[6] = {-21205, 10436, -8177, 12924, 4308, 4308}; //SN 2
 	signed int offset[6] = {-57, -8, 6324, -23, -23, 1508};		//SN 2, press in pratio
	signed int meas_temperature[6], temp_slope_adj[6], temp_offset_adj[6]; //temperature adjustment parameters
	signed int cal_slope[6], cal_offset[6]; //temperature adjustment parameters
	unsigned char i, deployed_flag=0;

   	SET_VECTOR(TIMER0, isr_timer0);  			// Set interrupt service routine ISR for timer0
   	SET_VECTOR(UART0_RX, isr_uart0_rx);  		// Set interrupt service routine ISR for UART 

   	initialize();                                    // Initialize the system
	Init_SPI_Master();								//Initialize the  SPI port
		//Read calibration parameters
		Assert_SS();
		Shift_Bytes(0x03000000,4);					//read page command ;plus address
//		Shift_Bytes(0,3);					//  Address, start at 0 in the buffer
//		Shift_Bytes(0x03,1);					//read page command
//		Shift_Bytes(0,3);					//  Address, start at 0 in the buffer
		Shift_Bytes(0,2);			// skip the calibration index from the first byte;
		for (i = 0; i <6; i++) {
			cal_offset[i] = Shift_Bytes(0,2);		//read int-length offset
			cal_slope[i] = Shift_Bytes(0,2);			//read long-length slope
			meas_temperature[i] = Shift_Bytes(0,2);  	//reference temperature for this sensor
			temp_offset_adj[i] = Shift_Bytes(0,2);  	//offset adjustment based on temperature
			temp_slope_adj[i] = Shift_Bytes(0,2);  	//slope adjustment based on temperature
		}  //The above reads the first 62 bytes.  Continue to the next page by reading 528-62 = 466 bytes
		Deassert_SS();
		// Now read the event program values.
		Assert_SS();
//		for (i = 1; i <234; i++) {
//			Shift_Bytes(0,2);
//		}
		Shift_Bytes(0x03000C00,4);					//read page command plus address for page 3, shifted 10 bits 

		//Byte assignment:  mask1a, mask1b, mask2a, mask2b, mask3a, mask3b, Tvala, Tvalb, P_val_1a, P_val_1b, P_val_2a, P_val_2b
		for (i = 0; i <12; i++) {
			program_data[i]  = Shift_Bytes(0,1);
		}
		Deassert_SS();

//		slope[3] = slope[3] * 3;	//Special case for Vbatt voltage divider
	EI();											// Enable Interrupts
	ADCCTL1 = adc_settings1[0];						// Set up first ADC channel 	
	ADCCTL0 = adc_settings0[0];						// Finish setting up 1st channel and start ADC

	while (1){
		new_step = 0;
		//Read sensors and store data
  		adc_settings0[2] = other_chan0[cycle];	//Set the other channel settings once each cycle
 		adc_settings1[2] = other_chan1[cycle];
		processed_index = 0;
		// While waiting for the ADC, adjust the calibration coefficients for temperature
		for (i = 0; i <6; i++) {
			slope[i] = cal_slope[i] + (int)( ( ((long)sensor_values[4]-(long)meas_temperature[i]) * temp_slope_adj[i]) >> 13);
			offset[i] = cal_offset[i] + (int)( ( ((long)sensor_values[4]-(long)meas_temperature[i]) * temp_offset_adj[i]) >> 13);
		}
		while (processed_index < 3) {
			while (ADCCTL0 >> 7) 	{					// just wait for ADC conversion to be complete
//				adc_wait++;
			}
			ADCCTL1 = adc_settings1[next_channel_index];	//	start collecting ADC values for next channel    
			ADCCTL0 = adc_settings0[next_channel_index];	//	start collecting ADC values for next channel
		    if (++next_channel_index > 2) next_channel_index = 0;	//Increment next channel index

 			ADC_counts = (((int)ADHR<<8) | ADLR) >> 3;	//Updated grab of the ADC values
			if (processed_index == 2){
				all_sensor_index = processed_index + cycle;			//correctly IDs pressure, temperature, batt_v or half_v
			}else {
				all_sensor_index = processed_index;
			} 
			temp1 = (long)(ADC_counts - offset[all_sensor_index]) * (long)slope[all_sensor_index];
			sensor_values[processed_index] = (int) (temp1 >> 13);
//			sensor_values[1] = velocity;
			if (all_sensor_index == 0) {//about to record ref voltage, record FER instead
				sensor_values[0] =sensor_values[0] * 4;
			}
			if (cycle == 1) {//about to record ref voltage, record FER instead
				sensor_values[2] = FER;
				//sensor_values[2] = velocity;
			}
//				sensor_values[2] = filtered_press;}
			if (flight_mode > 0x0d && page_index < 768 && (page_index < 700 || (major_cycle == 0)) ) {  //Looking for 1 Hz data
				Shift_Bytes(sensor_values[processed_index],2);	//Write data to buffer.  Assumes that flash chip has already been commanded into a write cycle
				buff_meas_index++;
			}
			processed_index ++;
		}										//Line complete after 3 channels written to flash
		//Manage buffer swap
		if (buff_meas_index > 263){				//Each page holds 88 lines x 3 channels of data x 2 bytes, (264 2-byte measurements)
			buff_meas_index = 0;
			buffer_index = !buffer_index;		//Toggle from buffer 0 to 1 or vice-versa.  buffer_index is the new buffer index.
			if (flight_mode == LIFTOFF) Prog_Other_Buffer();
			Start_Buffer_Write();				//Start writing to buffer with index 0
		}

		sensor_values[2 +cycle] = sensor_values[2];		//Store off batt V, ref V, or temperature into other slots when it's their turn to be the extra channel
//		sensor_values[0] = (sensor_values[0])*4;		//Multiply by 4 to account for 1/4 scale slope
		//Propagate measurements
		filt_accel_times_16 += (sensor_values[0] -(filt_accel_times_16>>4));		// 1/16 gain recursive filter
		//Only integrate the velocity if the accel > 1.5 or the rocket is in liftoff flight_mode.
		velocity += (sensor_values[0] - old_accel_offset) * (flight_mode == LIFTOFF || sensor_values[0] > 300);

		// USB Output data at 1 Hz
		if (major_cycle == 0 && cycle == 0) {
			continuity = sensor_values[2] > 200;
		//			T1RH = 14-(char)(sensor_values[2] >>9);  //Set the beeper tone according to continuity while cycle = 0
//			T1RL = 0xff;  
	 		if (flight_mode == WAIT && !enable_program) {
				Print_Line();
			}
		}
		//Handle button pushes
		button_pushed = PAIN & 0x40;  //Check button
		if (button_pushed)  {
			button_time++;				//Time, in minor (5 ms) cycles
			if (flight_mode != LIFTOFF && button_time > 400) {		//Button held down for 2 seconds, at least 10 seconds into program
				PAOUT &= ~0x02;			//Turn off power enable.  Board powers down when button released.
				LEDEN |= 32;						//Turn on red LED
				LEDEN |= 16;						//Turn on green LED
			}
		} else if (button_time >=1)  { //The button was pushed but now it's done
			button_time = 0;		//Only do one flight_mode transition per button push
			if (flight_mode == PRELAUNCH) {
				flight_mode = WAIT;
			} else 	if (flight_mode == WAIT) {
				flight_mode = PRELAUNCH;
				//Increment flight counter and erase current memory space, unless it's already been done since the last liftoff.
				while (Flight_Is_Full() && flight_mode == PRELAUNCH) {
					if (++flight_index > 5) {
						flight_mode = WAIT;	//Increment Flight unless there's no more room
						flight_index = 1;
					}
				}
				buff_meas_index = 0;
				page_index = 0;
				PAOUT = 2;	//Turn off any latched outputs	
				//Convert ASCII-typable time and pressure values to 10-bit values and initialize output stop time register
				for (i = 0; i < 3; i++) {
					output_start_cycle[i] = -1;
					t_p_vals[i] = ((program_data[6+ i*2] & 62) << 7) + ((program_data[7+i*2] & 62) << 2);
				}
				FER = 0;	
				velocity = 0;
				cycle = -1;		//Make it zero when the control comes around next and starts writing to the new buffer
			}
			if ((flight_mode == LIFTOFF || flight_mode == BEEP) && seconds > 60) {		//Make sure to get at least 60 sec of data before exiting liftoff flight_mode
				flight_mode = WAIT;
			}
		}

		//Special flight_mode-specific events
		//  ***Pre-launch***
		if (flight_mode == PRELAUNCH) {	
			LEDEN &= ~64;						//Turn off blue LED
			flight_index += !flight_index;		//Increment flight flight_mode if it's 0
			Switch_Beep(continuity * 0x10 + 1);				//turn on low beep
			if (velocity >=8000) {		// about 27 ft/sec
				flight_mode = LIFTOFF;					//Change flight_mode in time to go into launch flight_mode logic
				seconds = 0;
				msec = 0;
				Prog_Other_Buffer();
				Start_Buffer_Write();
			} else if (filt_accel_times_16 < 1280 && velocity > 0) velocity = 0;  //Reset velocity count if unit is dropped pre-launch
		}
		//  ***Liftoff***
		if (flight_mode ==LIFTOFF)	{		
			LEDEN |= 32;						//Turn on red LED
			Switch_Beep(0x00);
/* Flight Event mask.  16 bits with reserved bits so that ASCII conversion is typable.
8 	Liftoff				8	Reserved 0
4 	Reserved 0			4	Reserved 1
2	Burnout				2	Pressure Increasing
1	Timer<time			1	Latch the output indefinitely
8	Timer>time			8	Pressure decreasing
4	delta-P<input_P1	4	Velocity < 400 mph
2	delta-P>input_P2	2	Velocity < 0
1	Reserved 00			1	Reserved 0
*/
			//msec++;		//Moved to major cycle increment
			full_cycle = (major_cycle<<2) + cycle;
			FER = 0xC1D1;	//Default has zeros only for legit flight events
			FER |= (0x2000 * (sensor_values[0] < 0));
			FER |= (0x1000 * (msec < t_p_vals[0]));		
			FER |= (0x0800 * (msec > t_p_vals[0]));
			FER |= (0x0400 * ((signed int)((signed int)liftoff_press -(signed int)filtered_press) < (signed int) t_p_vals[1]));
			FER |= (0x0200 * ((signed int)((signed int)liftoff_press -(signed int)filtered_press) > (signed int) t_p_vals[2]));
			FER |= (0x0020 * (filtered_press > last_press));	//Pressure increasing
			FER |= (0x0008 * (filtered_press < last_press));	//Pressure decreaseing
//			FER |= (0x0004 * (velocity < 169528));	//196 mph
			FER |= (0x0004 * (velocity < 840367));	//specific for Brian andrew deihl
			FER |= (0x0002 * (velocity < 0));
			last_press = filtered_press;

			//Control Outputs
			for (i = 0; i<3; i++) {
				output_mask = (int)((program_data[2*i])<<8) | (int)( program_data[2*i+1]);
				if(output_start_cycle[i] == full_cycle) {  //time to turn the output off
					PAOUT &= ~port_mask[i];
				}			
				if (output_start_cycle[i] == 255  && (FER | output_mask) == 0xffff) {	//If the pyro hasn't fired yet
					PAOUT |=  port_mask[i];  //If all the FER events are true or don't-care
					//Set the off time, except for latching outputs (xmt switch)
					output_start_cycle[i] = full_cycle;
					if (output_mask & 0x0010) {
				 		output_start_cycle[i] = 230;
					}
				}
//				temp = output_start_cycle[i];
			}
			if (page_index >= 767) {	//If the page index would be about to 
				flight_mode = BEEP;
			}
		//	*** Beep flight_mode ***
//		} else if (flight_mode == BEEP) {
			
			//Switch_Beep(0x13);				//turn on long-short beep

		//	*** Download flight_mode ***
		} else if (flight_mode == DOWNLOAD) {
			Download_Data();
			flight_mode = WAIT;
		} else if (flight_mode == WAIT) {
			LEDEN &= ~32;						//Turn oFF red LED
			LEDEN &= ~64;						//Turn oFF blue LED
			Switch_Beep(0);
			msec = 0;
	  	}

   		if(++cycle == 4) {
			cycle=0;	//Start new minor cycle
			major_cycle++;	//Start new minor cycle
			msec++;
			//4x pressure data increases precision.  1/16 gain filter overall.
			filtered_press_times4 += (signed int)((sensor_values[5] -(signed int)(filtered_press_times4 >> 2)) >>2);		
			filtered_press = filtered_press_times4 >>2;
			}

		if (major_cycle > 49) { //Major cycle.  Once per second
			major_cycle = 0;
			seconds ++;
			//msec = 0;	 msec taken care of during flight with continuously-incrementing counter for deployments.  Download takes care of it during download.
			if (flight_mode != LIFTOFF) {
				old_liftoff_press = liftoff_press; // Double-buffer pressure values
				liftoff_press = filtered_press;
				if (filt_accel_times_16 > 1280) {	//Double-buffer pre-launch accel if Parrot is resting with more than 0.8 Gs
					old_accel_offset = accel_offset;
					accel_offset = filt_accel_times_16 >> 4;
				} else if (filt_accel_times_16 < -1280) {	//Flip the sign convention for up
					cal_slope[0] = -cal_slope[0];
					accel_offset = -accel_offset;
				} 
			}
/*			if (flight_mode != LIFTOFF) {	//Double-buffer pre-launch accel and pressure values
				old_accel_offset = accel_offset;
				accel_offset = filt_accel_times_16 >> 4;
				if (accel_offset < -80) {	//Flip the sign convention for up
					cal_slope[0] = -cal_slope[0];
					accel_offset = -accel_offset;
				} 
				old_liftoff_press = liftoff_press;
				liftoff_press = filtered_press;
			}*/
			if (flight_mode == WAIT) {
				LEDEN |=64;
	//			Print_Line();	// USB Output data at 1 Hz
			}
		}
/*		if (sensor_values[3] < 3200) {	
			batt_low++;
			if (batt_low >=3) {		// If 3 consecutive samples < 3.20 V
//				PAOUT &= ~2;		//Turn yourself off
			}
		} else {
			batt_low = 0;
		}
*/
//		margin_time = 0;
		while (new_step ==0) {
//			margin_time++;
		} 
	}	// end while loop
}  //end main subroutine

/*
******************************************************************************
**
**  Routine    :      isr_timer0
**  Parameters :      None
**  Return     :      int
**
**  Purpose:	Increment time and exit the halt flight_mode
**
******************************************************************************
*/
#pragma interrupt

void isr_timer0(void) 
{
	new_step=1;
//	if (cycle == 3)
//		save_margin_time = margin_time;
} 
 
/*
******************************************************************************


/*
******************************************************************************
**
**  Routine    :      isr_uart0
**  Parameters :      None
**  Return     :      int
**
**  Purpose:
**
******************************************************************************
*/
#pragma interrupt

void isr_uart0_rx(void) 
{
	int input;
	char i;

	flight_mode = WAIT;
	input = getch();
	if (!enable_program) {
		if (input == (int)'d') {
  			flight_mode = DOWNLOAD;	
		} else if (input < 54  && input >47) {
			flight_index = input - 48;	// Choose flight index to work on
		//ready_for_liftoff = 0;
		} else if (input == 'E') {	//Erase flight
			Erase_Flight();
		} else if (input == 'P') { //Program the altimeter
			enable_program++;
//			LEDEN |= 16;
		} 
	} else {
		program_data[enable_program++ -1] = input;
		putch(input);
		if (enable_program > 12) {  //End programming phase.  Write all program values to flash at once to make sure it's contiguous
			enable_program =0;
			Deassert_SS();
			Assert_SS();
			Shift_Bytes(0x82000C00,4);	//Command a Write to flash through buffer with erase, plus address
//			Shift_Bytes((3 << 10),3); //Page address.  Write to page 1
			for (i = 0; i < 13; i++) {
				Shift_Bytes(program_data[i],1);
			}
			Deassert_SS();
			Wait_For_Flash();
//		} else if (enable_program > 5) { //Time and pressure values
			//Byte assignment:  mask1a, mask1b, mask2a, mask2b, mask3a, mask3b, Tvala, Tvalb, P_val_1a, P_val_1b, P_val_2a, P_val_2b
//				program_data[enable_program] = (program_data[enable_program] &= ~193)>>1;  //Take the middle 5 bits and right-justify them
		}
	}

} 

 void Download_Data(void) 
{
	int d_seconds = 0, junk;
	unsigned int msec_increment = 5;
	major_cycle = 0;
	seconds = 0;
	msec = 0;
	cycle = 0;
	Deassert_SS();			// End previous write command
	Assert_SS();			// Start a new command
	Shift_Bytes(0x3,1);											//Command a continuous read
	if (flight_index > 0) {
		Shift_Bytes((FLIGHT_START + (flight_index-1) *768) << 10,3);		//12-bit page address shifted left 10 bits
	} else {
		Shift_Bytes(0,3);		//Special case for spitting back calibration data.  Feb update: No extra byte now
		//Shift_Bytes(0,4);		//Special case for spitting back calibration data.  Extra byte for skipping past cal data index
	}
//	for (seconds =0; seconds < 1914 && flight_mode == DOWNLOAD;) {		//51600 lines divided by 4
	while( seconds < 1804 && flight_mode == DOWNLOAD) {		//51600 lines divided by 4
		sensor_values[0] = Shift_Bytes(0,2);
		sensor_values[1] = Shift_Bytes(0,2);
		sensor_values[2 + cycle] = Shift_Bytes(0,2);

		Print_Line();
//		Assert_SS();
		if (++cycle > 3) cycle = 0;
		if ((msec += msec_increment) > 995){
//		if ((msec += 5) > 995){
//			major_cycle = 0;
			msec = 0;
			seconds++;
		}
		if (seconds > 307 && cycle ==3) {
			msec_increment = 985;
		} else {
			msec_increment = 5;
		}
		//} //end cycle loop
	} //end row loop
	Deassert_SS();
}

 void Print_Line(void) 
{
char i;
char *str;
//	printf("%4d.%03d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", seconds, msec, x_accel, y_accel, press_ratio,batt_v,half_v,temperature,save_margin_time);
//	printf("%4d.%03d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", seconds, msec,
//	sensor_values[0], sensor_values[1], sensor_values[2], sensor_values[3], sensor_values[4], sensor_values[5], flight_index);
//	printf("%4d.%03d\n", seconds, save_margin_time);
	putstring(itoa(seconds));
	putch('.');
	if (msec < 100)  {
		putch('0');
		if (msec < 10) {
			putch('0');
		}
	}
	putstring(itoa(msec));
	putch('\t');
	for (i = 0; i <6; i++) {
		putstring(itoa(sensor_values[i]));
		putch('\t');
	}
	putch(flight_index + 48);
//	putch('\t');
//	putstring(itoa(Calc_Altitude(sensor_values[5])));
//	putch('\t');
//	putstring(itoa(FER));
//	putch('\t');
//	putstring(itoa(velocity));
//	putch('\t');
	putch('\n');
}
void Switch_Beep(int on_mask)
{
	//on_mask defines when the beep is on vs. off.  Each bit corresponds to 5 ms.  Only the first 10 bits are important.
	if (on_mask & (1<<(major_cycle>>2))) {
//		PCOUT |= 1;				// Turn on the enable
//		PACTL  |= 0x80;			// Make port A7 alternate function (timer) 
		T1CTL1 = 0x81;			//Turn on timer
		LEDEN |= 16;
	} else {
//		PCOUT &= ~0x01;			// Turn off the enable
		T1CTL1 &= ~0x80;			//Turn off timer
//		PACTL  &= ~0x80;		// Make port A7 GPIO in prep for SPI 
		LEDEN &= ~16;
	}
}		
/*
int Calc_Altitude(int press_ratio) 
{
	if (press_ratio >=8350) {
		altitude = (int)(((long)(9986-press_ratio) * 194923) >> 16);	//Low-altitude approximation, good for 0-4860 ft
	} else {
		altitude = (int)(((long)(8352-press_ratio) * 225116) >> 16 + 4860);  //high-altitude approximation, good for 4860-10,000 ft
	}
	return altitude;
}
*/
void Erase_Flight(void)
{
	char i=0;
	for (i = 00; i<3; ++i) {
		Assert_SS();
		Shift_Bytes(0x7c,1);						//Sector Erase Command
		if (flight_index ==0) flight_index = 1;
		Shift_Bytes(((flight_index-1)*3 + 1 + i) << 18,3);	//sector address
		Deassert_SS();
		Wait_For_Flash();
	}
}
void Wait_For_Flash(void)
{
	char stat =80, red_led_on=0;
	while (!(stat >>7)) {  //Pick off the msb to determine when ready
		Assert_SS();
		stat = Shift_Bytes(0xd7 <<8, 2);	//Status register command, followed by 8 dummy bits
		Deassert_SS();
/*		if (new_step) {
			new_step = 0;	//Do this just once per 5 ms interrupt
			red_led_on = ~red_led_on;
			if (red_led_on) {
		*/		LEDEN |= 64; /* blue light
				Switch_Beep(0xffff);
			}else{
				LEDEN &= ~32;
				Switch_Beep(0);
			}
		}
*/
	}
				LEDEN &= ~64;
}

char Flight_Is_Full(void)
{
	long unsigned int test;
	//char return_value;
	Deassert_SS();
	Assert_SS();
	Shift_Bytes(0x03,1);						//Command a read
	Shift_Bytes(((flight_index-1)*3 + 1)<<18,3);	//24-byte address, 1st byte of flight
	test = Shift_Bytes(0,4);						//1st 4 bytes of flight
	Deassert_SS();
	return (test < (unsigned long int) 0xffffffff);
//	return return_value;
}

void Prog_Other_Buffer(void)
{				//Write the buffer to main memory
	Deassert_SS();						//End the write cycle to the current buffer
	Assert_SS();
	Shift_Bytes(0x88 + !buffer_index,1);			//Command the previous buffer to be written to write to memory
	Shift_Bytes((FLIGHT_START + (flight_index-1) *768 + page_index++) <<10, 3);	//  Page address, middle 12 bits of 1st 2 bytes
	//Deassert_SS();					//End buffer program to flash command
	//Start_Buffer_Write();
}
void Start_Buffer_Write(void)
{
	Deassert_SS();					//End buffer program to flash command
	Assert_SS();					//re-start write to buffer command
	Shift_Bytes(0x84 + 3*buffer_index,1);	//Write to current buffer command
	Shift_Bytes(buff_meas_index*2,3);	//  Address, start at current index in the buffer
}

char* itoa(signed int val){
	
	static char buf[32] = {0};
	char isneg = 0;
	
	int i = 30;
	if (val < 0) {
		isneg = 1;
		val = val * (-1);
	} else if (val == 0) {
		return "0";
	}

	for(; val && i ; --i, val /= 10)
		buf[i] = "0123456789"[val % 10];

	if (isneg) {
		buf[i] = '-';
	} else {
		buf[i] = '';
		++i;
	}
	return &buf[i];
	
}

//my version of puts
void putstring(char * str)
{
  for(;*str;++str)
    putch(*str);
} 
/*
int Calc_Altitude(long pressure_Pa) 
{ /*threshold	square coeff	lin coeff	sclar coeff
		0		722				35657		3
		5750	1123			31446		353
		9750	1646			21872		1723
		12750	2512			597			5809
		15550	4163			-49128		17519

	unsigned long alt_counts_ov_4, alt_square, square_temp, lin_temp;
	unsigned int threshold[5] = {5750, 9750, 12750, 15550, 20000}, altitude;
	unsigned long A[5] = {722, 1123, 1646, 2512, 4163};
	signed long B[5] = {35657, 31446, 21872, 597, -49128};
	unsigned long C[5] = {3, 353, 1723, 5809, 17519};
	char ind = 0;

	alt_counts_ov_4 = (101000 -pressure_Pa)>>2;
	alt_square = (alt_counts_ov_4 * alt_counts_ov_4) >> 15;  //square divided by 2^15

	while(alt_counts_ov_4 > threshold[ind])  { //Determine which set of coeff to use
		ind++;
	}
	square_temp = (A[ind] * alt_square) >> 10;
	lin_temp = ((B[ind] * alt_counts_ov_4)) >> 15 + C[ind];
// 	lin_temp = lin_temp >> 15;
//   lin_temp = lin_temp + C[ind];
	altitude = square_temp + lin_temp;
	return altitude;
}
*/
/*
******************************************************************************
**  End of File
******************************************************************************
*/
