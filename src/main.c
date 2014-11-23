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

#include "H3L.h" // 3-axis high g accelerometer
#include "LSM.h" // 9-axis IMU
#include "LPS.h" // barometric pressure
#include "light_ws2812_cortex.h" // neopixel

#include <stdlib.h>
#include <string.h>
#include "Drivers/interrupts.h"
#include "Drivers/gpio.h"

#define DEFAULT_I2C          I2C0
#define SPEED_100KHZ         100000
#define SPEED_400KHZ         400000
#define I2C_DEFAULT_SPEED    SPEED_100KHZ
#define I2C_FASTPLUS_BIT     0

#define XAXIS 0x01
#define YAXIS 0x02
#define ZAXIS 0x03

#define SDCARD_START_RETRY_LIMIT 10

#if (I2C_DEFAULT_SPEED > SPEED_400KHZ)
#undef  I2C_FASTPLUS_BIT
#define I2C_FASTPLUS_BIT IOCON_FASTI2C_EN
#endif

// default values
// can be changed in setup file
static bool GPS_present = FALSE;
static bool RADIO_present = FALSE;
static bool FIRE_present = FALSE;
static bool CHANNEL_ONE = FALSE;
static bool CHANNEL_TWO = FALSE;
static bool CHANNEL_THREE = FALSE;
static bool CHANNEL_FOUR = FALSE;

static uint8_t imu_vert_axis = XAXIS;
static uint8_t hig_vert_axis = XAXIS;

static float baro_baseline = 0.0f;

static int mode_poll;   /* Poll/Interrupt mode flag */
static I2C_ID_T i2cDev = DEFAULT_I2C;   /* Currently active I2C device */

static volatile uint32_t tick_cnt;

static void setup_pinmux() {
    // SPI0 SDCard
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 21, (IOCON_FUNC2 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // MOSI
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 20, (IOCON_FUNC2 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // SCK
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, (IOCON_FUNC3 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // MISO
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, (IOCON_FUNC4 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 16, (IOCON_FUNC0 | IOCON_MODE_INACT) | IOCON_DIGMODE_EN); // SSEL


    // I2C on-board I2C0
    Chip_SYSCTL_PeriphReset(RESET_I2C0);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4,
        (IOCON_FUNC1 | IOCON_FASTI2C_EN) | IOCON_DIGMODE_EN);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5,
        (IOCON_FUNC1 | IOCON_FASTI2C_EN) | IOCON_DIGMODE_EN);

    // I2C off-board I2C1
    // Chip_SYSCTL_PeriphReset(RESET_I2C1);
    // Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4,
    //     (IOCON_FUNC1 | IOCON_FASTI2C_EN) | IOCON_DIGMODE_EN);
    // Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5,
    //     (IOCON_FUNC1 | IOCON_FASTI2C_EN) | IOCON_DIGMODE_EN);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 20);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 2);
}


static void debug_uart_init(void) {
    Chip_Clock_SetUSARTNBaseClockRate((115200 * 256), false);
    uart0_init();
    uart0_setup(115200, 1);
}

#define ONBOARD_I2C I2C0 // mainboard bus
// #define OFFBOARD_I2C I2C1 // wing bus
static void i2c_onboard_init(void) {
    Chip_I2C_Init(ONBOARD_I2C);
    // Chip_I2C_Init(OFFBOARD_I2C);
    Chip_I2C_SetClockRate(ONBOARD_I2C, 100000);
    // Chip_I2C_SetClockRate(OFFBOARD_I2C, 100000);
    // mode_poll &= ~(1 << id);
    Chip_I2C_SetMasterEventHandler(ONBOARD_I2C, Chip_I2C_EventHandler);
    // Chip_I2C_SetMasterEventHandler(OFFBOARD_I2C, Chip_I2C_EventHandler);
    NVIC_EnableIRQ(I2C0_IRQn);
    // NVIC_EnableIRQ(I2C1_IRQn);
}

void I2C0_IRQHandler(void)
{
    if (Chip_I2C_IsMasterActive(ONBOARD_I2C)) {
        Chip_I2C_MasterStateHandler(ONBOARD_I2C);
    }
    else {
        Chip_I2C_SlaveStateHandler(ONBOARD_I2C);
    }
}

// void I2C1_IRQHandler(void)
// {
//     if (Chip_I2C_IsMasterActive(OFFBOARD_I2C)) {
//         Chip_I2C_MasterStateHandler(OFFBOARD_I2C);
//     }
//     else {
//         Chip_I2C_SlaveStateHandler(OFFBOARD_I2C);
//     }
// }

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
}


