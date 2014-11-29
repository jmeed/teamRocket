/*
 * crc.h
 *
 *  Created on: Apr 18, 2014
 *      Author: Max Zhao
 */

#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>
#include <stddef.h>
uint8_t crc_crc7(const void* buffer, size_t length);
uint16_t crc_crc16(const void* buffer, size_t length);

#endif /* CRC_H_ */
