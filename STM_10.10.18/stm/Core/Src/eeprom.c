#include "eeprom.h"

usart_timer_struct command_received_global;

void eeprom_write_square_wave_data_save_status(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_SAVE_SQUARE_WAVE_DATA_ADDRESS, value);
}

uint8_t eeprom_read_square_wave_data_save_status(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_SAVE_SQUARE_WAVE_DATA_ADDRESS, 1, 2, DEVICE_SAVE_SQUARE_WAVE_DATA_DEFAULT_VALUE);
	return read_value;
}

void eeprom_write_external_baud_rate_setting(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_EXTERNAL_BAUD_RATE_ADDRESS, value);
}

uint8_t eeprom_read_external_baud_rate_setting()
{
	uint8_t read_value = 0;
	//setting 1-9600, 2-14400, 3-19200, 4-28800, 5-38400, 6-56000, 7-57600, 8-115200
	read_value = eeprom_read_byte_with_checks(DEVICE_EXTERNAL_BAUD_RATE_ADDRESS, 1, 8, DEVICE_EXTERNAL_BAUD_RATE_DEFAULT_VALUE);
	return read_value;
}

void eeprom_write_sensor_array_interval_setting(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_ARRAY_INTERVAL_ADDRESS, value);
}

uint8_t eeprom_read_sensor_array_interval_setting(void)
{
	uint8_t read_value = 0;
	//the following takes care of when 0 is has been deliberately set, so long as DEVICE_ARRAY_INTERVAL_DEFAULT_VALUE is 0
	read_value = eeprom_read_byte_with_checks(DEVICE_ARRAY_INTERVAL_ADDRESS, 10, 240, DEVICE_ARRAY_INTERVAL_DEFAULT_VALUE);
	return read_value;
}

void eeprom_write_sensor_mode_setting(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_MODE_CONTROL_ADDRESS, value);
}

uint8_t eeprom_read_sensor_mode_setting(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_MODE_CONTROL_ADDRESS, 1, 255, DEVICE_MODE_CONTROL_DEFAULT_VALUE);
	return read_value;
}

void eeprom_write_sensor_communication_style(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_COMMUNICATION_STYLE_ADDRESS, value);
}

uint8_t eeprom_read_sensor_communication_style(void)
{
	uint8_t read_value = 0;
	//		0x01 if ASCII
	//		0x02 if MODBUS
	read_value = eeprom_read_byte_with_checks(DEVICE_COMMUNICATION_STYLE_ADDRESS, 1, 2, DEVICE_COMMUNICATION_STYLE_DEFAULT_VALUE);
	return read_value;
}

void eeprom_write_sensor_serial_number(uint8_t * buffer_start_address)
{
	// erase the entire eeprom on every write of serial number
	eeprom_erase_all();
	// and put in defaults
	reset_eeprom_settings();

	//triple save as spurious readbacks have been observed
	eeprom_write_buffer_with_readback_check(EEPROM_ADDR + DEVICE_SERIAL_NUMBER_ADDRESS, buffer_start_address, DEVICE_SERIAL_NUMBER_LENGTH);
	eeprom_write_buffer_with_readback_check(EEPROM_ADDR + DEVICE_SERIAL_NUMBER_BACKUP_1_ADDRESS, buffer_start_address, DEVICE_SERIAL_NUMBER_LENGTH);
	eeprom_write_buffer_with_readback_check(EEPROM_ADDR + DEVICE_SERIAL_NUMBER_BACKUP_2_ADDRESS, buffer_start_address, DEVICE_SERIAL_NUMBER_LENGTH);
}

/**
* SG - 19/10/2023. use a triple check using backup locations in case of data corruption
*/
void eeprom_read_sensor_serial_number(uint8_t * buffer_start_address)
{
	uint8_t read_value = 0;
	uint8_t serial_number_array_1[DEVICE_SERIAL_NUMBER_LENGTH];
	uint8_t serial_number_array_2[DEVICE_SERIAL_NUMBER_LENGTH];
	uint8_t serial_number_array_3[DEVICE_SERIAL_NUMBER_LENGTH];
	uint8_t serial_number_array_validated[DEVICE_SERIAL_NUMBER_LENGTH];

	int i;
	bool one_matches_two = true;
	bool one_matches_three = true;
	bool two_matches_three = true;

	bool write_default_value = false;

	for(i = 0; i < DEVICE_SERIAL_NUMBER_LENGTH; i++)
	{
		serial_number_array_1[i] = eeprom_read_byte((EEPROM_ADDR + DEVICE_SERIAL_NUMBER_ADDRESS)+i);
		serial_number_array_2[i] = eeprom_read_byte((EEPROM_ADDR + DEVICE_SERIAL_NUMBER_BACKUP_1_ADDRESS)+i);
		serial_number_array_3[i] = eeprom_read_byte((EEPROM_ADDR + DEVICE_SERIAL_NUMBER_BACKUP_2_ADDRESS)+i);

		if(serial_number_array_1[i] != serial_number_array_2[i])
			one_matches_two = false;
		if(serial_number_array_1[i] != serial_number_array_3[i])
			one_matches_three = false;
		if(serial_number_array_2[i] != serial_number_array_3[i])
			two_matches_three = false;
	}

	//the following is a sign that the serial number hasn't been programmed yet
	if((serial_number_array_1[0] == 0xff) && (serial_number_array_2[0] == 0xff) && (serial_number_array_3[0] == 0xff))
		write_default_value = true;

	//the following is a sign that only the backup locations haven't been programmed yet, so any 'match' of 0xffs is misleading.
	if((serial_number_array_1[0] != 0xff) && (serial_number_array_2[0] == 0xff) && (serial_number_array_3[0] == 0xff))
	{
		two_matches_three = false;
		eeprom_write_sensor_serial_number(serial_number_array_1);
	}

	for(i = 0; i < DEVICE_SERIAL_NUMBER_LENGTH; i++)
	{
		if(one_matches_two || one_matches_three)
			serial_number_array_validated[i] = serial_number_array_1[i];
		else if(two_matches_three)
				serial_number_array_validated[i] = serial_number_array_2[i];
		else //no matches, go with first location
			serial_number_array_validated[i] = serial_number_array_1[i];
	}

	// read the byte(s)
	for (int i = 0; i < DEVICE_SERIAL_NUMBER_LENGTH; i++)
	{
		read_value = serial_number_array_validated[i];
		if (read_value < 0x30)
		{
			write_default_value = true;
		}
		else if (read_value > 0x39)
		{
			write_default_value = true;
		}
		else
		{
			*(buffer_start_address + i) = read_value;
		}

		// write the default value
		if (write_default_value)
		{
			// set i to length, to break out of the outer for loop once one value is not a number between 0 and 9
			i = DEVICE_SERIAL_NUMBER_LENGTH;
			// set the default serial number 0
			for (int j = 0; j < DEVICE_SERIAL_NUMBER_LENGTH; j++)
			{
				*(buffer_start_address + j) = DEVICE_SERIAL_NUMBER_DEFAULT_VALUE;
			}
			// write the default value
			eeprom_write_sensor_serial_number(buffer_start_address);
		}
	}
}

