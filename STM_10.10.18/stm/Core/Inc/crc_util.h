/**
 * @file crc_util.h
 * @brief Implements crc utility functions used in this project.
 *
 * @author Palaniappan Valliappan
 * @version 1.0
 * @date 202301181033 created - PV
 */

#ifndef CRC_UTIL_H_
#define CRC_UTIL_H_

#include "stdbool.h"
#include "crc.h"

typedef struct
{
	bool crc_util_initialised_flag;
	bool crc_calculated;		// set to true on conversion & first byte read function is called, set to false once the second byte is read
	uint16_t last_crc_value;
	uint8_t last_crc_reversed_first_byte;
	uint8_t last_crc_reversed_second_byte;
} crc_util_struct;

/**
* @brief Initialises the default values for crc.
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202301181043 created - PV
*/
void crc_util_initialise(void);

/**
* @brief Initialises the default values for crc.
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202301181044 created - PV
*/
uint16_t crc_calculate(uint8_t array_of_bytes[], int length);

/**
* @brief Calculates and returns first reversed crc byte of the input array.
*
* @return reversed first byte of crc
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202301181044 created - PV
*/
uint8_t crc_get_reversed_first_byte(uint8_t array_of_bytes[], int length);

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
uint8_t crc_get_reversed_second_byte(void);

#endif /* CRC_UTIL_H_ */
