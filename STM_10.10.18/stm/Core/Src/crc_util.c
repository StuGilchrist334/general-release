/**
 * @file crc_util.c
 * @brief Implements crc utility functions used in this project.
 *
 * @author Palaniappan Valliappan
 * @version 1.0
 * @date 202301181034 created - PV
 */

#include "crc_util.h"

crc_util_struct crc_global;

/**
* @brief Initialises the default values for crc.
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202301181043 created - PV
*/
void crc_util_initialise()
{
	crc_global.crc_calculated = false;
	crc_global.last_crc_value = 0;
	crc_global.crc_util_initialised_flag = true;
}

/**
* @brief Initialises the default values for crc.
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202301181044 created - PV
*/
uint16_t crc_calculate(uint8_t array_of_bytes[], int length)
{
	if (!crc_global.crc_util_initialised_flag)
	{
		// if not initialsed, do it
		crc_util_initialise();
	}
	crc_global.last_crc_value = ModRTU_CRC(array_of_bytes, length);
	// reversed for transmission
	crc_global.last_crc_reversed_first_byte = crc_global.last_crc_value & 0xff;
	crc_global.last_crc_reversed_second_byte = crc_global.last_crc_value >> 8;
	crc_global.crc_calculated = true;
	return crc_global.last_crc_value;
}

/**
* @brief Calculates and returns first reversed crc byte of the input array.
*
* @return reversed first byte of crc
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202301181044 created - PV
*/
uint8_t crc_get_reversed_first_byte(uint8_t array_of_bytes[], int length)
{
	crc_calculate(array_of_bytes, length);
	return crc_global.last_crc_reversed_first_byte;
}

/**
* @brief If the crc value has been already calculated returns the second reversed crc byte.
*
* The crc value is of the input array passed to the get first byte function.
*
* @return reversed second byte of crc
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202301181050 created - PV
*/
uint8_t crc_get_reversed_second_byte()
{
	if (crc_global.crc_calculated)
	{
		return crc_global.last_crc_reversed_second_byte;
		crc_global.crc_calculated = false;
	}
	else
	{
		return 0;
	}
}
