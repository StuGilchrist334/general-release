/**
 * @file temperature.c
 * @brief Temperature measurement from the array for Mark IV sensor.
 *
 * @author Palaniappan Valliappan
 * @version 1.2
 * @date 202301211942 created - PV
 * @date 202301xxxxxx modified - BM - added delay before the temperature is read
 * @date 202302151205 modified - PV - added comments to previous change
 */

#include "temperature.h"


volatile vp_temp vt;
int16_t temperature_adc, temperature_adc_range;
/**
* @brief Read the adc after the temperature sensor is enabled.
*
* @author Palaniappan Valliappan
* @version v1.2
* @date 202212141510 created - PV
* @date 202301xxxxxx modified - BM - added delay before the temperature is read
* @date 202302151205 modified - PV - added comments to previous change
*/
int32_t get_temperature(uint8_t a_v)
{
	int32_t retval, adc_current, adc_min = 0xffffff, adc_max = 0;
	uint64_t sum = 0;
	int32_t num_samples = 1000; //was 10000.

	disable_afe_all_gain_resistors();

	// enable the temperature channel selection
	enable_temperature_switch();
	
	// wait for the specified ms for the temperature read value to be valid	after powering on
	HAL_Delay(TEMPERATURE_DELAY_MS_BEFORE_READING_ADC_AFTER_SWITCHING);

	if (a_v == COMMAND_OPTION_VALEPORT_HEAD)
	{
		enable_afe_all_gain_resistors();
		HAL_Delay(1000);
		retval = adc_convert_manipulate_vp();
	}
	else
	{
		sum = 0;
		// call ADC to get measurements
		for(int h = 0; h < num_samples; h++)
		{
			adc_current = adc_convert_manipulate();
			sum += adc_current;
			if(adc_current < adc_min)
				adc_min = adc_current;
			if(adc_current > adc_max)
				adc_max = adc_current;
		}

		retval = sum / num_samples;
	}

	temperature_adc = retval / 1000;
	temperature_adc_range = abs(adc_max - adc_min);

	// disable temperature channel
	disable_sensor_electrodes();
	
	return retval;
}

