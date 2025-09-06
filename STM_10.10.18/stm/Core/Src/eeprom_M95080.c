/*
 * eeprom_M95080.c
 *
 *  Created on: Jul 6, 2023
 *      Author: ANB
 */

#include "eeprom_M95080.h"

void eeprom_write_byte(eeprom_addr_t address, uint8_t value)
{
	uint8_t txData[5];

	txData[0] = 0x06;                   // Write enable command
	txData[1] = 0x02;                   // Write command
	txData[2] = (address >> 8) & 0xFF;  // MSB of address
	txData[3] = address & 0xFF;         // LSB of address
	txData[4] = value;                   // Data to be written

	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_LOW);
	spi2_send_byte(txData[0]); //first send write enable separately from main write sequence
	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_HIGH);

	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_LOW);
	for (int i = 1; i < 5; i++)
		spi2_send_byte(txData[i]);
	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_HIGH);

	eeprom_wait_for_write_to_complete();
}

void eeprom_write_byte_with_readback_check(eeprom_addr_t address, uint8_t value)
{
	int max_tries = 5;
	uint8_t readback;

	readback = eeprom_read_byte(address);
	if(readback == value) //no need to write, save aging the memory location
		return;

	for(int i = 0; 1 < max_tries; i++)
	{
		eeprom_write_byte(address, value);
		readback = eeprom_read_byte(address);
		if(readback == value) //good news, it matches
			break;
	}
}

uint8_t eeprom_read_byte(eeprom_addr_t address)
{
	uint8_t txData[4];

	txData[0] = 0x03;                   // Read command
	txData[1] = (address >> 8) & 0xFF;  // MSB of address
	txData[2] = address & 0xFF;         // LSB of address

	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_LOW);

	for (int i = 0; i < 3; i++)
	{
		spi2_send_byte(txData[i]);
	}

	uint8_t result = spi2_get_byte();

	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_HIGH);

	return result;
}

void eeprom_erase_all(void)
{
	uint8_t txData[EEPROM_PAGE_SIZE];;
	eeprom_addr_t page_address;

	for(int i = 0; i < EEPROM_PAGE_SIZE; i++)
		txData[i] = 0xff;

	for(page_address = EEPROM_ADDR; page_address < EEPROM_SIZE; page_address = page_address + EEPROM_PAGE_SIZE)
		eeprom_write_buffer_with_readback_check(page_address, txData, EEPROM_PAGE_SIZE);
}

void eeprom_write_buffer(eeprom_addr_t address, uint8_t *buffer, uint8_t buffer_length)
{
	uint8_t txData[4];

	txData[0] = 0x06;                   // Write enable command
	txData[1] = 0x02;                   // Write command
	txData[2] = (address >> 8) & 0xFF;  // MSB of address
	txData[3] = address & 0xFF;         // LSB of address

	//first send write enable separately from main write sequence
	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_LOW);
	spi2_send_byte(txData[0]);
	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_HIGH);

	//send command
	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_LOW);
	for (int i = 1; i < 4; i++)
		spi2_send_byte(txData[i]);

	//send data
	spi2_send_bytes(buffer_length, buffer, SPI_PHASE_1EDGE);
	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_HIGH);

	eeprom_wait_for_write_to_complete();
}

void eeprom_write_buffer_with_readback_check(eeprom_addr_t address, uint8_t *buffer, uint8_t buffer_length)
{
	int max_tries = 5;
	uint8_t readback;
	bool failed;

	for(int i = 0; i < max_tries; i++)
	{
		failed = false;
		eeprom_write_buffer(address, buffer, buffer_length);
		for(int j = 0; j < buffer_length; j++)
		{
			readback = eeprom_read_byte(address + j);
			if(readback != *(buffer + j))
			{
				failed = true;
				break; //proceed to next try
			}
		}
		if(!failed)
			break; //if we got here then all bytes matched. exit try loop.
	}
}

void eeprom_wait_for_write_to_complete(void)
{
	spi2_wait_for_eeprom_write_to_complete();
}