void eeprom_write_ph_output_style_setting(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_OUTPUT_STYLE_ADDRESS, value);
}

uint8_t eeprom_read_ph_output_style_setting(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_PH_OUTPUT_STYLE_ADDRESS, 1, 2, DEVICE_PH_OUTPUT_STYLE_DEFAULT_VALUE);
	return read_value;
}

void eeprom_write_immersion_rule(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IMMERSION_RULE_ADDRESS, value);
}

void eeprom_write_immersion_rule_interval(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IMMERSION_RULE_INTERVAL_ADDRESS, value);
}

uint8_t eeprom_read_immersion_rule(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_IMMERSION_RULE_ADDRESS, 1, 2, DEVICE_IMMERSION_RULE_DEFAULT_VALUE);
	return read_value;
}

uint8_t eeprom_read_immersion_rule_interval(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_IMMERSION_RULE_INTERVAL_ADDRESS, 0, 240, DEVICE_IMMERSION_RULE_INTERVAL_DEFAULT_VALUE);
	return read_value;
}

void eeprom_write_array_health(uint8_t health)
{
	uint8_t value[4];

	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_ARRAY_HEALTH_ADDRESS, health);

	float_to_byte_array(value, scan_health);
	for(int i =0 ; i < 4; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_SCAN_HEALTH_ADDRESS + i, value[i]);
}

uint8_t eeprom_read_array_health(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_ARRAY_HEALTH_ADDRESS, 0, 6, DEVICE_ARRAY_HEALTH_DEFAULT_VALUE);

	scan_health = eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_SCAN_HEALTH_ADDRESS);

	return read_value;
}

/**
* @brief Check if sensor autonomous start delay setting is valid.
*
* The valid values are 0 minutes, 5 minutes to 24 hours.
* This code would do the following:
*	1 to 4 minutes would be set to 5 minutes, the minimum allowed
*	anything working out to more than 24 hours would be set to 24 hours
*	eg:
* 	2 hours 75 minutes would be set as 3 hours and 15 minutes
*	67 minutes would be set as 1 hour and 7 minutes
*	60 minutes would be set as 1 hour and 0 minutes
*	23 hours 75 minutes would be set as 24 hours and 0 minutes, as
*		24 hours and 15 minutes exceeds maximum of 24 hours
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202302211250 created - PV
*/
uint16_t condition_check_sensor_autonomous_start_delay_setting(uint8_t hours, uint8_t minutes)
{
	// check if the value is within the values allowed
	//		if yes, write the values / corrected byte(s)
	//		if no, ignore
	// condition check
	//		valid values are 0 minutes, 5 minutes to 24 hours.
	//		This code would force do the following:
	//		1 to 4 minutes would be set to 5 minutes, the minimum allowed
	//		anything working out to more than 24 hours would be set to 24 hours
	//		eg:
	//			2 hours 75 minutes would be set as 3 hours and 15 minutes
	//			67 minutes would be set as 1 hour and 7 minutes
	//			60 minutes would be set as 1 hour and 0 minutes
	//			23 hours 75 minutes would be set as 24 hours and 0 minutes, as
	//			24 hours and 15 minutes exceeds maximum of 24 hours
	uint16_t return_value = 0;
	uint8_t temp_minutes = minutes;
	uint8_t temp_hours = 0;

	// if minutes greater than or equal to 60, increment hour(s)
	while (temp_minutes >= 60)
	{
		temp_minutes = temp_minutes - 60;
		temp_hours = temp_hours + 1;
	}
	// add the input hours to overflow hours
	temp_hours = temp_hours + hours;

	// condition check
	// currently - if 1 to 4 minutes, set to 5 minutes
	if (temp_hours == DEVICE_AUTONOMOUS_ON_DELAY_MINIMUM_HOUR_VALUE)
	{
		if (temp_minutes > DEVICE_AUTONOMOUS_ON_DELAY_MINIMUM_MINUTE_VALUE)
		{
			if (temp_minutes < DEVICE_AUTONOMOUS_ON_DELAY_MINIMUM_MINUTES_VALUE_ABOVE_ZERO)
			{
				temp_minutes = DEVICE_AUTONOMOUS_ON_DELAY_MINIMUM_MINUTES_VALUE_ABOVE_ZERO;
			}
		}
	}
	// currently - if > 24 hours, limit to 24 hours
	if (temp_hours > DEVICE_AUTONOMOUS_ON_DELAY_MAXIMUM_HOUR_VALUE)
	{
		temp_hours = DEVICE_AUTONOMOUS_ON_DELAY_MAXIMUM_HOUR_VALUE;
		temp_minutes = DEVICE_AUTONOMOUS_ON_DELAY_MAXIMUM_MINUTE_VALUE;
	}

	// convert to 16 bit value for return
	return_value = ((uint16_t) (temp_hours << 8)) + ((uint16_t) temp_minutes);

	return return_value;
}