void read_setup(){
    // this file includes settings for four channels
    // on firing board.
    // channels one and two are in meters
    // channels three and four are in sec
    if(user_file_found){
        // read user file
    }else{
        // read defualt file
    }
}

xTaskHandle boot_handle;

static FATFS root_fs;
void vtask_init(void *pvParameters){
    read_setup();
    LOG_INFO("Start: vtask_init");

    int result;
    LOG_INFO("Wait for voltage stabilization");
    vTaskDelay(1000);

    {
        int sdcard_retry_limit = SDCARD_START_RETRY_LIMIT;
        while (sdcard_retry_limit > 0) {
            LOG_INFO("Attempting to mount FAT on SDCARD");
            result = f_mount(&root_fs, "0:", 1);
            if (result == FR_OK) {
                break;
            }
            Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, !Chip_GPIO_GetPinState(LPC_GPIO, 0, 20));
            vTaskDelay(200);
            sdcard_retry_limit --;
        }
        if (sdcard_retry_limit == 0) {
            LOG_ERROR("SDCard Mount failed");
            exit_error(ERROR_CODE_SDCARD_MOUNT_FAILED);
        }

        Chip_GPIO_SetPinState(LPC_GPIO, 0, 20, false);
    }

    result = logging_init_persistent();
    if (result != 0) {
        exit_error(ERROR_CODE_SDCARD_LOGGING_INIT_FAILED);
    }

    LOG_INFO("Starting real tasks");

    mutex_i2c_1 = xSemaphoreCreateMutex();
    mutex_i2c_2 = xSemaphoreCreateMutex();

     // need this for H3L
    // initialize high G XL
    xSemaphoreTake(mutex_i2c_1, portMAX_DELAY);
    LOG_INFO("Initializing HGXL");
    if(H3L_init(I2C0, H3L_SCALE_200G, H3L_ODR_400)){
        H3L_enable();

        uint8_t int1_cfg = 0x80, int1_ths = 0, duration = 0, crt_fs = 0, crt_odr = 0;
        float hgax = 0, hgay = 0, hgaz = 0;
        hgax = H3L_read_accel_g(H3L_X);
        hgay = H3L_read_accel_g(H3L_Y);
        hgaz = H3L_read_accel_g(H3L_Z);

        // setup GPIO pin for intterupt

        // CRT_REG1
        // || pm2 || pm1 || pm0 || dr1 || dr0 || Zen || Yen || Xen ||
        crt_odr = (H3L_read_reg(H3L_CTRL_REG1) >> 3) & 0x3;
        // CRT_REG4
        // || bdu || ble || fs1 || fs0 || 0 || 0 || 0 || sim ||
        crt_fs = (H3L_read_reg(H3L_CTRL_REG4) >> 4) & 0x3;


        if(hgax > 1.0 || hgax < -1.0){
            int1_cfg |= (1<<1);
            hig_vert_axis = XAXIS;
        }else if(hgay > 1.0 || hgay < -1.0){
            int1_cfg |= (1<<3);
            hig_vert_axis = YAXIS;
        }else if(hgaz > 1.0 || hgaz < -1.0)
            int1_cfg |= (1<<5);
            hig_vert_axis = ZAXIS;
        }else{
            // error
        }

        // we want a 3'g detection
        // LSB's = floor(3000mg / sensitivity) Table 3 page 9
        if(crt_fs = H3L_SCALE_100G){
            int1_ths = 0x3c;
        }else if(crt_fs = H3L_SCALE_200G){
            int1_ths = 0x1e;
        }else if(crt_fs = H3L_SCALE_NA){
            // error
        }else if(crt_fs = H3L_SCALE_400G){
            int1_ths = 0x0f;
        }else{
            // error
        }

        // should the duration be left at 0 seconds?
        // trigger as soon as we seen 3g's
        // LSB's = floor(time in seconds * odr)
        if(crt_odr = H3L_ODR_50){
            // 1 LSB = 20ms
            duration = 0;
        }else if(crt_odr = H3L_ODR_100){
            // 1 LSB = 10ms
            duration = 0;
        }else if(crt_odr = H3L_ODR_400){
            // 1 LSB = 2.5ms
            duration = 0;
        }else if(crt_odr = H3L_ODR_1000){
            // 1 LSB = 1ms
            duration = 0;
        }else{
            // error
        }

        // setup launch interrupt
        H3L_configure_int_1(int1_cfg, int1_ths, duration); // 3 g's detected for 0 sec on n-axis - flight

        // blink LED
        // send bluetooth update
    }else{
        LOG_INFO("ERROR: H3L initization failure");
    }
    xSemaphoreGive(mutex_i2c_1);

    // initialize IMU
    xSemaphoreTake(mutex_i2c_1, portMAX_DELAY);
    LOG_INFO("Initializing IMU");
    if(LSM_init(I2C0, G_SCALE_2000DPS, A_SCALE_8G, M_SCALE_12GS, G_ODR_952, A_ODR_952, M_ODR_80)){
        // blink LED
        // send bluetooth update

        // LSM_calibrate(); // calibrate IMU

        // setup gyroscope interrupt (software)
        // find vert axis
        // vert spin (roll) is ok, that's what we want to measure on liftoff
        // large yaw or pitch spin could be a failed launch
        // if firing board is present, fire any chutes, shutdown booster channels
        float lgax = 0, lgay = 0, lgaz = 0;
        lgax = LSM_read_accel_g(LSM_ACCEL_X);
        lgay = LSM_read_accel_g(LSM_ACCEL_Y);
        lgaz = LSM_read_accel_g(LSM_ACCEL_Z);

        if(lgax > 1.0 || lgax < -1.0){
            imu_vert_axis = XAXIS;
        }else if(lgay > 1.0 || lgay < -1.0){
            imu_vert_axis = YAXIS;
        }else if(lgaz > 1.0 || lgaz < -1.0)
            imu_vert_axis = ZAXIS;
        }else{
            // error
        }

    }else{
        LOG_INFO("ERROR: L3G initization failure");
    }
    LOG_INFO("IMU initialized");
    xSemaphoreGive(mutex_i2c_1);

    // initialize baro sensor
    xSemaphoreTake(mutex_i2c_1, portMAX_DELAY);
    LOG_INFO("Initializing BARO");
    if(LPS_init(I2C0)){
        LPS_enable();
        // find baseline altitude altitude
        // maybe use GPS to find baseline altitude
        // look into using REF_REG's to calibrate
        baro_baseline = LPS_read_data(LPS_ALTITUDE);

        if(FIRE_present){ 
            // set up altitude interrupts for channels 1,2,3, and 4
        }
        // blink LED
        // send bluetooth update
    }else{
        LOG_INFO("ERROR: LPS initization failure");
    }
    xSemaphoreGive(mutex_i2c_1);

    // GPS detection
    xSemaphoreTake(mutex_i2c_2, portMAX_DELAY);
    GPS_present = gps_init(I2C1)
    xSemaphoreGive(mutex_i2c_2);

    // radio detection
    xSemaphoreTake(mutex_i2c_2, portMAX_DELAY);
    RADIO_present = Xbee_init(I2C1);
    xSemaphoreGive(mutex_i2c_2);

    // radio detection
    xSemaphoreTake(mutex_i2c_2, portMAX_DELAY);
    FIRE_present = Xbee_init(I2C1);
    xSemaphoreGive(mutex_i2c_2);


    // create setup task
    xTaskCreate(vtask_setup, "setup", 256, NULL, (tskIDLE_PRIORITY + 2), &boot_handle);
    // set_neopixel(PIXEL_ONE, GREEN);

    LOG_INFO("Finish: vtask_init has been suspended");
    vTaskSuspend(boot_handle);
}
    

