/*
 * usb.h
 *
 *	USB Initialization and management
 *
 *  Created on: Nov 16, 2014
 *      Author: Max Zhao
 */

#ifndef USB_H_
#define USB_H_

#include <chip.h>
#include <usbd_rom_api.h>

extern USBD_HANDLE_T g_hUsb;
bool usb_init(void);
bool usb_initialize_cdc_vcom(void);
void usb_connect();
void usb_disconnect();
bool usb_initialized(void);

void usb_enter(void);
void usb_exit(void);
void usb_init_freertos(void);




#endif /* USB_H_ */
