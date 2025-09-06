/**
 * @file crc.h
 * @brief Computes the crc based on modbus rtu
 *
 * @author Palaniappan Valliappan
 * @version 1.0
 * @date 202301111730 created - PV - based on Brandon's v4.11 of the driver board
 */

#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>

/**
 * @file crc.c
 * @brief Computes the crc based on modbus rtu
 *
 * @author Palaniappan Valliappan
 * @version 1.0
 * @date 202301111730 created - PV - based on Brandons v4.11 of the driver board
 */

uint16_t ModRTU_CRC(uint8_t buf[], int len);

#endif /* CRC_H_ */