/**
* @brief Convert data from ADC to temperature.
*
* @author rr
* @version v1.0
* @date 2023022023 created - rr
*/
double calculate_temperature(int32_t adc_value)
{
	int i;

	uint8_t default_eeprom[DEVICE_TEMPERATURE_CALIBRATION_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t calibration_array_1[DEVICE_TEMPERATURE_CALIBRATION_LENGTH];
	uint8_t calibration_array_2[DEVICE_TEMPERATURE_CALIBRATION_LENGTH];
	uint8_t calibration_array_3[DEVICE_TEMPERATURE_CALIBRATION_LENGTH];
	uint8_t calibration_array_validated[DEVICE_TEMPERATURE_CALIBRATION_LENGTH];

	bool one_matches_two = true;
	bool one_matches_three = true;
	bool two_matches_three = true;
	bool write_defaults = false;

	for(i = 0; i < DEVICE_TEMPERATURE_CALIBRATION_LENGTH; i++)
	{
		calibration_array_1[i] = eeprom_read_byte((EEPROM_ADDR + DEVICE_TEMPERATURE_CALIBRATION_ADDRESS)+i);
		calibration_array_2[i] = eeprom_read_byte((EEPROM_ADDR + DEVICE_TEMPERATURE_CALIBRATION_BACKUP_1_ADDRESS)+i);
		calibration_array_3[i] = eeprom_read_byte((EEPROM_ADDR + DEVICE_TEMPERATURE_CALIBRATION_BACKUP_2_ADDRESS)+i);

		if(calibration_array_1[i] != calibration_array_2[i])
			one_matches_two = false;
		if(calibration_array_1[i] != calibration_array_3[i])
			one_matches_three = false;
		if(calibration_array_2[i] != calibration_array_3[i])
			two_matches_three = false;
	}

	//the following is a sign that calibration hasn't been programmed yet
	if((calibration_array_1[0] == 0xff) && (calibration_array_2[0] == 0xff) && (calibration_array_3[0] == 0xff))
		write_defaults = true;

	//the following is a sign that only the backup locations haven't been programmed yet, so any 'match' of 0xffs is misleading.
	if((calibration_array_1[0] != 0xff) && (calibration_array_2[0] == 0xff) && (calibration_array_3[0] == 0xff))
	{
		two_matches_three = false;
		eeprom_write_temperature_calibration_number(calibration_array_1);
	}

	if(write_defaults) //store defaults and use them
	{
		eeprom_write_temperature_calibration_number(default_eeprom);
		one_matches_two = false;
		one_matches_three = false;
		two_matches_three = false;
	}

	if(one_matches_two || one_matches_three)
	{
		for(i = 0; i < DEVICE_TEMPERATURE_CALIBRATION_LENGTH; i++)
			calibration_array_validated[i] = calibration_array_1[i];
	}
	else if(two_matches_three)
	{
		for(i = 0; i < DEVICE_TEMPERATURE_CALIBRATION_LENGTH; i++)
			calibration_array_validated[i] = calibration_array_2[i];
	}
	else if(write_defaults)
	{
		for(i = 0; i < DEVICE_TEMPERATURE_CALIBRATION_LENGTH; i++)
			calibration_array_validated[i] = default_eeprom[i];
	}
	else
	{
		for(i = 0; i < DEVICE_TEMPERATURE_CALIBRATION_LENGTH; i++)
			calibration_array_validated[i] = calibration_array_1[i]; //if there are no matches and only location one has valid data
	}

	union calibrated_parameter_a{
		float float_a;
		uint8_t eeprom_a[4];
	}cp_a;

	union calibrated_parameter_b{
		float float_b;
		uint8_t eeprom_b[4];
	}cp_b;

	union calibrated_parameter_c{
		float float_c;
		uint8_t eeprom_c[4];
	}cp_c;

	//slope and intercept values have been derived from values given by monica.
	double y = adc_value; // y value
	double m = SLOPE_OF_TEMPERATURE_READINGS; // slope of the curve
	double c = INTERCEPT_OF_TEMPERATURE_READINGS; // intercept, value of y at x=0

	// the temperature is calculated as
	double x = (y-c)/m;

	//copy calibration values
	for(i=0; i<4;i++)
	{
		cp_a.eeprom_a[i] = calibration_array_validated[i];
		cp_b.eeprom_b[i] = calibration_array_validated[i + 4];
		cp_c.eeprom_c[i] = calibration_array_validated[i + 8];
	}

	// change endianness
	reverse_for_endianness(cp_a.eeprom_a, 4);
	reverse_for_endianness(cp_b.eeprom_b, 4);
	reverse_for_endianness(cp_c.eeprom_c, 4);

	//calibrated temperature
	double  calibrated_temperature = (x * x * cp_a.float_a) + (x * cp_b.float_b)+ cp_c.float_c ;

	return calibrated_temperature;
}

/**
* @brief Convert data from ADC to temperature.
*
* @author rr
* @version v1.0
* @date 202306061400 created - rr
*/
double VP_calculate_temperature(int32_t adc_value)
{
	vt.adc = (-adc_value * 5) /(pow(2,24));

	vt.r1 = MAX_GAIN_RESISTANCE;

	vt.r2 = VP_TEMPERATURE_IP_RESISTANCE;

	vt.gr_ipr= (-vt.r1/vt.r2);

	vt.t2 = vt.adc/vt.gr_ipr;    // Calculate VT2 from measured ADC (divide by gain control amp gain)

	vt.rt = (vt.t2-VT3) / (VT3/VP_TEMPERATURE_R3);                                // Calculate the resistance of the Thermistor from VT2

	vt.tk = BETA / (log(vt.rt/RT0)+(BETA/T0));                                     // Calculate temp in Kelvin using rearranged BETA formula

	vt.tc = vt.tk - KELVINTOC;

	return vt.tc;
}