void vtask_setup(void *pvParameters){
    LOG_INFO("Start: vtask_setup");

    xTaskCreate(vtask_read_gyro, "gyro" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    xTaskCreate(vtask_read_XL, "accel" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    xTaskCreate(vtask_read_HiG_XL, "hig_accel" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    xTaskCreate(vtask_read_mag, "mag" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    xTaskCreate(vtask_read_baro, "baro" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    xTaskCreate(vtask_read_temp, "temp" configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);

    // if(GPS_present){
    //     xTaskCreate(vtask_read_GPS, "gps", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    // }

    // if(RADIO_present){
    //     xTaskCreate(vtask_write_radio, "radio", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
    // }

/*
    if(FIRE_present){
        // should the neopixels be init by the ATtiny
        neo_pixel_init(); // setup neopixels on firing board and turn them red
        if(CHANNEL_ONE){
            xTaskCreate(vtask_channel_one, "channel_one", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
        }
        if(CHANNEL_TWO){
            xTaskCreate(vtask_channel_two, "channel_two", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
        }
        if(CHANNEL_THREE){
            xTaskCreate(vtask_channel_three, "channel_three", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
        }
        if(CHANNEL_FOUR){
            xTaskCreate(vtask_channel_four, "channel_four", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
        }
    }
*/
    // set_neo_pixel(PIXEL_TWO, GREEN);

    LOG_INFO("Finish: vtask_setup has been suspended");
    vTaskSuspend(boot_handle);
}

// 25-50hz
// detection of large spin on yaw or pitch should
// shut down any booster stages and deploy
// both main and drogue shuts
void vtask_read_gyro(void *pvParameters){
    static FIL f_gyro_log;
    static char gyro_str_buf[0x40];

    int result;
    int counter = 0;
    strcpy(gyro_str_buf, "GYRO.TAB");
    {
        int rename_number = 1;
        while(true) {
            if (f_stat(gyro_str_buf, NULL) == FR_OK) {
                sprintf(gyro_str_buf, "GYRO%d.TAB", rename_number);
                rename_number ++;
                continue;
            }
            break;
        }
    }
    LOG_INFO("GYRO output is %s", gyro_str_buf);
    result = f_open(&f_gyro_log, gyro_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
    for (;;) {
        float gx, gy, gz;
        xSemaphoreTake(mutex_i2c_1, portMAX_DELAY);
        gx = LSM_read_accel_g(LSM_GYRO_X);
        gy = LSM_read_accel_g(LSM_GYRO_Y);
        gz = LSM_read_accel_g(LSM_GYRO_Z);
        xSemaphoreGive(mutex_i2c_1);

        if (result == FR_OK) {
            sprintf(gyro_str_buf, "%d\t%.2f\t%.2f\t%.2f\n", xTaskGetTickCount(), gx, gy, gz);
            f_puts(gyro_str_buf, &f_gyro_log);
            if ((counter % 50) == 0) {
                f_sync(&f_gyro_log);
            }
        }

        if(apogee_detect){
            vTaskDelay(100); // 10hz
        }else{
            vTaskDelay(40); // 25hz
        }
        counter += 1;
    }
}

// start at apogee, 25-50hz
void vtask_read_XL(void *pParpvParametersam){
    static FIL f_xl_log;
    static char xl_str_buf[0x40];

    int result;
    int counter = 0;
    strcpy(xl_str_buf, "XL.TAB");
    {
        int rename_number = 1;
        while(true) {
            if (f_stat(xl_str_buf, NULL) == FR_OK) {
                sprintf(xl_str_buf, "XL%d.TAB", rename_number);
                rename_number ++;
                continue;
            }
            break;
        }
    }
    LOG_INFO("XL output is %s", xl_str_buf);
    result = f_open(&f_xl_log, xl_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
    for (;;) {
        float ax, ay, az;
        xSemaphoreTake(mutex_i2c_1, portMAX_DELAY);
        ax = LSM_read_accel_g(LSM_ACCEL_X);
        ay = LSM_read_accel_g(LSM_ACCEL_Y);
        az = LSM_read_accel_g(LSM_ACCEL_Z);
        xSemaphoreGive(mutex_i2c_1);

        if (result == FR_OK) {
            sprintf(xl_str_buf, "%d\t%.2f\t%.2f\t%.2f\n", xTaskGetTickCount(), ax, ay, az);
            f_puts(xl_str_buf, &f_xl_log);
            if ((counter % 50) == 0) {
                f_sync(&f_xl_log);
            }
        }

        if(apogee_detect){
            vTaskDelay(40); // 25hz
        }else{
            // no readings before apogee
        }
        counter += 1;
    }
}

// 25-50hz
void vtask_read_mag(void *pvParameters){
    static FIL f_mag_log;
    static char mag_str_buf[0x40];

    int result;
    int counter = 0;
    strcpy(mag_str_buf, "MAG.TAB");
    {
        int rename_number = 1;
        while(true) {
            if (f_stat(mag_str_buf, NULL) == FR_OK) {
                sprintf(mag_str_buf, "MAG%d.TAB", rename_number);
                rename_number ++;
                continue;
            }
            break;
        }
    }
    LOG_INFO("MAG output is %s", mag_str_buf);
    result = f_open(&f_mag_log, mag_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
    for (;;) {
        float mx, my, mz;
        xSemaphoreTake(mutex_i2c_1, portMAX_DELAY);
        mx = LSM_read_accel_g(LSM_MAG_X);
        my = LSM_read_accel_g(LSM_MAG_Y);
        mz = LSM_read_accel_g(LSM_MAG_Z);
        xSemaphoreGive(mutex_i2c_1);
            sprintf(mag_str_buf, "%d\t%.2f\t%.2f\t%.2f\n", mx, my, mz);
            f_puts(mag_str_buf, &f_mag_log);
            if ((counter % 50) == 0) {
                f_sync(&f_mag_log);
            }
        }

        if(apogee_detect){
            vTaskDelay(100); // 10hz
        }else{
            vTaskDelay(40); // 25hz
        }
        counter += 1;
    }    
}

// start at 400Hz, shut down at apogee, let LoG XL take over
void vtask_read_HiG_XL(void *pvParameters){
    static FIL f_hgxl_log;
    static char hgxl_str_buf[0x40];

    int result;
    int counter = 0;
    strcpy(hgxl_str_buf, "HGXL.TAB");
    {
        int rename_number = 1;
        while(true) {
            if (f_stat(hgxl_str_buf, NULL) == FR_OK) {
                sprintf(hgxl_str_buf, "HGXL%d.TAB", rename_number);
                rename_number ++;
                continue;
            }
            break;
        }
    }
    LOG_INFO("HGXL output is %s", hgxl_str_buf);
    result = f_open(&f_hgxl_log, hgxl_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
    for (;;) {
        float ax, ay, az;
        xSemaphoreTake(mutex_i2c_1, portMAX_DELAY);
        hgax = H3L_read_accel_g(H3L_X);
        hgay = H3L_read_accel_g(H3L_Y);
        hgaz = H3L_read_accel_g(H3L_Z);
        xSemaphoreGive(mutex_i2c_1);

        if (result == FR_OK) {
            sprintf(hgxl_str_buf, "%d\t%.2f\t%.2f\t%.2f\n", xTaskGetTickCount(), hgax, hgay, hgaz);
            f_puts(hgxl_str_buf, &f_hgxl_log);
            if ((counter % 50) == 0) {
                f_sync(&f_hgxl_log);
            }
        }

        if(apogee_detect){
            LOG_INFO("Finish: vtask_read_HiG_XL has been suspended");
            vTaskSuspend(vtask_read_HiG_XL);
        }else{
            vTaskDelay(40); // 25hz
        }
        counter += 1;
    }

}

// 25-50hz
void vtask_read_baro(void *pvParameters){
    static FIL f_baro_log;
    static char baro_str_buf[0x20];
    int result;
    int counter = 0;

    strcpy(baro_str_buf, "0:BARO.TAB");
    {
        int rename_number = 1;
        while(true) {
            if (f_stat(baro_str_buf, NULL) == FR_OK) {
                sprintf(baro_str_buf, "0:BARO%d.TAB", rename_number);
                rename_number ++;
                continue;
            }
            break;
        }
    }
    LOG_INFO("Baro output is %s", baro_str_buf);
    result = f_open(&f_baro_log, baro_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
    
    while (true) {
        float baro_alt;
        xSemaphoreTake(mutex_i2c_1, portMAX_DELAY);
        baro_alt = LPS_read_data(LPS_ALTITUDE) - baro_baseline;
        xSemaphoreGive(mutex_i2c_1);
        if (result == FR_OK) {
            sprintf(baro_str_buf, "%d\t%.2f\n", xTaskGetTickCount(), baro_alt);
            f_puts(baro_str_buf, &f_baro_log);
            if ((counter % 50) == 0) {
                f_sync(&f_baro_log);
            }
        }

        if(apogee_detect){
            vTaskDelay(100); // 10hz
        }else
            vTaskDelay(40); // 25z
        }
        counter ++;
    }
}

// 25-50hz
void vtask_read_temp(void *pvParameters){
    static FIL f_temp_log;
    static char temp_str_buf[0x20];
    int result;
    int counter = 0;

    strcpy(temp_str_buf, "0:TEMP.TAB");
    {
        int rename_number = 1;
        while(true) {
            if (f_stat(temp_str_buf, NULL) == FR_OK) {
                sprintf(temp_str_buf, "0:TEMP%d.TAB", rename_number);
                rename_number ++;
                continue;
            }
            break;
        }
    }
    LOG_INFO("TEMP output is %s", temp_str_buf);
    result = f_open(&f_temp_log, temp_str_buf, FA_WRITE | FA_CREATE_ALWAYS);
    
    while (true) {
        float baro_T, imu_T;
        xSemaphoreTake(mutex_i2c_1, portMAX_DELAY);
        baro_T = LPS_read_data(TEMPERATURE);
        imu_T = LSM_read_temperature_C(TEMPERATURE);
        xSemaphoreGive(mutex_i2c_1);
        if (result == FR_OK) {
            sprintf(temp_str_buf, "%d\t%.2f\t%.2f\n", xTaskGetTickCount(), baro_T, imu_T);
            f_puts(temp_str_buf, &f_temp_log);
            if ((counter % 50) == 0) {
                f_sync(&f_temp_log);
            }
        }

        if(apogee_detect){
            vTaskDelay(100); // 10hz
        }else
            vTaskDelay(40); // 25hz
        }
        counter ++;
    }
}

// 25-50hz
void vtask_GPS(void *pvParameters){
    // empty
}

void vtask_radio(void *pvParameters){
    // radio_send() // xl, baro, gyro data
    // radio_send() // gps data
    // empty
}

void vtask_interrupt_channel_one(void *pvParameters){
    // empty
    // drogue chute
    // set this neopixel to red
    // detect wiring
    // if present change neopixel to green
    // wait for interrupt
    // based on meters
    // fire -> send command to AT tiny
    // change neopixel to blue
}

void vtask_interrupt_channel_two(void *pvParameters){
    // empty
    // main chute
    // set this neopixel to red
    // detect wiring
    // if present change neopixel to green
    // wait for interrupt
    // based on meters
    // fire -> send command to AT tiny
    // change neopixel to blue
}

void vtask_interrupt_channel_three(void *pvParameters){
    // empty
    // set this neopixel to red
    // detect wiring
    // if present change neopixel to green
    // wait for interrupt
    // based on seconds
    // fire -> send command to AT tiny
    // change neopixel to blue
}

void vtask_interrupt_channel_four(void *pvParameters){
    // empty
    // set this neopixel to red
    // detect wiring
    // if present change neopixel to green
    // wait for interrupt
    // based on seconds
    // fire -> send command to AT tiny
    // change neopixel to blue
}

// set off by HiG Xl interrupt
void vtask_interrupt_launch(void *pvParameters){
    // this should be triggered by a hardware (high G XL) interrupt from GPIO pin
    // ? g's detectced for ? msec
    // empty
}

void vtask_interrupt_apogee(void *pvParameters){
    // high g xl detection getting colser to -1 g
    // trigger drogue chute (channel 1)
    // empty
}

// set off by combination of LoG XL and Baro
void vtask_interrupt_land(void *pvParameters){
    // baro sensor holding steady (somewhat - noise)
    // low g xl holding steady (somewhat - noise)
    // stop writing to flash
    // suspend sensor tasks (xl, baro, mag, gyro)
    // continue running BT, (radio, and GPS) (if present)
    // speed up calculation tasks
    // empty
}

void vtask_interrupt_shutdown(void *pvParameters){
    // should be called to shut down system and fire chutes (if present)
    // spin on pitch or yaw before apogee
    // failsafes
    // can be called over bluetooth and/or radio
}

void main(void) {
    prvSetupHardware();

    debug_uart_init();
    LOG_INFO("Initializing hardware");
    hardware_init();

    LOG_INFO("Starting tasks");

    // neo_pixel_init(); pass gpio pin and number of neopixels turn them red

    xTaskCreate(vtask_init, "init", 256, NULL, (tskIDLE_PRIORITY + 2), &boot_handle);  

    LOG_INFO("Tasks created; Starting scheduler");
    /* Start the scheduler */
    vTaskStartScheduler();

    // set_neopixel(PIXEL_ONE, RED);
    // set_neopixel(PIXEL_TWO, RED);

    exit_error(ERROR_CODE_MAIN_SCHEDULER_FALL_THRU);
    /* Should never arrive here */
    return 1;
}