/**
* @brief Write sensor autonomous start delay setting to internal EEPROM.
*
* @params hours a byte containing hours, valid values are 0 to 24, anything above 24 would be corrected
* @params minutes a byte containing minutes, valid values are 0 to 59, anything above would increment the hours
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202302211151 created - PV
*/
void eeprom_write_sensor_autonomous_start_delay_setting(uint8_t hours, uint8_t minutes)
{
	// check if the value is within the values allowed, or is corrected in the check function
	uint16_t temp_hour_minutes = condition_check_sensor_autonomous_start_delay_setting(hours, minutes);
	// write the bytes
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_AUTONOMOUS_ON_DELAY_HOUR_ADDRESS, ((uint8_t) (temp_hour_minutes >> 8)));
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_AUTONOMOUS_ON_DELAY_MINUTES_ADDRESS, ((uint8_t) temp_hour_minutes));
}

/**
* @brief Read sensor autonomous start delay setting from internal EEPROM.
*
* @return setting 0 minutes or 5 minutes to 24 hours as two bytes hour byte + minute byte (16 bits)
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202302211154 created - PV
*/
uint16_t eeprom_read_sensor_autonomous_start_delay_setting(void)
{
	uint8_t read_hour_value = 0;
	uint8_t read_minute_value = 0;

	// read the byte(s)
	read_hour_value = eeprom_read_byte(EEPROM_ADDR + DEVICE_AUTONOMOUS_ON_DELAY_HOUR_ADDRESS);
	read_minute_value = eeprom_read_byte(EEPROM_ADDR + DEVICE_AUTONOMOUS_ON_DELAY_MINUTES_ADDRESS);

	// check if the value is within the values allowed, or is corrected in the check function
	//		so only valid value is sent out
	uint16_t temp_hour_minutes = condition_check_sensor_autonomous_start_delay_setting(read_hour_value, read_minute_value);

	return temp_hour_minutes;
}

void reset_eeprom_settings(void)
{
	// set the default autonomous delay value
	eeprom_write_sensor_autonomous_start_delay_setting(DEVICE_AUTONOMOUS_ON_DELAY_HOUR_DEFAULT_VALUE, DEVICE_AUTONOMOUS_ON_DELAY_MINUTES_DEFAULT_VALUE);
	eeprom_write_sensor_mode_setting(COMMAND_OPTION_TEXT_SENSOR_MODE_CONTROLLED);
	eeprom_write_sensor_communication_style(COMMAND_OPTION_TEXT_ASCII_COMMUNICATION_STYLE);
	eeprom_write_loop_count(0);
	//eeprom_write_external_baud_rate_setting(DEVICE_EXTERNAL_BAUD_RATE_DEFAULT_VALUE);
}

void eeprom_write_temperature_calibration_number(uint8_t * buffer_start_address)
{
	// writes the calibration value entered by the customer into the eeprom where it is read to calibrate the temperature readings
	// write the byte(s)
	eeprom_write_buffer(EEPROM_ADDR + DEVICE_TEMPERATURE_CALIBRATION_ADDRESS, buffer_start_address, DEVICE_TEMPERATURE_CALIBRATION_LENGTH);
	eeprom_write_buffer(EEPROM_ADDR + DEVICE_TEMPERATURE_CALIBRATION_BACKUP_1_ADDRESS, buffer_start_address, DEVICE_TEMPERATURE_CALIBRATION_LENGTH);
	eeprom_write_buffer(EEPROM_ADDR + DEVICE_TEMPERATURE_CALIBRATION_BACKUP_2_ADDRESS, buffer_start_address, DEVICE_TEMPERATURE_CALIBRATION_LENGTH);
}

void eeprom_write_sensor_name(uint8_t * buffer_start_address)
{
	if(command_received_global.command_size == (2 + DEVICE_SENSOR_NAME_LENGTH))
	{
		eeprom_write_buffer_with_readback_check(EEPROM_ADDR + DEVICE_SENSOR_NAME_ADDRESS, buffer_start_address, DEVICE_SENSOR_NAME_LENGTH);
	}
	else if (command_received_global.command_size < (2 + DEVICE_SENSOR_NAME_LENGTH))
	{

		for(int i = (command_received_global.command_size -1) ; i < (DEVICE_SENSOR_NAME_LENGTH +2) ; i++)
		{
			eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_SENSOR_NAME_ADDRESS + i, DEVICE_SENSOR_NAME_DEFAULT_VALUE);
		}

		eeprom_write_buffer_with_readback_check(EEPROM_ADDR + DEVICE_SENSOR_NAME_ADDRESS, buffer_start_address, (command_received_global.command_size -2));

	}
	else
	{

	}
}

