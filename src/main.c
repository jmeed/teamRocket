#include <stdio.h>
#include "chip.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "logging.h"
#include "error_codes.h"
#include "ff.h"
#include "drivers/uart0.h"
#include "drivers/spi.h"
#include "drivers/sdcard.h"

#include <stdlib.h>
#include <string.h>
#include "L3G.h"
#include "LPS.h"
#include "LSM303.h"

#include "H3L.h"
#include "LSM.h"

#include "Drivers/interrupts.h"
#include "Drivers/gpio.h"

#define ON_N 0
#define OFF_N 1
// #define LONG_TIME 0xffff
// #define TICKS_TO_WAIT    10

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#define DEFAULT_I2C          I2C0

#define I2C_EEPROM_BUS       DEFAULT_I2C
#define I2C_IOX_BUS          DEFAULT_I2C

#define SPEED_100KHZ         100000
#define SPEED_400KHZ         400000
#define I2C_DEFAULT_SPEED    SPEED_100KHZ
#define I2C_FASTPLUS_BIT     0

#if (I2C_DEFAULT_SPEED > SPEED_400KHZ)
#undef  I2C_FASTPLUS_BIT
#define I2C_FASTPLUS_BIT IOCON_FASTI2C_EN
#endif

static int mode_poll;   /* Poll/Interrupt mode flag */
static I2C_ID_T i2cDev = DEFAULT_I2C;   /* Currently active I2C device */

static volatile uint32_t tick_cnt;

static void Init_I2C_PinMux(void) {
    Chip_SYSCTL_PeriphReset(RESET_I2C0);
#if defined(BOARD_MANLEY_11U68)
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5, IOCON_FUNC1 | I2C_FASTPLUS_BIT);

#elif defined(BOARD_NXP_LPCXPRESSO_11U68)
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4,
        (IOCON_FUNC1 | I2C_FASTPLUS_BIT) | IOCON_DIGMODE_EN);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5,
        (IOCON_FUNC1 | I2C_FASTPLUS_BIT) | IOCON_DIGMODE_EN);

#else
#warning "No I2C Pin Muxing defined for this example"
#endif
}


/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */


/*static void setup_pinmux() {
    // SPI0 SDCard
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 12, (IOCON_FUNC1 | IOCON_MODE_INACT)); // MOSI
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 29, (IOCON_FUNC1 | IOCON_MODE_INACT)); // SCK
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 8, (IOCON_FUNC1 | IOCON_MODE_INACT)); // MISO
}

static void debug_uart_init(void) {
    Chip_Clock_SetUSARTNBaseClockRate((115200 * 256), false);
    uart0_init();
    uart0_setup(115200, 1);
}

static void hardware_init(void) {
    // Setup UART clocks

    setup_pinmux();
    spi_init();
    spi_setup_device(SPI_DEVICE_0, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE0, true);
    SDCardInit();
}

static void prvSetupHardware(void)
{
    SystemCoreClockUpdate();
    Board_Init();
}*/


L3G gyro;
LPS baro;
LSM303 magXL;

SemaphoreHandle_t i2c_bus1_Semaphore; // gyro, xl, baro
SemaphoreHandle_t i2c_bus2_Semaphore; // oled, gps, radio
SemaphoreHandle_t timer_Semaphore;

bool OLED_present = FALSE;
bool GPS_present = FALSE;
bool RADIO_present = FALSE;

// /* Timer ISR */
// void vTimerISR( void * pvParameters ){
//     static unsigned char ucLocalTickCount = 0;
//     static signed BaseType_t xHigherPriorityTaskWoken; 
 
//     /* A timer tick has occurred. */
    
//     // ... Do other time functions.
    
//     /* Is it time for vATask() to run? */
//     xHigherPriorityTaskWoken = pdFALSE;
//     ucLocalTickCount++;
//     if( ucLocalTickCount >= TICKS_TO_WAIT )
//     {
//         /* Unblock the task by releasing the semaphore. */
//         xSemaphoreGiveFromISR( timer_Semaphore, &xHigherPriorityTaskWoken );
        
//         /* Reset the count so we release the semaphore again in 10 ticks time. */
//         ucLocalTickCount = 0;
//     }
    
//     /* If xHigherPriorityTaskWoken was set to true you
//     we should yield.  The actual macro used here is 
//     port specific. */
//     portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
// }

