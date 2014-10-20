#include <FreeRTOS.h>
#include <task.h>
#include "L3G.h"
#include "LSM303.h"
#include "LPS.h"

#include "Drivers/interrupts.h"
#include "Drivers/gpio.h"

#define ON_N 0
#define OFF_N 1
// #define LONG_TIME 0xffff
// #define TICKS_TO_WAIT    10

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

void task_init(void *pParam){
    if(i2c_Semaphore = xSemaphoreCreateBinary()){
        // no error
    }else{
        // error
    }

    if(gyro.init()){
        gyro.enable();
        // blink LED
        // send bluetooth update
    }else{
        // error
    }

    if(magXL.init()){
        magXL.enable();
        // xl reg 0x30 - 0x33
        // setup launch interrupt
        // blink LED
        // send bluetooth update
    }else{
        // error
    }

    if(baro.init()){
        baro.enable();
        // blink LED
        // send bluetooth update
    }else{
        // error
    }

    // all main sensors are running, init is done
    vTaskSuspend(task_init);
}
    

void task_setup(void *pParam){

    read_setup();

    xTaskCreate(task_read_gyro, "gyro" 128, NULL, 1, NULL);
    xTaskCreate(task_read_XL, "XL" 128, NULL, 2, NULL);
    xTaskCreate(task_read_mag, "mag" 128, NULL, 1, NULL);
    xTaskCreate(task_read_baro, "baro" 128, NULL, 1, NULL);
    
    if(OLED_present){
        xTaskCreate(task_write_OLED, "oled", 128, NULL, 2, NULL);
    }
    if(GPS_present){
        xTaskCreate(task_read_GPS, "gps", 128, NULL, 1, NULL);
    }
    if(RADIO_present){
        xTaskCreate(task_write_radio, "radio", 128, NULL, 1, NULL);
    }
    xTaskCreate(task_write_sdcard, "baro" 128, NULL, 1, NULL);

    // all tasks have bee created, setup is done
    vTaskSuspend(task_setup);
}

void task_read_gyro(void *pParam){

    volatile float gyroX, gyroY, gyroZ, gyroT;
    if(xSemaphoreTake(i2c_bus1_Semaphore,(TickType_t) 10) == pdTRUE){
        gyroX = gyro.read_data(SPIN_RATE_X);
        gyroY = gyro.read_data(SPIN_RATE_Y);
        gyroZ = gyro.read_data(SPIN_RATE_Z);
        gyroT = gyro.read_data(TEMPERATURE);
        xSemaphoreGive(i2c_bus1_Semaphore);
    }else{
        // could not get semaphore
    }
    
}

void task_read_XL(void *pParam){

    volatile float XLX, XLY, XLZ, XLT;
    if(xSemaphoreTake(i2c_bus1_Semaphore,(TickType_t) 10) == pdTRUE){
        XLX = magXL.read_data(ACCEL_X);
        XLY = magXL.read_data(ACCEL_Y);
        XLZ = magXL.read_data(ACCEL_Z);
        XLT = magXL.read_data(TEMPERATURE);
        xSemaphoreGive(i2c_bus1_Semaphore);
    }else{
        // could not get semaphore
    }
    
}

void task_read_mag(void *pParam){

    volatile float magX, magY, magZ, magH;
    if(xSemaphoreTake(i2c_bus1_Semaphore,(TickType_t) 10) == pdTRUE){
        magX = magXL.read_data(MAG_X);
        magY = magXL.read_data(MAG_Y);
        magZ = magXL.read_data(MAG_Z);
        magH = magXL.read_data(MAG_HEADING);
        xSemaphoreGive(i2c_bus1_Semaphore);
    }else{
        // could not get semaphore
    }
    
}

void task_read_baro(void *pParam){

    volatile float baro, baroT;
    if(xSemaphoreTake(i2c_bus1_Semaphore,(TickType_t) 10) == pdTRUE){
        baroX = baro.read_data(ALTITUDE);
        baroT = baro.read_data(TEMPERATURE);
        xSemaphoreGive(i2c_bus1_Semaphore);
    }else{
        // could not get semaphore
    }
    
}

void task_write_OLED(void *pParam){
    if(xSemaphoreTake(i2c_bus2_Semaphore,(TickType_t) 10) == pdTRUE){
        // empty
    }else{
        // could not get semaphore
    }
}

void task_read_GPR(void *pParam){
    if(xSemaphoreTake(i2c_bus2_Semaphore,(TickType_t) 10) == pdTRUE){
        // empty
    }else{
        // could not get semaphore
    }
}

void task_write_radio(void *pParam){
    if(xSemaphoreTake(i2c_bus2_Semaphore,(TickType_t) 10) == pdTRUE){
        // empty
    }else{
        // could not get semaphore
    }
}

void task_write_sdcard(void *pParam){
    // empty
}


// GPIO18 pin 45 
void interrupt_launch(void *pParam){

}

void main(void) {

    DisableInterrupts();
    InitInterruptController();

    xTaskCreate(task_init, "init", 128, NULL, 3, NULL);
    xTaskCreate(task_setup, "init", 128, NULL, 3, NULL);
    
    vTaskStartScheduler();

    /*
    *	We should never get here, but just in case something goes wrong,
    *	we'll place the CPU into a safe loop.
    */
    while(1) {
        ;
    }
}