void eeprom_read_sensor_name(uint8_t * buffer_start_address)
{
	uint8_t read_value = 0;
	bool write_default_value = false;
	// read the byte(s)
	for (int i = 0; i < DEVICE_SENSOR_NAME_LENGTH; i++)
	{
		read_value = eeprom_read_byte(EEPROM_ADDR + DEVICE_SENSOR_NAME_ADDRESS + i);
		// check if the value is within the values allowed
		//		if yes, return that byte
		//		if no, write the default value from program code and return the default value
		//			this should happen the first time at least when the eeprom hasn't been written to
		// condition check
		//		0x30 to 0x39 - allowed characters
		//		anything else set to 0x30 / 0 / zero
		if (read_value ==0x03)
		{
			*(buffer_start_address + i) = read_value;
		}
		else if (read_value == 0x20)
		{
			*(buffer_start_address + i) = read_value;
		}
		else if (read_value == 0x2d)
		{
			*(buffer_start_address + i) = read_value;
		}
		else if (read_value == 0x2e)
		{
			*(buffer_start_address + i) = read_value;
		}
		else if (read_value < 0x30)
		{
			write_default_value = true;
		}
		else if (read_value > 0x39)
		{	if(read_value < 0x41)
			{
				write_default_value = true;
			}
			else
			{
				*(buffer_start_address + i) = read_value;
			}

		}
		else if (read_value > 0x5A)
		{
			if (read_value < 0x61)
			{
				write_default_value = true;
			}
			else
			{
				*(buffer_start_address + i) = read_value;
			}

		}
		else if (read_value > 0x7A)
		{
			write_default_value = true;
		}
		else
		{
			*(buffer_start_address + i) = read_value;
		}

		// write the default value
		if (write_default_value)
		{
			// set i to length, to break out of the outer for loop once one value is not a number between 0 and 9
			i = DEVICE_SENSOR_NAME_LENGTH;
			// set the default serial number 0
			for (int j = 0; j < DEVICE_SENSOR_NAME_LENGTH; j++)
			{
				*(buffer_start_address + j) = DEVICE_SENSOR_NAME_DEFAULT_VALUE;
			}
			// write the default value
			eeprom_write_sensor_name(buffer_start_address);
		}
	}
}

void eeprom_write_sensor_sub_name(uint8_t * buffer_start_address)
{
	if(command_received_global.command_size == (2 + DEVICE_SENSOR_SUB_NAME_LENGTH) )
	{
		eeprom_write_buffer_with_readback_check(EEPROM_ADDR + DEVICE_SENSOR_SUB_NAME_ADDRESS, buffer_start_address, DEVICE_SENSOR_SUB_NAME_LENGTH);
	}
	else if (command_received_global.command_size < (2 + DEVICE_SENSOR_SUB_NAME_LENGTH))
	{

		for(int i = (command_received_global.command_size -1) ; i < (DEVICE_SENSOR_SUB_NAME_LENGTH +2) ; i++)
		{
			eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_SENSOR_SUB_NAME_ADDRESS + i, DEVICE_SENSOR_SUB_NAME_DEFAULT_VALUE);
		}

		eeprom_write_buffer_with_readback_check(EEPROM_ADDR + DEVICE_SENSOR_SUB_NAME_ADDRESS, buffer_start_address, (command_received_global.command_size -2));

	}
	else
	{
	}
}

void eeprom_read_sensor_sub_name(uint8_t * buffer_start_address)
{
	uint8_t read_value = 0;
	bool write_default_value = false;
	// read the byte(s)
	for (int i = 0; i < DEVICE_SENSOR_SUB_NAME_LENGTH; i++)
	{
		read_value = eeprom_read_byte(EEPROM_ADDR + DEVICE_SENSOR_SUB_NAME_ADDRESS + i);
		// check if the value is within the values allowed
		//		if yes, return that byte
		//		if no, write the default value from program code and return the default value
		//			this should happen the first time at least when the eeprom hasn't been written to
		// condition check
		//		0x30 to 0x39 - allowed characters
		//		anything else set to 0x30 / 0 / zero
		if (read_value ==0x03)
		{
			*(buffer_start_address + i) = read_value;
		}
		else if (read_value == 0x20)
		{
			*(buffer_start_address + i) = read_value;
		}
		else if (read_value == 0x2d)
		{
			*(buffer_start_address + i) = read_value;
		}
		else if (read_value == 0x2e)
		{
			*(buffer_start_address + i) = read_value;
		}
		else if (read_value < 0x30)
		{
			write_default_value = true;
		}
		else if (read_value > 0x39)
		{	if(read_value < 0x41)
			{
				write_default_value = true;
			}
			else
			{
				*(buffer_start_address + i) = read_value;
			}

		}
		else if (read_value > 0x5A)
		{
			if (read_value < 0x61)
			{
				write_default_value = true;
			}
			else
			{
				*(buffer_start_address + i) = read_value;
			}

		}
		else if (read_value > 0x7A)
		{
			write_default_value = true;
		}
		else
		{
			*(buffer_start_address + i) = read_value;
		}

		// write the default value
		if (write_default_value)
		{
			// set i to length, to break out of the outer for loop once one value is not a number between 0 and 9
			i = DEVICE_SENSOR_SUB_NAME_LENGTH;
			// set the default serial number 0
			for (int j = 0; j < DEVICE_SENSOR_SUB_NAME_LENGTH; j++)
			{
				*(buffer_start_address + j) = DEVICE_SENSOR_SUB_NAME_DEFAULT_VALUE;
			}
			// write the default value
			eeprom_write_sensor_name(buffer_start_address);
		}
	}
}

void eeprom_write_heartbeat_output_style_setting(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_SENSOR_HEARTBEAT_STYLE_ADDRESS, value);
}

