/*
 * spi1_os.c
 *
 *  Created on: Apr 10, 2014
 *      Author: Max Zhao
 * Changelog:
 * Apr. 13, 2014: Rewrite driver to use single byte transfer modes
 */

#include <a2fxxxm3.h>
#include <mss_pdma.h>
#include <mss_spi.h>
#include <stdlib.h>
#include "./spi1_os.h"
#include "os.h"

#define RX_FIFO_EMPTY_MASK      0x00000040u

SemaphoreHandle_t xSemSPI1TXComplete;
SemaphoreHandle_t xSemSPI1RXComplete;

static uint8_t* txBuffer;
static uint8_t* bufferEnd;
static uint8_t dummyByte;

#define TXDONE_IRQ_MASK         0x00000001u
#define CTRL_TX_IRQ_EN_MASK     0x00000020u

static bool sRestartTransmission(void);
#if defined(__GNUC__)
__attribute__((__interrupt__))
#endif
void SPI1_IRQHandler(void) {
	// Handle transfer

	if (!g_mss_spi1.hw_reg->MIS & TXDONE_IRQ_MASK) {
		// Did not disable interrupts properly.
		exit(15);
	}
	g_mss_spi1.hw_reg->IRQ_CLEAR = TXDONE_IRQ_MASK;
	NVIC_ClearPendingIRQ(SPI1_IRQn);

	if (!sRestartTransmission()) {
		// Done
		if (txBuffer != bufferEnd) {
			// Internal error!
			exit(16);
		}

		txBuffer = NULL;
		bufferEnd = NULL;

		BaseType_t wokeHighPriority = pdTRUE;
		// allow underlying task to wake faster.
		xSemaphoreGiveFromISR(xSemSPI1TXComplete, &wokeHighPriority);
		portYIELD_FROM_ISR(wokeHighPriority);
	}
}

static void sSPI1ReadCompleteHandler() {
	PDMA_clear_irq(PDMA_CHANNEL_2);
	xSemaphoreGiveFromISR(xSemSPI1RXComplete, NULL);
}

static void sSPI1EmptyReadHandler() {
	PDMA_clear_irq(PDMA_CHANNEL_1);
	xSemaphoreGiveFromISR(xSemSPI1RXComplete, NULL);
}

void spi1_init() {
	PDMA_configure
	(
		PDMA_CHANNEL_2,
		PDMA_FROM_SPI_1,
		PDMA_HIGH_PRIORITY | PDMA_BYTE_TRANSFER | PDMA_INC_DEST_ONE_BYTE,
		PDMA_DEFAULT_WRITE_ADJ
	);

	PDMA_configure
	(
		PDMA_CHANNEL_1,
		PDMA_FROM_SPI_1,
		PDMA_HIGH_PRIORITY | PDMA_BYTE_TRANSFER | PDMA_NO_INC,
		PDMA_DEFAULT_WRITE_ADJ
	);

	xSemSPI1TXComplete = xSemaphoreCreateBinary();
	xSemSPI1RXComplete = xSemaphoreCreateBinary();

	PDMA_set_irq_handler(PDMA_CHANNEL_2, sSPI1ReadCompleteHandler);
	PDMA_set_irq_handler(PDMA_CHANNEL_1, sSPI1EmptyReadHandler);
	PDMA_enable_irq(PDMA_CHANNEL_2);
	PDMA_enable_irq(PDMA_CHANNEL_1);

	NVIC_EnableIRQ(SPI1_IRQn);
}

static bool sRestartTransmission(void) {
	if (txBuffer >= bufferEnd) {
		return false;
	}

	// the critical section here is absolutely vital.
	// This took 7.5 hours to debug. Don't make this kind of mistake again.
	portENTER_CRITICAL();
	g_mss_spi1.hw_reg->TXRXDF_SIZE = 1;
	g_mss_spi1.hw_reg->TX_DATA = *(txBuffer ++);
	portEXIT_CRITICAL();
	return true;
}

void spi1_transmit_internal(void* buffer, int length, bool read) {
	if (!osSchedulerStarted) {
		exit(17);
	}

	if (txBuffer != NULL || bufferEnd != NULL) {
		exit(11);
	}

	// Clear SPI RX buffer to ensure nothing goes bad (unexpected write over)
	{
		// Code from mss_spi.c:422
		/* Flush Rx FIFO in case we are executing on A2F200. */
	    bool rx_fifo_empty = g_mss_spi1.hw_reg->STATUS & RX_FIFO_EMPTY_MASK;
	    while(!rx_fifo_empty)
	    {
	        uint32_t dummy = g_mss_spi1.hw_reg->RX_DATA;
	        dummy = dummy;  /* Prevent Lint warning. */
	        rx_fifo_empty = g_mss_spi1.hw_reg->STATUS & RX_FIFO_EMPTY_MASK;
	    }
	}

	// Always start the read process before write, so that no data is missed.
	if (xSemaphoreTake(xSemSPI1RXComplete, 0) == pdTRUE) {
		exit(12);
	}
	if (read) {
		// Clear semaphore
		PDMA_start(PDMA_CHANNEL_2, PDMA_SPI1_RX_REGISTER, (uintptr_t) buffer, length);
	} else {
		// Clear read buffer
		PDMA_start(PDMA_CHANNEL_1, PDMA_SPI1_RX_REGISTER, (uintptr_t) &dummyByte, length);
	}

	txBuffer = bufferEnd = buffer;
	bufferEnd += length;

	// Start the write process
	// Clear semaphore
	if (xSemaphoreTake(xSemSPI1TXComplete, 0) == pdTRUE) {
		exit(13);
	}

	// Enable TXComplete INT
	g_mss_spi1.hw_reg->IRQ_CLEAR = TXDONE_IRQ_MASK;
	NVIC_ClearPendingIRQ(SPI1_IRQn);
	g_mss_spi1.hw_reg->CONTROL |= CTRL_TX_IRQ_EN_MASK;

	if (!sRestartTransmission()) {
		// Length must be greater than or equal to 0
		exit(18);
	}

	if (xSemaphoreTake(xSemSPI1TXComplete, 1000 * portTICK_PERIOD_MS) != pdTRUE) {
		// failed to obtained semaphore within 1 second. Nope. Can't happen. May be interrupt wasn't enabled?
		exit(19);
	}

	if (txBuffer != NULL || bufferEnd != NULL) {
		exit(14);
	}

	if (xSemaphoreTake(xSemSPI1RXComplete, 1000 * portTICK_PERIOD_MS) != pdTRUE) {
		// failed to obtained semaphore within 1 second. Nope. Can't happen. May be interrupt wasn't enabled?
		exit(19);
	}
}
uint8_t spi1_transmit_byte(uint8_t data) {
	spi1_transmit_internal(&data, 1, true);
	return data;
}

void spi1_send_byte(uint8_t data) {
	spi1_transmit_internal(&data, 1, false);
}
