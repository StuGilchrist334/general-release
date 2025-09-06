/**
 * @file crc.c
 * @brief Computes the crc based on modbus rtu
 *
 * @author Palaniappan Valliappan
 * @version 1.0
 * @date 202301111730 created - PV - based on Brandon's v4.11 of the driver board
 */

#include "crc.h"

uint16_t ModRTU_CRC(uint8_t buf[], int length)
{
	uint16_t crc = 0xFFFF;
	
	for (int pos = 0; pos < length; pos++)
	{
		crc ^= (uint16_t) buf[pos];			// XOR byte into least sig. byte of crc
		
		for (int i = 8; i != 0; i--)
		{
			// Loop over each bit
			if ((crc & 0x0001) != 0)
			{
				// If the LSB is set
				crc >>= 1;					// Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else
			{
				// Else LSB is not set
				crc >>= 1;					// Just shift right
			}
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
	return crc;
}
