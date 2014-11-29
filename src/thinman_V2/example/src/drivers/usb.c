/*
 * usb.c

 *
 *  Created on: Nov 16, 2014
 *      Author: Max Zhao
 */
#include <FreeRTOS.h>
#include <semphr.h>
#include <string.h>
#include <chip.h>
#include <usbd_rom_api.h>
#include <usbd_core.h>
#include "logging.h"
#include "error_codes.h"
#include "./cdc_vcom.h"
#include "usb.h"

const  USBD_API_T *g_pUsbApi;
USBD_HANDLE_T g_hUsb = NULL;
static USBD_API_INIT_PARAM_T usb_param;
static USB_CORE_DESCS_T desc;

static xSemaphoreHandle mutex_usb_api;

void USB_IRQHandler(void)
{
	uint32_t *addr = (uint32_t *) LPC_USB->EPLISTSTART;

	/*	WORKAROUND for artf32289 ROM driver BUG:
	    As part of USB specification the device should respond
	    with STALL condition for any unsupported setup packet. The host will send
	    new setup packet/request on seeing STALL condition for EP0 instead of sending
	    a clear STALL request. Current driver in ROM doesn't clear the STALL
	    condition on new setup packet which should be fixed.
	 */
	if ( LPC_USB->DEVCMDSTAT & _BIT(8) ) {	/* if setup packet is received */
		addr[0] &= ~(_BIT(29));	/* clear EP0_OUT stall */
		addr[2] &= ~(_BIT(29));	/* clear EP0_IN stall */
	}
	USBD_API->hw->ISR(g_hUsb);
}

void usb_init_freertos(void) {
	mutex_usb_api = xSemaphoreCreateMutex();
}

bool usb_init(void) {
	ErrorCode_t ret = LPC_OK;
	usb_enter();
	Chip_USB_Init();
	usb_exit();
	g_pUsbApi = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

	{
		memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));

		usb_param.usb_reg_base = LPC_USB0_BASE;
		/*	WORKAROUND for artf44835 ROM driver BUG:
			Code clearing STALL bits in endpoint reset routine corrupts memory area
			next to the endpoint control data. For example When EP0, EP1_IN, EP1_OUT,
			EP2_IN are used we need to specify 3 here. But as a workaround for this
			issue specify 4. So that extra EPs control structure acts as padding buffer
			to avoid data corruption. Corruption of padding memory doesn’t affect the
			stack/program behaviour.
		 */
		usb_param.max_num_ep = 3 + 1;
		usb_param.mem_base = USB_STACK_MEM_BASE;
		usb_param.mem_size = USB_STACK_MEM_SIZE;

		usb_param.max_num_ep = 3 + 1;
		usb_param.mem_base = USB_STACK_MEM_BASE;
		usb_param.mem_size = USB_STACK_MEM_SIZE;

		/* Set the USB descriptors */
		desc.device_desc = (uint8_t *) &USB_DeviceDescriptor[0];
		desc.string_desc = (uint8_t *) &USB_StringDescriptor[0];
		/* Note, to pass USBCV test full-speed only devices should have both
		   descriptor arrays point to same location and device_qualifier set to 0.
		 */
		desc.high_speed_desc = (uint8_t *) &USB_FsConfigDescriptor[0];
		desc.full_speed_desc = (uint8_t *) &USB_FsConfigDescriptor[0];
		desc.device_qualifier = 0;

		/* USB Initialization */
		usb_enter();
		ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
		usb_exit();

		if (ret == LPC_OK) {

			/*	WORKAROUND for artf32219 ROM driver BUG:
			    The mem_base parameter part of USB_param structure returned
			    by Init() routine is not accurate causing memory allocation issues for
			    further components.
			 */
			usb_param.mem_base = USB_STACK_MEM_BASE + (USB_STACK_MEM_SIZE - usb_param.mem_size);
			return true;
		}
	}
	LOG_ERROR("USB Initialization failed with error code %d", ret);
	return false;
}

void usb_enter(void) {
	if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) return;
	xSemaphoreTake(mutex_usb_api, portMAX_DELAY);
}

void usb_exit(void) {
	if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) return;
	xSemaphoreGive(mutex_usb_api);
}

bool usb_initialize_cdc_vcom() {
	ErrorCode_t err;
	if (!usb_initialized()) {
		exit_error(ERROR_CODE_USB_NOT_INITIALIZED);
	}

	if ((err = vcom_init(g_hUsb, &desc, &usb_param)) == LPC_OK) {
		return true;
	}

	LOG_ERROR("USB CDC device failed with error code %d", err);
	return false;
}

void usb_connect() {
	usb_enter();
	NVIC_EnableIRQ(USB0_IRQn);
	/* now connect */
	USBD_API->hw->Connect(g_hUsb, 1);
	usb_exit();
}

void usb_disconnect() {
	usb_enter();
	USBD_API->hw->Connect(g_hUsb, 0);

	NVIC_DisableIRQ(USB0_IRQn);
	usb_exit();
}

bool usb_initialized() {
	return g_hUsb != NULL;
}

/* Find the address of interface descriptor for given class type. */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
	USB_COMMON_DESCRIPTOR *pD;
	USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
	uint32_t next_desc_adr;

	pD = (USB_COMMON_DESCRIPTOR *) pDesc;
	next_desc_adr = (uint32_t) pDesc;

	while (pD->bLength) {
		/* is it interface descriptor */
		if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

			pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
			/* did we find the right interface descriptor */
			if (pIntfDesc->bInterfaceClass == intfClass) {
				break;
			}
		}
		pIntfDesc = 0;
		next_desc_adr = (uint32_t) pD + pD->bLength;
		pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
	}

	return pIntfDesc;
}

