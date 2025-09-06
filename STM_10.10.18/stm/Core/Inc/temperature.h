/**
 * @file temperature.h
 * @brief Temperature measurement from the array for Mark IV sensor.
 *
 * @author Palaniappan Valliappan
 * @version 1.2
 * @date 202301211948 created - PV
 * @date 202301xxxxxx modified - BM - added delay before the temperature is read
 * @date 202302151205 modified - PV - added comments to previous change
 */


#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include "subsystem.h"
#include "eeprom_address_definition.h"
#include "globals.h"
#include "math.h"
#include "stdlib.h"
#include "usart_commsboard.h"

#define TEMPERATURE_DELAY_MS_BEFORE_READING_ADC_AFTER_SWITCHING 10

#define SLOPE_OF_TEMPERATURE_READINGS -110360
#define INTERCEPT_OF_TEMPERATURE_READINGS 6750400

#define MAX_GAIN_RESISTANCE 246880            // Maximum feedback resistance in gain control amplifier
#define VP_TEMPERATURE_IP_RESISTANCE 511000   // R2 on 0920507 Valeport PCB (input resistor to gain control amplifier)

#define VP_TEMPERATURE_R3 68100              // R3 on 0920507 Valeport PCB
#define VT3 0.5120                          // Voltage at T3 on VP temperature board, set by voltage regulator.
#define BETA 4171                            // BETA Value for DC95G104VN
#define RT0 100000                           // Resistance of DC95G104VN at T0 (25 deg C)
#define T0 298.15                            // Temperature in Kelvin at T0 (25 deg C)
#define KELVINTOC 273.15                     // Temperature difference between Kelvin and C

int32_t get_temperature(uint8_t a_v);

double calculate_temperature(int32_t adc_value);

void get_calibrated_m_value(void);

void get_calibrated_c_value(void);

double VP_calculate_temperature(int32_t adc_value);

typedef struct
{
	double adc;
	double r1;
	double r2;
	double gr_ipr;
	double t2;
	double rt;
	double tk;
	double tc;
}vp_temp;


#endif /* TEMPERATURE_H_ */