void eeprom_write_system_modbus_address(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_SENSOR_SYSTEM_MODBUS_ADDRESS, value);
}

uint8_t eeprom_read_system_modbus_address(void)
{
	uint8_t read_value = 0;
	bool write_default_value = false;
	// read the byte
	read_value = eeprom_read_byte(EEPROM_ADDR + DEVICE_SENSOR_SYSTEM_MODBUS_ADDRESS);

	if (read_value == 0xff)
	{
		write_default_value = true;
	}
	// write the default value
	if (write_default_value)
	{
		read_value = DEVICE_SENSOR_SYSTEM_MODBUS_ADDRESS_DEFAULT_VALUE;
		// write the default value
		eeprom_write_system_modbus_address(read_value);
	}

	return read_value;
}

uint8_t eeprom_read_heartbeat_output_style_setting(void)
{
	uint8_t read_value = 0;
	bool write_default_value = false;
	// read the byte
	read_value = eeprom_read_byte(EEPROM_ADDR + DEVICE_SENSOR_HEARTBEAT_STYLE_ADDRESS);
	// check if the value is within the values allowed
	//		if yes, return that byte
	//		if no, write the default value from program code and return the default value
	//			this should happen the first time at least when the eeprom hasn't been written to
	// condition check
	//		0x01 if output all heartbeat
	//		0x02 if output only ph

	if (read_value == 0xff)
	{
		write_default_value = true;
	}
	// write the default value
	if (write_default_value)
	{
		read_value = DEVICE_SENSOR_HEARTBEAT_STYLE_DEFAULT_VALUE;
		// write the default value
		eeprom_write_heartbeat_output_style_setting(read_value);
	}

	return read_value;
}

void eeprom_read_ph_array(void)
{
	uint8_t read_value = 0;

	for(int i = 0 ; i < DEVICE_PH_ARRAY_LENGTH; i++)
		ph_values[i] = eeprom_get_float_from_eeprom_bytes(DEVICE_PH_ARRAY_START + 4*i);

	read_value = eeprom_read_byte_with_checks(DEVICE_PH_ARRAY_INDEX, 0, PH_ARRAY_SIZE_MAX, DEVICE_PH_ARRAY_INDEX_DEFAULT_VALUE);
	ph_array_index = read_value;

	read_value = eeprom_read_byte_with_checks(DEVICE_PH_ARRAY_INDEX_LOOPED_FLAG, 0, 1, DEVICE_PH_ARRAY_INDEX_LOOPED_FLAG_DEFAULT_VALUE);
	ph_array_index_looped = (bool)read_value;

	displayed_ph_running_average = eeprom_read_ph_running_average();
	previous_ph = displayed_ph_running_average;

	last_temperature = eeprom_read_last_temperature();
	highest_ph_qaqc = eeprom_read_highest_ph_qaqc();
	avg_ph_peak_potential = eeprom_read_avg_ph_peak_potential();
	previous_ph_peak_potential = avg_ph_peak_potential;
	read_value = eeprom_read_byte_with_checks(DEVICE_AVG_PH_PEAK_POTENTIAL_STORED, 0, 1, 0);
	avg_ph_peak_potential_stored = (bool)read_value;
	read_value = eeprom_read_byte_with_checks(DEVICE_PRECONDITIONING_DONE, 0, 1, 0);
	preconditioning_done = (bool)read_value;

	read_value = eeprom_read_byte_with_checks(DEVICE_ELECTRODE_ARRAY_LOOPED, 0, 1, 0);
	electrode_array_looped = (bool)read_value;
}

void eeprom_read_peak_potential_offsets(void)
{
	for(int i = 0 ; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		peak_potential_offsets[i] = eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_PEAK_POTENTIAL_OFFSETS_START + 4*i);
}

void eeprom_read_peak_potentials(void)
{
	for(int i = 0 ; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		peak_potentials[i] = eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_PEAK_POTENTIALS_START + 4*i);
}

void eeprom_read_qaqcs(void)
{
	for(int i = 0 ; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		qaqcs[i] = eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_QAQCS_START + 4*i);
}

void eeprom_read_ph_peaks_loops(void)
{
	for(int i = 0 ; i < PEAKS_LOOPS_ARRAY_SIZE; i++)
		ph_peaks_loops[i] = eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_PH_PEAKS_LOOPS_START + 4*i);
}

void eeprom_read_iref_peaks_loops(void)
{
	for(int i = 0 ; i < PEAKS_LOOPS_ARRAY_SIZE; i++)
		iref_peaks_loops[i] = eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_IREF_PEAKS_LOOPS_START + 4*i);
}

void eeprom_read_pulse_amplitudes(void)
{
	for(int i = 0 ; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		electrode_pulse_amplitudes[i] = eeprom_read_byte(EEPROM_ADDR + DEVICE_PULSE_AMPLITUDES_START + i);
}

void eeprom_read_frequencies(void)
{
	for(int i = 0 ; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		electrode_frequencies[i] = eeprom_read_byte(EEPROM_ADDR + DEVICE_FREQUENCIES_START + i);
}

void eeprom_read_clipped_at_gain(void)
{
	for(int i = 0 ; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		electrode_clipped_at_gain[i] = eeprom_read_byte_with_checks(DEVICE_CLIPPED_AT_GAIN_START + i, 1, 255, 255);
}

void eeprom_save_ph_array(void)
{
	uint8_t value[4];

	for(int i = 0; i < DEVICE_PH_ARRAY_LENGTH; i++)
	{
		float_to_byte_array(value, ph_values[i]);

		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_ARRAY_START + (4*i +0), value[0] ) ;
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_ARRAY_START + (4*i +1), value[1] ) ;
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_ARRAY_START + (4*i +2), value[2] ) ;
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_ARRAY_START + (4*i +3), value[3] ) ;
	}

	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_ARRAY_INDEX_LOOPED_FLAG, ph_array_index_looped);
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_ARRAY_INDEX, ph_array_index);

	//now save other stuff while we're here
	eeprom_save_ph_running_average();
	//eeprom_save_last_temperature();
	//eeprom_save_avg_iref_peak_potential();
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_ELECTRODE_ARRAY_LOOPED, electrode_array_looped);
}

