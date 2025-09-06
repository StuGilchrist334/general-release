/*
 * eeprom_M95080.h
 *
 *  Created on: Jul 6, 2023
 *      Author: ANB
 */

#ifndef EEPROM_M95080_H_
#define EEPROM_M95080_H_

#include "stdint.h"
#include "spi_device.h"

#define EEPROM_PAGE 0
#define EEPROM_PAGE_SIZE 32
#define EEPROM_ADDR EEPROM_PAGE * EEPROM_PAGE_SIZE
#define EEPROM_SIZE 1024

typedef uint16_t eeprom_addr_t;

uint8_t eeprom_read_byte(eeprom_addr_t addr);
void eeprom_write_byte(eeprom_addr_t address, uint8_t value);
void eeprom_write_byte_with_readback_check(eeprom_addr_t address, uint8_t value);
void eeprom_erase_all(void);
void eeprom_write_buffer(eeprom_addr_t address, uint8_t * buffer_start_address, uint8_t buffer_length);
void eeprom_write_buffer_with_readback_check(eeprom_addr_t address, uint8_t * buffer_start_address, uint8_t buffer_length);
void eeprom_wait_for_write_to_complete(void);

#endif /* EEPROM_M95080_H_ */
