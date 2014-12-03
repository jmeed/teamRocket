/*
 * spi.c
 *
 *  Created on: Oct 22, 2014
 *      Author: Max Zhao
 */
#include <FreeRTOS.h>
#include "semphr.h"
#include "logging.h"
#include "spi.h"
#include "chip.h"

spi_device_t spi_devices[SPI_DEVICE_COUNT];

void spi_init() {
	int i;
	spi_devices[0].ssp_device = LPC_SSP0;
	spi_devices[1].ssp_device = LPC_SSP1;
	for (i = 0; i < sizeof(spi_devices) / sizeof(spi_devices[0]); i++) {
		spi_devices[i].mutex = xSemaphoreCreateMutex();
		vSemaphoreCreateBinary(spi_devices[i].sem_ready);
		xSemaphoreTake(spi_devices[i].sem_ready, 0);
	}
	NVIC_EnableIRQ(SSP0_IRQn);
	NVIC_EnableIRQ(SSP1_IRQn);
}

void spi_setup_device(spi_device_t* device, uint32_t bits, uint32_t frameFormat, uint32_t clockMode, bool master) {
	Chip_SSP_Init(device->ssp_device);
	Chip_SSP_SetMaster(device->ssp_device, master);
	Chip_SSP_SetFormat(device->ssp_device, bits, frameFormat, clockMode);
	spi_enable(device);
}

void spi_enable(spi_device_t* device) {
	Chip_SSP_Enable(device->ssp_device);
}

void spi_disable(spi_device_t* device) {
	Chip_SSP_Disable(device->ssp_device);
}

void spi_set_bit_rate(spi_device_t* device, uint32_t bit_rate) {
	Chip_SSP_SetBitRate(device->ssp_device, bit_rate);
}


static void spi_interrupt_transceive(spi_device_t* device) {
	Chip_SSP_DATA_SETUP_T* setup = &device->xf_setup;
	Chip_SSP_Int_Disable(device->ssp_device);	/* Disable all interrupt */
	Chip_SSP_Int_RWFrames8Bits(device->ssp_device, setup);

	if ((setup->rx_cnt != setup->length) || (setup->tx_cnt != setup->length)) {
		Chip_SSP_Int_Enable(device->ssp_device);	/* enable all interrupts */
	}
	else {
		xSemaphoreGiveFromISR(device->sem_ready, NULL);
		device->ready = true;
	}
}

void SSP0_IRQHandler(void) {
	 spi_interrupt_transceive(&spi_devices[0]);
}

void SSP1_IRQHandler(void) {
	 spi_interrupt_transceive(&spi_devices[1]);
}

static void spi_transceive_internal(spi_device_t* device, uint8_t* read_buffer, const uint8_t* write_buffer, size_t size) {
	if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
		xSemaphoreTake(device->mutex, portMAX_DELAY);
	device->xf_setup.rx_data = read_buffer;
	device->xf_setup.tx_data = (void*) write_buffer;
	device->xf_setup.length = size;
	device->xf_setup.rx_cnt = 0;
	device->xf_setup.tx_cnt = 0;

	Chip_SSP_Int_FlushData(device->ssp_device);
	Chip_SSP_Int_RWFrames8Bits(device->ssp_device, &device->xf_setup);
	Chip_SSP_Int_Enable(device->ssp_device);

	// Wait till done

	device->ready = false;

	if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
		xSemaphoreTake(device->sem_ready, portMAX_DELAY);
	} else {
		while (!device->ready) {
			Chip_GPIO_SetPinToggle(LPC_GPIO, 0, 20);
		}
	}

	if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
		xSemaphoreGive(device->mutex);
}

void spi_transceive(spi_device_t* device, uint8_t* buffer, size_t size) {
	spi_transceive_internal(device, buffer, buffer, size);
}

void spi_send(spi_device_t* device, const uint8_t* buffer, size_t size) {
	spi_transceive_internal(device, NULL, buffer, size);
}

uint8_t spi_transceive_byte(spi_device_t* device, uint8_t b) {
	uint8_t buffer[1];
	buffer[0] = b;
	spi_transceive(device, buffer, 1);
	return buffer[0];
}

void spi_receive(spi_device_t* device, uint8_t* buffer, size_t size) {
	spi_transceive_internal(device, buffer, NULL, size);
}