void eeprom_save_peak_potential_offsets(void)
{
	uint8_t value[4];

	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
	{
		float_to_byte_array(value, peak_potential_offsets[i]);

		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PEAK_POTENTIAL_OFFSETS_START + (4*i +0), value[0]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PEAK_POTENTIAL_OFFSETS_START + (4*i +1), value[1]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PEAK_POTENTIAL_OFFSETS_START + (4*i +2), value[2]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PEAK_POTENTIAL_OFFSETS_START + (4*i +3), value[3]);
	}
}

void eeprom_save_peak_potentials(void)
{
	uint8_t value[4];

	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
	{
		float_to_byte_array(value, peak_potentials[i]);

		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PEAK_POTENTIALS_START + (4*i +0), value[0]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PEAK_POTENTIALS_START + (4*i +1), value[1]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PEAK_POTENTIALS_START + (4*i +2), value[2]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PEAK_POTENTIALS_START + (4*i +3), value[3]);
	}
}

void eeprom_save_qaqcs(void)
{
	uint8_t value[4];

	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
	{
		float_to_byte_array(value, qaqcs[i]);

		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_QAQCS_START + (4*i +0), value[0]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_QAQCS_START + (4*i +1), value[1]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_QAQCS_START + (4*i +2), value[2]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_QAQCS_START + (4*i +3), value[3]);
	}
}

void eeprom_save_ph_peaks_loops(void)
{
	uint8_t value[4];

	for(int i = 0; i < PEAKS_LOOPS_ARRAY_SIZE; i++)
	{
		float_to_byte_array(value, ph_peaks_loops[i]);

		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_PEAKS_LOOPS_START + (4*i +0), value[0]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_PEAKS_LOOPS_START + (4*i +1), value[1]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_PEAKS_LOOPS_START + (4*i +2), value[2]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_PEAKS_LOOPS_START + (4*i +3), value[3]);
	}
}

void eeprom_save_iref_peaks_loops(void)
{
	uint8_t value[4];

	for(int i = 0; i < PEAKS_LOOPS_ARRAY_SIZE; i++)
	{
		float_to_byte_array(value, iref_peaks_loops[i]);

		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IREF_PEAKS_LOOPS_START + (4*i +0), value[0]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IREF_PEAKS_LOOPS_START + (4*i +1), value[1]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IREF_PEAKS_LOOPS_START + (4*i +2), value[2]);
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IREF_PEAKS_LOOPS_START + (4*i +3), value[3]);
	}
}

void eeprom_read_electrode_health(void)
{
	for(int i = 0 ; i < DEVICE_HEALTH_ARRAY_LENGTH; i++)
		electrode_health_values[i] = eeprom_read_byte(EEPROM_ADDR + DEVICE_HEALTH_ARRAY_START + i);
}

void eeprom_save_electrode_health(void)
{
	for(int i = 0; i < DEVICE_HEALTH_ARRAY_LENGTH; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_HEALTH_ARRAY_START + i, electrode_health_values[i]);
}

void eeprom_save_pulse_amplitudes(void)
{
	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PULSE_AMPLITUDES_START + i, electrode_pulse_amplitudes[i]);
}

void eeprom_save_frequencies(void)
{
	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_FREQUENCIES_START + i, electrode_frequencies[i]);
}

void eeprom_save_clipped_at_gain(void)
{
	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_CLIPPED_AT_GAIN_START + i, electrode_clipped_at_gain[i]);
}

void eeprom_read_electrode_gains(void)
{
	uint8_t read_value = 0;

	for(int i = 0 ; i < DEVICE_GAIN_ARRAY_LENGTH; i++)
	{
		read_value = eeprom_read_byte(EEPROM_ADDR + DEVICE_GAIN_ARRAY_START + i);
		if((read_value < 1) || (read_value > MAX_GAIN_SETTING))
		{
			read_value = PH_GAIN_RESISTOR_SETTING_OCEAN; //might want to refine this. only applicable on new build when eeprom values uninitialized
			eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_GAIN_ARRAY_START + i, read_value);
		}

		electrode_gain_values[i] = read_value;
	}
}

void eeprom_save_electrode_gains(void)
{
	for(int i = 0 ; i < DEVICE_GAIN_ARRAY_LENGTH; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_GAIN_ARRAY_START + i, electrode_gain_values[i]);

}

void eeprom_save_last_temperature()
{
	uint8_t value[4];

	float_to_byte_array(value, last_temperature);
	for(int i =0 ; i < 4; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_LAST_TEMPERATURE_START + i, value[i]);
}

void eeprom_save_highest_ph_qaqc()
{
	uint8_t value[4];

	float_to_byte_array(value, highest_ph_qaqc);
	for(int i =0 ; i < 4; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_highest_PH_QAQC_START + i, value[i]);
}

void eeprom_save_ph_running_average()
{
	uint8_t value[4];

	float_to_byte_array(value, displayed_ph_running_average);
	for(int i =0 ; i < 4; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PH_RUNNING_AVERAGE_START + i, value[i]);
}

