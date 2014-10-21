#include <FreeRTOS.h>
#include <stdlib.h>
#include <task.h>
#include <string.h>
#include "./uart0.h"
#include "semphr.h"
#include "logging.h"
#include "error_codes.h"

static uint8_t uartHandleMEM[0x40];
static UART_HANDLE_T* uartHandle = NULL;
static bool uart_ready = false;
static xSemaphoreHandle sem_uart_ready;
static xSemaphoreHandle mutex_uart_in_use;

static void uart0_hard_fault() {
	LOG_ERROR("Failed to initialize UART0!");
	exit_error(ERROR_CODE_DEBUG_UART_FAILED);
}

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
	mutex_uart_in_use = xSemaphoreCreateMutex();

	// FreeRTOS craziness!!!
	xSemaphoreTake(sem_uart_ready, 0);
	uart_ready = false;
	NVIC_EnableIRQ(USART0_IRQn);
}

void uart0_setup(uint32_t baudrate, uint32_t config) {
	int errCode;
	UART_CONFIG_T cfg = {
		0,				/* U_PCLK frequency in Hz */
		baudrate,		/* Baud Rate in Hz */
		config,				/* 8N1 */
		0,				/* Asynchronous Mode */
		NO_ERR_EN	/* Enable No Errors */
	};

	/* Initialize UART0 */
	Chip_UART0_Init(LPC_USART0);

	/* Perform a sanity check on the storage allocation */
	if (LPC_UART0D_API->uart_get_mem_size() > sizeof(uartHandleMEM)) {
		/* Example only: this should never happen and probably isn't needed for
		   most UART code. */
		uart0_hard_fault();
	}

	/* Setup the UART handle */
	uartHandle = LPC_UART0D_API->uart_setup((uint32_t) LPC_USART0, (uint8_t *) &uartHandleMEM);
	if (uartHandle == NULL) {
		uart0_hard_fault();
	}

	/* Need to tell UART ROM API function the current UART peripheral clock
		 speed */
	cfg.sys_clk_in_hz = Chip_Clock_GetSystemClockRate();

	/* Initialize the UART with the configuration parameters */
	errCode = LPC_UART0D_API->uart_init(uartHandle, &cfg);
	if (errCode != LPC_OK) {
		/* Some type of error handling here */
		uart0_hard_fault();
	}
}

static void uart0_callback(uint32_t err_code, uint32_t n) {
	uart_ready = true;
	portBASE_TYPE woken = pdFALSE;
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
		xSemaphoreGiveFromISR(sem_uart_ready, &woken);
	}
	// portYIELD_FROM_ISR(woken);
}

void USART0_IRQHandler(void)
{
	if (uartHandle) {
		LPC_UART0D_API->uart_isr(uartHandle);
	}
}

static void uart0_write_internal(const uint8_t* data, size_t size, bool in_rtos) {
	UART_PARAM_T param;

	param.buffer = (uint8_t *) data;
	param.size = size;

	/* Interrupt mode, do not append CR/LF to sent data */
	param.transfer_mode = TX_MODE_BUF_EMPTY;
	if (!in_rtos)
		param.driver_mode = DRIVER_MODE_POLLING;
	else
		param.driver_mode = DRIVER_MODE_INTERRUPT;

	/* Setup the transmit callback, this will get called when the
	   transfer is complete */
	param.callback_func_pt = (UART_CALLBK_T) uart0_callback;

	/* Transmit the data using interrupt mode, the function will
	   return */
	uart_ready = false;
	int error_code = LPC_UART0D_API->uart_put_line(uartHandle, &param);
	if (error_code) {
		if (in_rtos) {
			LOG_CRITICAL("CRITICAL: UART0 failed to send string with length %d", size);
			exit_error(ERROR_CODE_DEBUG_UART_FAILED_SEND * 10 + (error_code & 0xf));
		} else {
			exit_error(ERROR_CODE_DEBUG_UART_FAILED_SEND_CRITICAL * 10 + (error_code & 0xf));
		}
	}
}

void uart0_write(const uint8_t* data, size_t size) {
	if (uartHandle) {
		if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
			uart0_write_internal(data, size, false);
		} else {
			if (!xSemaphoreTake(mutex_uart_in_use, 2000)) {
				exit_error(24);
			}

			// locked
			uart0_write_internal(data, size, true);

			if (!xSemaphoreTake(sem_uart_ready, 2000)) {
				exit_error(23);
			}

			xSemaphoreGive(mutex_uart_in_use);
		}
	}
}

void uart0_write_string(const char* str) {
	uart0_write((const uint8_t*) str, strlen(str));
}

void uart0_write_critical(const uint8_t* data, size_t size) {
	if (!uartHandle) {
		exit_error(ERROR_CODE_DEBUG_UART_NOT_READY_FOR_CRITICAL);
	}
	uart0_write_internal(data, size, false);
}

void uart0_write_string_critical(const char* str) {
	uart0_write_critical((const uint8_t*) str, strlen(str));
}