void read_setup(){
    // read setup.txt
    // OLED_present = 
    // GPS_present =
    // RADIO_present =
}

void vtask_init(void *pParam){
    LOG_INFO("Start: vtask_init");
    if(i2c_Semaphore = xSemaphoreCreateBinary()){
        // no error
    }else{
        LOG_INFO("ERROR: I2C semaphore failure");
    }

    if(L3G_init(I2C0)){
        L3G_enable();
        // blink LED
        // send bluetooth update
    }else{
        LOG_INFO("ERROR: L3G initization failure");
    }

    if(LSM303_init(I2C0)){
        LSM303_enable();
        // xl reg 0x30 - 0x33
        // setup launch interrupt
        // blink LED
        // send bluetooth update
    }else{
        LOG_INFO("ERROR: LMS303 initization failure");
    }

    if(LPS_init(I2C0)){
        LPS_enable();
        // blink LED
        // send bluetooth update
    }else{
        LOG_INFO("ERROR: LPS initization failure");
    }

    LOG_INFO("Finish: vtask_init has been suspended");
    vTaskSuspend(vtask_init);
}
    

void vtask_setup(void *pParam){
    LOG_INFO("Start: vtask_setup");

    read_setup();

    xTaskCreate(vtask_read_gyro, "gyro" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    xTaskCreate(vtask_read_XL, "accel" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    // xTaskCreate(vtask_read_HiG_XL, "hig_accel" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    xTaskCreate(vtask_read_mag, "mag" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    xTaskCreate(vtask_read_baro, "baro" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    xTaskCreate(vtask_read_temp, "temp" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);

    // if(GPS_present){
    //     xTaskCreate(vtask_read_GPS, "gps", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    // }

    // if(RADIO_present){
    //     xTaskCreate(vtask_write_radio, "radio", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    // }

    LOG_INFO("Finish: vtask_setup has been suspended");
    vTaskSuspend(vtask_setup);
}

//DEBUGOUT("Temps: baro = %.2f, gyro = %.2f, imu = %.2f\n", baro_T, gyro_T, imu_T);
        
        //DEBUGOUT("MAG/XL: ax = %.2f, ay = %.2f, az = %.2f, mx = %.2f, my = %.2f, mz = %.2f, temp = %.2f\n", acc_x, acc_y, acc_z, mag_x, mag_y, mag_z, imu_T);
        

void vtask_read_gyro(void *pParam){

    static volatile float gyro_x = 0.0f; 
    static volatile float gyro_y = 0.0f; 
    static volatile float gyro_z = 0.0f; 
    static volatile float gyro_T = 0.0f;
    if(xSemaphoreTake(i2c_bus1_Semaphore, (TickType_t) 10) == pdTRUE){
        gyro_x = L3G_read_data(L3G_SPIN_RATE_X);
        gyro_y = L3G_read_data(L3G_SPIN_RATE_Y);
        gyro_z = L3G_read_data(L3G_SPIN_RATE_Z);
        gyro_T = L3G_read_data(TEMPERATURE);
        DEBUGOUT("Gyro: x = %.2f, y = %.2f, z = %.2f, temp = %.2f\n", gyro_x, gyro_y, gyro_z, gyro_T);
        xSemaphoreGive(i2c_bus1_Semaphore);
    }else{
        // could not get semaphore
    }
    
}

void vtask_read_XL(void *pParam){

    static volatile float acc_x = 0.0f; 
    static volatile float acc_y = 0.0f; 
    static volatile float acc_z = 0.0f; 
    static volatile float imu_T = 0.0f;
    if(xSemaphoreTake(i2c_bus1_Semaphore, (TickType_t) 10) == pdTRUE){
        acc_x = magXL.read_data(ACCEL_X);
        acc_y = magXL.read_data(ACCEL_Y);
        acc_z = magXL.read_data(ACCEL_Z);
        imu_T = magXL.read_data(TEMPERATURE);
        DEBUGOUT("XL: ax = %.2f, ay = %.2f, az = %.2f, temp = %.2f\n", acc_x, acc_y, acc_z, imu_T);
        xSemaphoreGive(i2c_bus1_Semaphore);
    }else{
        // could not get semaphore
    }
    
}

void vtask_read_HiG_XL(void *pParam){

    static volatile float hig_acc_x = 0.0f; 
    static volatile float hig_acc_y = 0.0f; 
    static volatile float hig_acc_z = 0.0f; 
    static volatile float hig_imu_T = 0.0f;
    if(xSemaphoreTake(i2c_bus1_Semaphore, (TickType_t) 10) == pdTRUE){
        hig_acc_x = LSM303_read_data(ACCEL_X);
        hig_acc_y = LSM303_read_data(ACCEL_Y);
        hig_acc_z = LSM303_read_data(ACCEL_Z);
        hig_imu_T = LSM303_read_data(TEMPERATURE);
        DEBUGOUT("XL: ax = %.2f, ay = %.2f, az = %.2f, temp = %.2f\n", hig_acc_x, hig_acc_y, hig_acc_z, hig_imu_T);
        xSemaphoreGive(i2c_bus1_Semaphore);
    }else{
        // could not get semaphore
    }
    
}

void vtask_read_mag(void *pParam){

    static volatile float mag_x = 0.0f; 
    static volatile float mag_y = 0.0f, 
    static volatile float mag_z = 0.0f;
    // static volatile float magH = 0.0f;
    if(xSemaphoreTake(i2c_bus1_Semaphore, (TickType_t) 10) == pdTRUE){
        mag_x = LSM303_read_data(MAG_X);
        mag_y = LSM303_read_data(MAG_Y);
        mag_y = LSM303_read_data(MAG_Z);
        // magH = LSM303_read_data(MAG_HEADING);
        DEBUGOUT("MAG: mx = %.2f, my = %.2f, mz = %.2f\n", mag_x, mag_y, mag_z);
        xSemaphoreGive(i2c_bus1_Semaphore);
    }else{
        // could not get semaphore
    }
    
}

void vtask_read_baro(void *pParam){

    static volatile float baro_alt = 0.0f;
    static volatile float baro_T = 0.0f;
    if(xSemaphoreTake(i2c_bus1_Semaphore, (TickType_t) 10) == pdTRUE){
        baro_alt = LPS_read_data(LPS_ALTITUDE);
        baro_alt = LPS_read_data(TEMPERATURE);
        DEBUGOUT("Baro: alt = %.2f, temp = %.2f\n", baro_alt, baro_T);
        xSemaphoreGive(i2c_bus1_Semaphore);
    }else{
        // could not get semaphore
    }
    
}

void vtask_read_temp(void *pParm){
    static volatile float baro_T = 0.0f;
    static volatile float imu_T = 0.0f; 
    static volatile float gyro_T = 0.0f;
    if(xSemaphoreTake(i2c_bus1_Semaphore, (TickType_t) 10) == pdTRUE){
        baro_T = LPS_read_data(TEMPERATURE);
        imu_T = LSM303_read_data(TEMPERATURE);
        gyro_T = L3G_read_data(TEMPERATURE);
        DEBUGOUT("Baro: baro_temp = %.2f, imu_temp = %.2f, gyro_temp = %.2f\n", baro_T, imu_t, gyro_T);
    xSemaphoreGive(i2c_bus1_Semaphore);
    }else{
        // could not get semaphore
    }
}

void vtask_read_GPS(void *pParam){
    if(xSemaphoreTake(i2c_bus2_Semaphore, (TickType_t) 10) == pdTRUE){
        // empty
    }else{
        // could not get semaphore
    }
}

void vtask_write_radio(void *pParam){
    if(xSemaphoreTake(i2c_bus2_Semaphore, (TickType_t) 10) == pdTRUE){
        // empty
    }else{
        // could not get semaphore
    }
}



// GPIO18 pin 45 
void vtask_interrupt_launch(void *pParam){
    if(xSemaphoreTake(i2c_bus1_Semaphore, (TickType_t) 10) == pdTRUE){
        // empty
    }else{
        // could not get semaphore
    }

}

void main(void) {
    prvSetupHardware();

    debug_uart_init();
    LOG_INFO("Initializing hardware");
    hardware_init();

    LOG_INFO("Starting tasks");
    // DisableInterrupts();
    // InitInterruptController();


    xTaskCreate(vtask_init, "init", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    xTaskCreate(vtask_setup, "init", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    

    LOG_INFO("Tasks created; Starting scheduler");
    /* Start the scheduler */
    vTaskStartScheduler();


    exit_error(ERROR_CODE_MAIN_SCHEDULER_FALL_THRU);
    /*
    *   We should never get here, but just in case something goes wrong,
    *   we'll place the CPU into a safe loop.
    */
    while(1) {
        ;
    }
}