void eeprom_save_avg_iref_peak_potential()
{
	uint8_t value[4];

	float_to_byte_array(value, avg_iref_peak_potential);
	for(int i = 0 ; i < 4; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_AVG_IREF_PEAK_POTENTIAL_START + i, value[i]);
}

void eeprom_save_avg_ph_peak_potential()
{
	uint8_t value[4];

	float_to_byte_array(value, avg_ph_peak_potential);
	for(int i = 0 ; i < 4; i++)
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_AVG_PH_PEAK_POTENTIAL_START + i, value[i]);

	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_AVG_PH_PEAK_POTENTIAL_STORED, avg_ph_peak_potential_stored);
}

double eeprom_read_last_temperature()
{
	return eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_LAST_TEMPERATURE_START);
}

double eeprom_read_highest_ph_qaqc()
{
	return eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_highest_PH_QAQC_START);
}

double eeprom_read_ph_running_average()
{
	return eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_PH_RUNNING_AVERAGE_START);
}

double eeprom_read_last_iref_peak_potential()
{
	return eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_AVG_IREF_PEAK_POTENTIAL_START);
}

double eeprom_read_avg_ph_peak_potential()
{
	return eeprom_get_float_from_eeprom_bytes(EEPROM_ADDR + DEVICE_AVG_PH_PEAK_POTENTIAL_START);
}

void eeprom_write_anbsensors_or_alternate_head(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_SCAN_ANB_OR_ALTERNATE_HEAD_ADDRESS, value);
}

void eeprom_write_interface(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_INTERFACE_ADDRESS, value);
}

void eeprom_write_fast_profiling(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_FAST_PROFILING_ADDRESS, value);
}

void eeprom_write_use_sdcard(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_USE_SDCARD_ADDRESS, value);
}

void eeprom_write_power(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_POWER_ADDRESS, value);
}

void eeprom_write_salinity_range(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_SALINITY_RANGE_ADDRESS, value);
}

void eeprom_write_phs_per_loop(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PHS_PER_LOOP_ADDRESS, value);
}

void eeprom_write_fresh_electrodes(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_FRESH_ELECTRODES_ADDRESS, value);
}

void eeprom_write_loops_since_last_good_ph(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_LOOPS_SINCE_LAST_GOOD_PH_ADDRESS, value);
}

void eeprom_write_loops_since_last_event(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_LOOPS_SINCE_LAST_EVENT_ADDRESS, value);
}

void eeprom_write_loops_since_last_correction(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_LOOPS_SINCE_LAST_CORRECTION_ADDRESS, value);
}

void eeprom_write_loops_since_start(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_LOOPS_SINCE_START_ADDRESS, value);
}

void eeprom_write_next_set_electrode(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_NEXT_SET_ELECTRODE_ADDRESS, value);
}

//loop count uses 3 bytes, so wraps around at 16777216
void eeprom_write_loop_count(uint32_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_LOOP_COUNT_ADDRESS + 2, (uint8_t)(value >> 16));
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_LOOP_COUNT_ADDRESS + 1, (uint8_t)(value >> 8));
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_LOOP_COUNT_ADDRESS, (uint8_t)(value));
}

uint8_t eeprom_read_interface(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_INTERFACE_ADDRESS, 1, 2, 1);
	return read_value;
}

uint8_t eeprom_read_fast_profiling(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_FAST_PROFILING_ADDRESS, 1, 2, 2);
	return read_value;
}

uint8_t eeprom_read_use_sdcard(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_USE_SDCARD_ADDRESS, 1, 2, 1);
	return read_value;
}

uint8_t eeprom_read_power(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_POWER_ADDRESS, 1, 2, 1);
	return read_value;
}

uint8_t eeprom_read_salinity_range(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_SALINITY_RANGE_ADDRESS, 1, 3, 1);
	return read_value;
}

uint8_t eeprom_read_phs_per_loop(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_PHS_PER_LOOP_ADDRESS, 1, 254, 11);
	return read_value;
}

uint8_t eeprom_read_fresh_electrodes(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_FRESH_ELECTRODES_ADDRESS, 1, 2, 1);
	return read_value;
}

uint8_t eeprom_read_loops_since_last_good_ph(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_LOOPS_SINCE_LAST_GOOD_PH_ADDRESS, 0, 254, 0);
	return read_value;
}

uint8_t eeprom_read_loops_since_last_event(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_LOOPS_SINCE_LAST_EVENT_ADDRESS, 0, 254, 0);
	return read_value;
}

uint8_t eeprom_read_loops_since_last_correction(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_LOOPS_SINCE_LAST_CORRECTION_ADDRESS, 0, 254, 0);
	return read_value;
}

uint8_t eeprom_read_loops_since_start(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_LOOPS_SINCE_START_ADDRESS, 0, 254, 0);
	return read_value;
}

uint8_t eeprom_read_next_set_electrode(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_NEXT_SET_ELECTRODE_ADDRESS, 0, 13, 0);
	return read_value;
}

uint32_t eeprom_read_loop_count(void)
{
	uint8_t read_value = 0;
	uint32_t loops = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_LOOP_COUNT_ADDRESS + 2, 0, 255, 0);
	loops += read_value << 16;
	read_value = eeprom_read_byte_with_checks(DEVICE_LOOP_COUNT_ADDRESS + 1, 0, 255, 0);
	loops += read_value << 8;
	read_value = eeprom_read_byte_with_checks(DEVICE_LOOP_COUNT_ADDRESS, 0, 255, 0);
	loops += read_value;
	return loops;
}

