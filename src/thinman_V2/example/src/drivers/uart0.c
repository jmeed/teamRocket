#include <FreeRTOS.h>
#include <stdlib.h>
#include <task.h>
#include <string.h>
#include "./uart0.h"
#include "semphr.h"
#include "logging.h"
#include "error_codes.h"

static bool uart_ready = false;
static xSemaphoreHandle sem_uart_ready;
static xSemaphoreHandle sem_uart_read_ready;
static xSemaphoreHandle mutex_uart_in_use;
static xSemaphoreHandle mutex_uart_read_in_use;

static RINGBUFF_T txring, rxring;
static uint8_t rxbuff[UART0_READ_RB_SIZE], txbuff[UART0_WRITE_RB_SIZE];

static void Init_UART0_PinMux(void)
{
#if defined(BOARD_MANLEY_11U68)
	/* UART signals on pins PIO0_18 (FUNC1, U0_TXD) and PIO0_19 (FUNC1, U0_RXD) */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));

#elif defined(BOARD_NXP_LPCXPRESSO_11U68)
	/* UART signals on pins PIO0_18 (FUNC1, U0_TXD) and PIO0_19 (FUNC1, U0_RXD) */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));

#else
	/* Configure your own UART pin muxing here if needed */
#warning "No UART pin muxing defined"
#endif
}

void uart0_init() {
	Init_UART0_PinMux();
	vSemaphoreCreateBinary(sem_uart_ready);
	vSemaphoreCreateBinary(sem_uart_read_ready);
	mutex_uart_in_use = xSemaphoreCreateMutex();
	mutex_uart_read_in_use = xSemaphoreCreateMutex();

	// FreeRTOS craziness!!!
	xSemaphoreTake(sem_uart_ready, 0);
	xSemaphoreTake(sem_uart_read_ready, 0);
	uart_ready = false;
}

void uart0_setup(uint32_t baudrate, uint32_t config) {
	/* Initialize UART0 */
	Chip_UART0_Init(LPC_USART0);
	Chip_UART0_SetBaud(LPC_USART0, baudrate);
	Chip_UART0_ConfigData(LPC_USART0, config);
	Chip_UART0_SetupFIFOS(LPC_USART0, (UART0_FCR_FIFO_EN | UART0_FCR_TRG_LEV2));
	Chip_UART0_TXEnable(LPC_USART0);

	RingBuffer_Init(&rxring, rxbuff, 1, UART0_READ_RB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART0_WRITE_RB_SIZE);

	Chip_UART0_IntEnable(LPC_USART0, (UART0_IER_RBRINT | UART0_IER_RLSINT));
	NVIC_EnableIRQ(USART0_IRQn);
}

void USART0_IRQHandler(void)
{
	Chip_UART0_IRQRBHandler(LPC_USART0, &rxring, &txring);
	if (RingBuffer_IsEmpty(&txring)) {
		xSemaphoreGiveFromISR(sem_uart_ready, NULL);
	}
	if (!RingBuffer_IsEmpty(&rxring)) {
		xSemaphoreGiveFromISR(sem_uart_read_ready, NULL);
	}
}

/**
 * @remarks: behavior changed to block only when txring buffer is full (prior to return)
 * Guarantees that everything is written (thus retries whenever necessary)
 */
static void uart0_write_internal(const uint8_t* data, size_t size, bool in_rtos) {
	uint32_t written;
	while (size > 0) {
		written = Chip_UART0_SendRB(LPC_USART0, &txring, data, size);
		data += written;
		size -= written;

		if (in_rtos && RingBuffer_IsFull(&txring) && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
			xSemaphoreTake(sem_uart_ready, portMAX_DELAY);
		}
	}
}

/**
 * Reads as much as possible from the USART buffer
 * Guarantees reading at least one byte. If none present, blocks until at least one byte is received.
 */
size_t uart0_read(char* buf, size_t size) {
	xSemaphoreTake(mutex_uart_read_in_use, portMAX_DELAY);

	uint32_t read = 0;

	while (read == 0) {
		read += Chip_UART0_ReadRB(LPC_USART0, &rxring, buf, size);
		size -= read;
		buf += read;

		// Nothing read, wait
		if (RingBuffer_IsEmpty(&rxring) && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
			xSemaphoreTake(sem_uart_read_ready, portMAX_DELAY);
		}
	}

	xSemaphoreGive(mutex_uart_read_in_use);
	return read;
}

int uart0_readchar() {
	char b;
	uart0_read(&b, 1);
	return b;
}

void uart0_write(const uint8_t* data, size_t size) {
	if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
		uart0_write_internal(data, size, false);
	} else {
		xSemaphoreTake(mutex_uart_in_use, portMAX_DELAY);
		// locked
		uart0_write_internal(data, size, true);
		xSemaphoreTake(sem_uart_ready, portMAX_DELAY);

		xSemaphoreGive(mutex_uart_in_use);
	}
}

void uart0_write_string(const char* str) {
	uart0_write((const uint8_t*) str, strlen(str));
}

void uart0_write_critical(const uint8_t* data, size_t size) {
	uart0_write_internal(data, size, false);
}

void uart0_write_string_critical(const char* str) {
	uart0_write_critical((const uint8_t*) str, strlen(str));
}