uint8_t eeprom_read_anbsensors_or_alternate_head(void)
{
	uint8_t read_value = 0;
	// 0x01 if anb sensor interface
	// 0x02 if valeport interface
	read_value = eeprom_read_byte_with_checks(DEVICE_SCAN_ANB_OR_ALTERNATE_HEAD_ADDRESS, 1, 2, DEVICE_SCAN_ANB_OR_ALT_HEAD_DEFAULT_VALUE);
	return read_value;
}

void eeprom_write_iref_health(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IREF_HEALTH_ADDRESS, (uint8_t)value);
}

void eeprom_write_iref_secondary_health(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IREF_SECONDARY_HEALTH_ADDRESS, (uint8_t)value);
}

void eeprom_write_iref_last_used(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IREF_LAST_USED_ADDRESS, (uint8_t)value);
}

void eeprom_write_iref_health_bad_count(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IREF_HEALTH_BAD_COUNT_ADDRESS, (uint8_t)value);
}

void eeprom_write_iref_secondary_health_bad_count(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_IREF_SECONDARY_HEALTH_BAD_COUNT_ADDRESS, (uint8_t)value);
}

uint8_t eeprom_read_iref_primary_health(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_IREF_HEALTH_ADDRESS, 0, 1, 0);
	return read_value;
}

uint8_t eeprom_read_iref_secondary_health(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_IREF_SECONDARY_HEALTH_ADDRESS, 0, 1, 0);
	return read_value;
}

uint8_t eeprom_read_iref_last_used(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_IREF_LAST_USED_ADDRESS, IREF_SECONDARY_SENSOR_NUMBER, IREF_PRIMARY_SENSOR_NUMBER, IREF_PRIMARY_SENSOR_NUMBER);
	return read_value;
}

uint8_t eeprom_read_iref_health_bad_count(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_IREF_HEALTH_BAD_COUNT_ADDRESS, 0, 255, 0);
	return read_value;
}

uint8_t eeprom_read_iref_secondary_health_bad_count(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_IREF_SECONDARY_HEALTH_BAD_COUNT_ADDRESS, 0, 255, 0);
	return read_value;
}

void eeprom_write_scan_n_no_of_ph_electrodes(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_SCAN_N_NO_OF_PH_ELECTRODES_ADDRESS, value);
}

uint8_t eeprom_read_scan_n_no_of_ph_electrodes(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_SCAN_N_NO_OF_PH_ELECTRODES_ADDRESS, 1, NUMBER_OF_PH_ELECTRODES_MAX, NUMBER_OF_PH_ELECTRODES_ANB);
	return read_value;
}

void eeprom_write_last_electrode_number(uint8_t value)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_LAST_ELECTRODE_NUMBER_ADDRESS, value);
}

uint8_t eeprom_read_last_electrode_number(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_LAST_ELECTRODE_NUMBER_ADDRESS, 1, 13, DEVICE_LAST_ELECTRODE_NUMBER_DEFAULT_VALUE);
	return read_value;
}

void eeprom_read_driver_version(uint8_t * buffer_start_address)
{
	uint8_t read_value = 0;
	for (int i = 0; i < DEVICE_DRIVER_FIRMWARE_VERSION_LENGTH; i++)
	{
		read_value = eeprom_read_byte(EEPROM_ADDR + DEVICE_DRIVER_FIRMWARE_VERSION_ADDRESS + i);
		*(buffer_start_address + i) = read_value;
	}
}

void eeprom_write_driver_version(uint8_t * buffer_start_address)
{
	eeprom_write_buffer_with_readback_check(EEPROM_ADDR + DEVICE_DRIVER_FIRMWARE_VERSION_ADDRESS, buffer_start_address, DEVICE_DRIVER_FIRMWARE_VERSION_LENGTH);
}

uint8_t read_interface_driver_version(void)
{
	uint8_t read_value = 0;
	read_value = eeprom_read_byte_with_checks(DEVICE_INTERFACE_VERSION_ADDRESS, 0, 255, 255);
	return read_value;
}

void eeprom_write_interface_version(uint8_t byte_one, uint8_t byte_two)
{
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_INTERFACE_VERSION_ADDRESS, byte_one);
	eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_INTERFACE_VERSION_ADDRESS + 1, byte_two);
}

//helper function to check eeprom contents against expected range, and does a default write if necessary
uint8_t eeprom_read_byte_with_checks(eeprom_addr_t address_definition, uint8_t lower_bound, uint8_t upper_bound, uint8_t default_value)
{
	uint8_t read_value = 0, second_read = 0, third_read = 0;
	bool write_default_value = false;

	read_value = eeprom_read_byte(EEPROM_ADDR + address_definition);
	second_read = eeprom_read_byte(EEPROM_ADDR + address_definition);

	if(read_value != second_read)
	{
		third_read = eeprom_read_byte(EEPROM_ADDR + address_definition);
		if(third_read != second_read)
		{
			third_read = eeprom_read_byte(EEPROM_ADDR + address_definition);
		}
		else
			read_value = third_read;
	}

	if (read_value < lower_bound)
		write_default_value = true;

	if (read_value > upper_bound)
		write_default_value = true;

	if (write_default_value)
	{
		read_value = default_value;
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + address_definition, read_value);
	}

	return read_value;

}

float eeprom_get_float_from_eeprom_bytes(eeprom_addr_t address_definition_start)
{
	union return_value{
		float float_retval;
		uint8_t eeprom_retval[4];
	}retval;

	for(int i=0; i<4;i++)
		retval.eeprom_retval[i] = eeprom_read_byte(EEPROM_ADDR + address_definition_start + i);

	reverse_for_endianness(retval.eeprom_retval, 4);

	if(retval.float_retval != retval.float_retval) //test for nan
		retval.float_retval = 0;

	return retval.float_retval;
}
