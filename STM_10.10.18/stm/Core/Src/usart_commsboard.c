#include "usart_commsboard.h"
#include "calculate_ph.h"
#include "ioport.h"

qaqc_reply_struct * local_qaqc_reply;

uint8_t number_of_ph_electrodes = NUMBER_OF_PH_ELECTRODES;

uint8_t transmit_data_array[TRANSMIT_BUFFER_SIZE];
uint8_t received_data_array[COMMAND_MAXIMUM_SIZE];

volatile bool perform_ocean_measurement_routine_flag = false;
volatile bool perform_fresh_measurement_routine_flag = false;
volatile bool perform_vf_measurement_routine_flag = false;
volatile bool perform_existing_measurement_routine_flag = false;
volatile bool continue_measurement_flag = true;
volatile bool perform_precondition_ph_electrodes_flag = false;
volatile int precondition_ph_case = 1;
volatile bool immersion_status_requested_flag = false;
volatile bool immersed_flag = false;
volatile float pre_conditioning_potential = 0;
volatile bool already_scanning = false;

volatile bool scan_internal_interval_flag = false;
volatile bool scan_interval_flag = false;
volatile bool scan_long_interval_flag = false;
volatile bool scan_short_interval_flag = false;
volatile bool scan_external_interval_flag = false;
volatile bool scan_first_flag = false;

extern volatile usart_timer_struct command_received_global;
uint32_t immersion_adc_difference = 0;
volatile uint16_t precon_diagnostic = 0, failure_diagnostic = 0, parameters_diagnostic = 0;
float peak_potential_1 = 0, peak_potential_2 = 0, peak_potential_3 = 0, peak_potential_4 = 0,  peak_potential_5 = 0;

uint32_t first_peak_potential = 0;
bool salinity_out_of_range = false;
float signal_scatter = 0;


void clear_received_buffer(void)
{
	memset(received_data_array, 0, sizeof(received_data_array));
	command_received_global.received_count = 0;
	command_received_global.command_size = 0;
	command_received_global.command_byte = 0;
	command_received_global.command_option_byte = 0;
}

void process_commands(void)
{
	command_received_global.command_size = received_data_array[0];
	command_received_global.command_byte = received_data_array[1];
	command_received_global.command_size_with_crc = command_received_global.command_size + 2;

	//command_received_global.command_size = received_data_array[0];
	if (received_data_array[command_received_global.command_size] == crc_get_reversed_first_byte(received_data_array, command_received_global.command_size))
	{
		// 1st crc byte matches, check the second
		if (!(received_data_array[command_received_global.command_size + 1] == crc_get_reversed_second_byte()))
		{
		// second byte doesnt match (even though first byte was a match), clear read buffer
		// ie the received command is discarded
			clear_received_buffer();
			return;
		}
	}
	else
	{
		// first byte doesnt match, clear read buffer
		//		 ie the received command is discarded
		clear_received_buffer();
		return;
	}

	if (command_received_global.command_size == 0x05 && command_received_global.command_byte == 0x41)
	{
		if (received_data_array[1]== 0x41 && received_data_array[2]==0x4e && received_data_array[3]== 0x42 && received_data_array[4]==0x0d)
		{
			anb_sensors_interface_board_connected = true;
			reply_echo_data_board();
		}
	}

	if(command_received_global.command_byte == COMMAND_TEXT_SCAN_START)
	{
		// if its start command, check the next byte for option
		command_received_global.command_option_byte = received_data_array[2];

		if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_OCEAN_FIRST)
		{
			perform_ocean_measurement_routine_flag = true;
			scan_internal_interval_flag = true;
			scan_first_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_SI_OCEAN_FIRST)
		{
			perform_ocean_measurement_routine_flag = true;
			scan_short_interval_flag = true;
			scan_external_interval_flag = true;
			scan_first_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_LI_OCEAN_FIRST)
		{
			perform_ocean_measurement_routine_flag = true;
			scan_long_interval_flag = true;
			scan_external_interval_flag = true;
			scan_first_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_FRESH_FIRST)
		{
			perform_fresh_measurement_routine_flag = true;
			scan_internal_interval_flag = true;
			scan_first_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_SI_FRESH_FIRST)
		{
			perform_fresh_measurement_routine_flag = true;
			scan_short_interval_flag = true;
			scan_external_interval_flag = true;
			scan_first_flag = true;
		}
		if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_LI_FRESH_FIRST)
		{
			perform_fresh_measurement_routine_flag = true;
			scan_long_interval_flag = true;
			scan_external_interval_flag = true;
			scan_first_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_VF_FIRST)
		{
			perform_vf_measurement_routine_flag = true;
			scan_internal_interval_flag = true;
			scan_first_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_SI_VF_FIRST)
		{
			perform_vf_measurement_routine_flag = true;
			scan_short_interval_flag = true;
			scan_external_interval_flag = true;
			scan_first_flag = true;
		}
		if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_LI_VF_FIRST)
		{
			perform_vf_measurement_routine_flag = true;
			scan_long_interval_flag = true;
			scan_external_interval_flag = true;
			scan_first_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_OCEAN)
		{
			perform_ocean_measurement_routine_flag = true;
			scan_internal_interval_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_SI_OCEAN)
		{
			perform_ocean_measurement_routine_flag = true;
			scan_short_interval_flag = true;
			scan_external_interval_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_LI_OCEAN)
		{
			perform_ocean_measurement_routine_flag = true;
			scan_long_interval_flag = true;
			scan_external_interval_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_FRESH)
		{
			perform_fresh_measurement_routine_flag = true;
			scan_internal_interval_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_SI_FRESH)
		{
			perform_fresh_measurement_routine_flag = true;
			scan_short_interval_flag = true;
			scan_external_interval_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_LI_FRESH)
		{
			perform_fresh_measurement_routine_flag = true;
			scan_long_interval_flag = true;
			scan_external_interval_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_VF)
		{
			perform_vf_measurement_routine_flag = true;
			scan_internal_interval_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_SI_VF)
		{
			perform_vf_measurement_routine_flag = true;
			scan_short_interval_flag = true;
			scan_external_interval_flag = true;
		}
		else if(command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_LI_VF)
		{
			perform_vf_measurement_routine_flag = true;
			scan_long_interval_flag = true;
			scan_external_interval_flag = true;
		}
		else if((command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_EXISTING_FIRST) || (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_START_EXISTING_FIRST))
		{
			perform_existing_measurement_routine_flag = true;
			if(eeprom_read_salinity_range() == 1)
				perform_fresh_measurement_routine_flag = true;
			else if(eeprom_read_salinity_range() == 2)
				perform_ocean_measurement_routine_flag = true;
			if(eeprom_read_salinity_range() == 3)
				perform_vf_measurement_routine_flag = true;

			scan_internal_interval_flag = true;
			scan_first_flag = true;
		}
		else if((command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SCAN_EXISTING) || (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_START_EXISTING))
		{
			perform_existing_measurement_routine_flag = true;
			if(eeprom_read_salinity_range() == 1)
				perform_fresh_measurement_routine_flag = true;
			else if(eeprom_read_salinity_range() == 2)
				perform_ocean_measurement_routine_flag = true;
			if(eeprom_read_salinity_range() == 3)
				perform_vf_measurement_routine_flag = true;

			scan_internal_interval_flag = true;
		}
		reply_echo_data();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SCAN_STOP)
	{
		// stop command	so drop all current measurements
		continue_measurement_flag = false;
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SCAN_N_NO_OF_PH_ELECTRODES)
	{
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else
			eeprom_write_scan_n_no_of_ph_electrodes(received_data_array[2]);

		reply_scan_n_number();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_EXTERNAL_BAUD_RATE)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
			reply_external_baud_rate();
		else
		{
			set_external_baud_rate_setting();
			reply_external_baud_rate();
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SENSOR_MODE)
	{
		// sensor mode
		// check the next byte for option - get / set
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else
			set_sensor_mode(command_received_global.command_option_byte);

		reply_sensor_mode();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SENSOR_NAME)
	{
		// sensor name
		// check the next byte for option - get (1 byte following command byte) / set (6 bytes following command byte)
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_size == 3)
		{
			// if command length 3, command could potentially be 'get'
			//		so check for get byte against byte 2 after length byte
			if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
				reply_sensor_name();
		}
		else if (command_received_global.command_size <= (2 + DEVICE_SENSOR_NAME_LENGTH))
		{
			// set the serial number
			//		6 ascii characters are expected
			// set new serial number to eeprom
			set_sensor_name();
			// send the serial number back to confirm its been written
			reply_sensor_name();
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SENSOR_SUB_NAME)
	{
		// sensor sub name
		// check the next byte for option - get (1 byte following command byte) / set (6 bytes following command byte)
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_size == 3)
		{
			// if command length 3, command could potentially be 'get'
			//		so check for get byte against byte 2 after length byte
			if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
			{
				reply_sensor_sub_name();
			}
		}
		else if (command_received_global.command_size <= (2 + DEVICE_SENSOR_SUB_NAME_LENGTH))
		{
			set_sensor_sub_name();
			reply_sensor_sub_name();
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SENSOR_ARRAY_INTERVAL)
	{
		// sensor array interval
		// check the next byte for option - get / set
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
			reply_sensor_array_interval();
		else
		{
			// if not zero for option, its the value and has to be set as seconds of delay
			// setting sensor array interval (if 1 to 14 set as 0 and valid values from 15 to 240)
			// else try to set the value received
			//		setting function checks for validity and ignores if not supported
			//		so just call the setting function, if invalid value
			//		clearing buffer always done at the end of the function
			set_sensor_array_interval();
			reply_sensor_array_interval();
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SENSOR_AUTON_DELAY)
	{
		// sensor autonomous start delay
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_size == 3)
		{
			// if command length 3, command could potentially be 'get'
			//		so check for get byte against byte 2 after length byte
			if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
				reply_autonomous_start_delay();
			else
			{}
		}
		else if (command_received_global.command_size == 4)
		{
			// set the auton delay
			//		byte 3 is hours
			//		byte 4 is minutes
			//		valid values are:
			//			0 minutes, or
			//			5 minutes to 24 hours
			//		if beyond the valid range,
			//			if 1 to 4 minutes will forced to 5 minutes
			//			if >24 hours forced to 24 hours
			set_autonomous_start_delay();
			reply_autonomous_start_delay();
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SENSOR_COMMUNICATION_STYLE)
	{
		// sensor communication style
		// check the next byte for option - get / set
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_ASCII_COMMUNICATION_STYLE)
			eeprom_write_sensor_communication_style(COMMAND_OPTION_TEXT_ASCII_COMMUNICATION_STYLE);

		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_MOBUS_COMMUNICATION_STYLE)
			eeprom_write_sensor_communication_style(COMMAND_OPTION_TEXT_MOBUS_COMMUNICATION_STYLE);

		reply_sensor_communication_style();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SENSOR_SERIAL_NUMBER)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_size == 3)
		{
			// if command length 3, command could potentially be 'get'
			if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
				reply_serial_number();
		}
		else if (command_received_global.command_size == (2 + DEVICE_SERIAL_NUMBER_LENGTH))
		{
			set_sensor_serial_number();
			// send the serial number back to confirm its been written
			reply_serial_number();
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SENSOR_IMMERSION_RULE)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_USE_IMMERSION)
			set_system_immersion_rule();
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_IGNORE_IMMERSION)
			set_system_immersion_rule();

		reply_system_immersion_rule();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SALINITY)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{
			// TODO: implement reply with last salinity reading
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_PH)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{
			// TODO: implement reply with last ph reading
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_TEMPERATURE)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{
			qaqc_reply.calculated_temperature = system_get_temperature();
			reply_get_temperature();
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_IMMERSED_DRY_STATUS)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
			reply_immersed_dry_status();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SQUAREWAVE_DATA)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_ON)
			set_square_wave_data_save_status();
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_OFF)
			set_square_wave_data_save_status();

		reply_square_wave_data_save_status();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_PH_HEARTBEAT_STYLE)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_ALL_HEARTBEAT)
			set_ph_heart_beat_style_status();
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_PH_ONLY)
			set_ph_heart_beat_style_status();

		reply_ph_heartbeat_style_status();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_MANUFACTURER)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
			reply_manufacturer_name();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_DRIVER_VERSION)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
			reply_driver_version();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SYSTEM_MODBUS_ADDRESS)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else if(command_received_global.command_byte != 0)
			set_system_modbus_address();

		reply_system_modbus_address();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_QAQC_DATA)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
			reply_qaqc_data();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_ARRAY_HEALTH)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
			reply_array_health();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_PH_OUTPUT_STYLE)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_PH_OUTPUT_LONG_ON)
			set_sensor_ph_output_style();
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_PH_OUTPUT_LONG_OFF)
			set_sensor_ph_output_style();

		reply_sensor_ph_output_style();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_TEMPERATURE_CALIBRATION)
	{
		// sensor temperature calibration
		if (command_received_global.command_size == (2 + DEVICE_TEMPERATURE_CALIBRATION_LENGTH))
		{	// set the temperature calibration
			//	12 characters are expected
			// write new temperature calibration numbers to eeprom
			set_sensor_temperature_calibration();
		}
		else
		{}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_ANB_OR_ALTERNATE_HEAD)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else if (command_received_global.command_option_byte == COMMAND_OPTION_ANBSENSORS_HEAD)
			set_anbsensors_or_alternate_head();
		else if (command_received_global.command_option_byte == COMMAND_OPTION_VALEPORT_HEAD)
			set_anbsensors_or_alternate_head();

		reply_anbsensors_or_alternate_head();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_INTERFACE)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_INTERFACE_ANB)
			set_interface();
		else if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_INTERFACE_WM)
			set_interface();

		reply_interface();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_CLEAR_PH_ARRAY)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else
		{
			clear_ph_array();
			reply_echo_data();
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_CONDITION_REF_OCEAN)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else
		{
			//condition_ref_ocean(); not for this version
			reply_echo_data();
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_CONDITION_REF_FRESHWATER)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else
		{
			//condition_ref_freshwater(); not for this version
			reply_echo_data();
		}
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_PRECONDITION_PH_ELECTRODES)
	{
		command_received_global.command_option_byte = received_data_array[2];
		avg_ph_peak_potential_stored = (bool)eeprom_read_byte_with_checks(DEVICE_AVG_PH_PEAK_POTENTIAL_STORED, 0, 1, 0);
		preconditioning_done = (bool)eeprom_read_byte_with_checks(DEVICE_PRECONDITIONING_DONE, 0, 1, 0);
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}

		reply_echo_data();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_EE_RESET)
	{
		reset_eeprom_settings();
		reply_echo_data();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_NO_STORE)
	{
		store_last_electrode_number = false;
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_FIRMWARE_VERSION)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else if(command_received_global.command_option_byte == 0x01) //set interface version bytes
		{
			eeprom_write_interface_version(received_data_array[3], received_data_array[4]);
		}
		reply_firmware_version(); //full reply in either case
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_FAST_PROFILING)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else
			set_fast_profiling();

		reply_fast_profiling();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_ABRADED)
	{
		clear_electrode_health();
		reset_electrode_gains_flag = true; //can't set gains yet as don't know what scan it's going to be
		clear_ph_array();
		clear_peak_potential_offsets();
		eeprom_write_loops_since_last_correction(0);
		avg_ph_peak_potential_stored = false;
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_AVG_PH_PEAK_POTENTIAL_STORED, avg_ph_peak_potential_stored);
		preconditioning_done = false;
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PRECONDITIONING_DONE, preconditioning_done);
		//reset_frequencies();
		reset_pulse_amplitudes();
		reset_qaqcs();
		reset_peaks_loops();
		reset_peak_potentials();
		reset_fresh_electrodes();
		eeprom_write_loops_since_last_event(0);
		event_passed = false;
		reset_clipped_at_gain();
		last_passed_ph_qaqc = 0;
		eeprom_save_last_passed_ph_qaqc();
		avg_iref_peak_potential = 0;
		eeprom_save_avg_iref_peak_potential();
		abraded = true;
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_ABRADED, abraded);
		reply_echo_data();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_USE_SDCARD)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else
			set_use_sdcard();

		reply_use_sdcard();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_POWER)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else
			set_power();

		reply_power();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_SALINITY_RANGE)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else
			set_salinity_range();

		reply_salinity_range();
	}
	else if (command_received_global.command_byte == COMMAND_TEXT_PHS_PER_LOOP)
	{
		command_received_global.command_option_byte = received_data_array[2];
		if (command_received_global.command_option_byte == COMMAND_OPTION_TEXT_GET)
		{}
		else
			set_phs_per_loop();

		reply_phs_per_loop();
	}
	else{}


	// command processed, so clear the buffer
	//		or doesnt match any known commands
	//		no error handling in the communication been defined for implementation
	// so for now, just clear the received buffer
	clear_received_buffer();
}

void check_message(uint8_t command[], uint8_t Size)
{
	memcpy(received_data_array, command, Size);
	process_commands();
}

/**
* @brief Sends single frame of 12 bytes of data from the communication board
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202301121215 created - PV
*/
void send_one_dataframe_as_spi_slave(uint8_t * dataframe)
{
	uint8_t byte_count;

	// put data for the spi transmit

	spi1_enable();

	for (byte_count = 0; byte_count < SPI_SLAVE_DATA_FRAME_SIZE_IN_BYTES; byte_count++)
	{
		// Put a byte on MISO and wait for a byte from MOSI
		spi1_send_byte(dataframe[byte_count]);
		if (byte_count <= 0)
		{
			// set data ready flag low
			// needs to be set back high after one byte is read on the spi
			ioport_set_pin_level(spi_slave_data_ready_flag, IOPORT_PIN_LEVEL_LOW);
		}
		while( !(SPI1->SR & (SPI_SR_BSY)) ); //SG - i believe TXE becomes 1 as soon as the byte is sent, but we
											 //want to wait til it is read. so wait while busy. this needs refining

		while( !(SPI1->SR & (SPI_SR_TXE)) );  // wait until transmit buffer empty
		while( (SPI1->SR & (SPI_SR_BSY)) );

		if (byte_count <= 0)
		{
			// set data ready flag high
			//		needs to be turned off after one byte is read on the spi
			ioport_set_pin_level(spi_slave_data_ready_flag, IOPORT_PIN_LEVEL_HIGH);
		}
	}
}

/**
* @brief Converts double to 3 BCD bytes for passing DAC dc potential to communication board.
*
* @author Palaniappan Valliappan
* @version 1.2
* @date 202301221720 created - PV
* @date 202301231020 modified - PV - double to BCD rounding error, so modified to convert from int instead of double
* @date 202302200955 modified - PV - added checks for negative and place 0xF for first nibble of first byte as Brandon wanted for negative numbers
*/
void convert_dac_potential_to_custom_bcd(uint8_t * output_data, double value)
{
	double temp_double = fabs(value) * 10;
	int32_t temp_int_1 = (int32_t) temp_double;
	int32_t temp_int_2 = 0;
	double temp_whole_number = 0;
	double temp_fraction = 0;

	temp_fraction = modf(temp_double, &temp_whole_number);
	if (temp_fraction > 0.9)
	{
		// check if its greater than 0.9
		//		if yes, increment the first decimal
		// 898.999... would convert to BCD of 899.0
		// so increment to round up
		temp_int_1 = temp_whole_number + 1;
	}

	// third byte
	//	anything less than 0 and greater than or equal to 0.1 in third byte
	//		least significant bits
	temp_int_2 = temp_int_1 / 10;
	*(output_data + 2) = ((int8_t) (temp_int_1 - (temp_int_2 * 10)));
	//	anything less than 10 and greater than or equal to 1 in third byte
	//		most significant bits
	temp_int_1 = temp_int_2;
	temp_int_2 = temp_int_1 / 10;
	*(output_data + 2) = (*(output_data + 2)) + (((int8_t) (temp_int_1 - (temp_int_2 * 10))) << 4);

	// set second byte
	//	anything less than 100 and greater than or equal to 10 in second byte
	//		least significant bits
	temp_int_1 = temp_int_2;
	temp_int_2 = temp_int_1 / 10;
	*(output_data + 1) = ((int8_t) (temp_int_1 - (temp_int_2 * 10)));
	//	anything less than 1000 and greater than or equal to 100 in second byte
	//		most significant bits
	temp_int_1 = temp_int_2;
	temp_int_2 = temp_int_1 / 10;
	*(output_data + 1) = (*(output_data + 1)) + (((int8_t) (temp_int_1 - (temp_int_2 * 10))) << 4);

	// set first byte
	//	anything greater than or equal to 1000 in first byte
	//		least significant bits
	temp_int_1 = temp_int_2;
	temp_int_2 = temp_int_1 / 10;
	*output_data = (int8_t) (temp_int_1 - (temp_int_2 * 10));
	//  cannot be higher than 5 due to the board supply being 5V, so ignoring
	//		values higher than that, ie.. the most significant bits

	// set sign, first nibble of most significant byte
	if (value < 0)
	{
		*output_data = *output_data + 0xF0;
	}
}

/**
* @brief Converts double to 2 BCD bytes for passing DAC dc potential to communication board during QAQC reply.
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202302201040 created - PV
*/
void convert_dac_potential_to_custom_bcd_2_byte_qaqc(uint8_t * output_data, double value)
{
	// This function would only return a 2 byte value ignoring sign and anything more than 999.9
	//		ie only output 000.0 to 999.9 in 2 BCD bytes
	double temp_double = fabs(value) * 10;
	int32_t temp_int_1 = (int32_t) temp_double;
	int32_t temp_int_2 = 0;
	double temp_whole_number = 0;
	double temp_fraction = 0;

	temp_fraction = modf(temp_double, &temp_whole_number);
	if (temp_fraction > 0.9)
	{
		// check if its greater than 0.9
		//		if yes, increment the first decimal
		// 898.999... would convert to BCD of 899.0
		// so increment to round up
		temp_int_1 = temp_whole_number + 1;
	}

	// set second byte
	//	anything less than 0 and greater than or equal to 0.1 in second byte
	//		least significant bits
	temp_int_2 = temp_int_1 / 10;
	*(output_data + 1) = ((int8_t) (temp_int_1 - (temp_int_2 * 10)));
	//	anything less than 10 and greater than or equal to 1 in second byte
	//		most significant bits
	temp_int_1 = temp_int_2;
	temp_int_2 = temp_int_1 / 10;
	*(output_data + 1) = (*(output_data + 1)) + (((int8_t) (temp_int_1 - (temp_int_2 * 10))) << 4);

	// set first byte
	//	anything less than 100 and greater than or equal to 10 in first byte
	//		least significant bits
	temp_int_1 = temp_int_2;
	temp_int_2 = temp_int_1 / 10;
	*output_data = ((int8_t) (temp_int_1 - (temp_int_2 * 10)));
	//	anything less than 1000 and greater than or equal to 100 in second byte
	//		most significant bits
	temp_int_1 = temp_int_2;
	temp_int_2 = temp_int_1 / 10;
	*output_data = *output_data + (((int8_t) (temp_int_1 - (temp_int_2 * 10))) << 4);
}

void send_data_as_spi_slave(
		uint16_t * adc_mark_data,
		uint16_t * adc_space_data,
		float * adc_mark_space_array_1_data,
		float * adc_mark_space_array_2_data,
		float * dac_output_voltage_data,
		uint16_t adc_data_size,
		bool last_scan_of_last_electrode)
{
	uint8_t dataframe[SPI_SLAVE_DATA_FRAME_SIZE_IN_BYTES];

	// send payload data frame, ie first data frame
	//		add the payload size
	// set all bytes to zero
	memset(dataframe, 0, sizeof(dataframe));
	// shift right to get most significant byte
	dataframe[2] = (uint8_t) (adc_data_size >> 8);
	// add with 0xff to get the least significant byte
	dataframe[3] = (uint8_t) (adc_data_size & 0x00ff);

	// send the dataframe
	send_one_dataframe_as_spi_slave(dataframe);

	// send all the data
	for (int i = 0; i < adc_data_size; i++)
	{
		// generate the row for sending
		dataframe[0] = 0xf0; //0xf0

		// put BCD conversion for the DAC value used to obtain the measurements
		convert_dac_potential_to_custom_bcd(&dataframe[1], dac_output_voltage_data[i]);

		// Mark value
		dataframe[4] = adc_mark_data[i] >> 8;
		dataframe[5] = adc_mark_data[i] ;

		// Space value
		dataframe[6] = adc_space_data[i] >> 8;
		dataframe[7] = adc_space_data[i] ;

		uint16_t adc_mark_space_array_1_int = (uint16_t)adc_mark_space_array_1_data[i];
		dataframe[8] = adc_mark_space_array_1_int >> 8;
		dataframe[9] = adc_mark_space_array_1_int;

		uint16_t adc_mark_space_array_2_int = (uint16_t)adc_mark_space_array_2_data[i];
		dataframe[10] = adc_mark_space_array_2_int >> 8;
		dataframe[11] = adc_mark_space_array_2_int;

		// send the dataframe
		send_one_dataframe_as_spi_slave(dataframe);
	}

	// send last data frame
	//		add the append or end symbol
	//		to let the communication board know its last or more data available
	memset(dataframe, 0, sizeof(dataframe));
	if (last_scan_of_last_electrode)
	{
		// if last scan, put end of transmission symbol
		for (int j = 3; j < 6; j++)
		{
			dataframe[j] = SPI_DATA_FRAME_END_SYMBOL;
		}
	}
	else
	{
		// if not last scan, put append symbol
		for (int j = 3; j < 6; j++)
		{
			dataframe[j] = SPI_DATA_FRAME_APPEND_SYMBOL;
		}
	}

	send_one_dataframe_as_spi_slave(dataframe);

	//while((UART_TXD0_DEBUG_GPIO_Port->IDR & UART_TXD0_DEBUG_Pin) == 0x00u);

	while (ioport_get_pin_level(spi_slave_data_already_read_flag))
	{
		// do nothing until the data already read flag from communication board
		// TODO: spi data transfer - to put a time out, just in case the communication board goes into a weird state
	}
}

/**
* @brief Echo the input command to see print out on terminal program from communication board
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202301061510 created - PV
*/
void reply_echo_data(void)
{
	uint8_t character_number = received_data_array[0];

	for (int i = 0; i < (character_number + 2); i++)
	{
		uart1_send_byte(received_data_array[i]);
	}
}


void reply_echo_data_board(void)
{
	// command identifier
	transmit_data_array[1] = received_data_array[1];
	transmit_data_array[2] = received_data_array[2];
	transmit_data_array[3] = received_data_array[3];

	transmit_data_array[4] = 'S';
	transmit_data_array[5] = 'T';
	transmit_data_array[6] = 'M';
	transmit_data_array[7] = STM_BOARD_REVISION;

	command_received_global.reply_size = 8;

	// transmit data has been assembled, send it
	send_reply();
}

/**
* @brief Send the data in the transmit buffer.
*
* This function is only called when the data is ready and the number of bytes to
* transmit has been updated to the global structure.
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202301181654 created - PV - refactored as a separate function all replies can use
*/
void send_reply(void)
{
	// update the transmit size with crc bytes
	command_received_global.reply_size_with_crc = command_received_global.reply_size + 2;
	// length of the reply
	transmit_data_array[0] = command_received_global.reply_size;
	// calculate the crc
	transmit_data_array[command_received_global.reply_size] = crc_get_reversed_first_byte(transmit_data_array, command_received_global.reply_size);
	transmit_data_array[command_received_global.reply_size + 1] = crc_get_reversed_second_byte();

	uart1_send_bytes(transmit_data_array, command_received_global.reply_size_with_crc);
}

void reply_serial_number(void)
{
	// command identifier
	transmit_data_array[1] = COMMAND_TEXT_SENSOR_SERIAL_NUMBER;

	// read serial number, length is taken from defined length of serial number #define
	eeprom_read_sensor_serial_number(&transmit_data_array[2]);

	// size = length byte + command byte + serial number length
	command_received_global.reply_size = 2 + DEVICE_SERIAL_NUMBER_LENGTH;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_sensor_name(void)
{
	// command identifier
	transmit_data_array[1] = COMMAND_TEXT_SENSOR_NAME;

	// read serial number, length is taken from defined length of serial number #define
	eeprom_read_sensor_name(&transmit_data_array[2]);

	// size = length byte + command byte + serial number length
	command_received_global.reply_size = 2 + DEVICE_SENSOR_NAME_LENGTH;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_sensor_sub_name(void)
{
	// command identifier
	transmit_data_array[1] = COMMAND_TEXT_SENSOR_SUB_NAME;

	// read serial number, length is taken from defined length of serial number #define
	eeprom_read_sensor_sub_name(&transmit_data_array[2]);

	// size = length byte + command byte + serial number length
	command_received_global.reply_size = 2 + DEVICE_SENSOR_SUB_NAME_LENGTH;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_manufacturer_name(void)
{
	int name_length;
	// command identifier
	transmit_data_array[1] = COMMAND_TEXT_MANUFACTURER;
	// TODO: get sub name from eeprom and delete the following being sent now
	transmit_data_array[2] = 'A';
	transmit_data_array[3] = 'N';
	transmit_data_array[4] = 'B';
	transmit_data_array[5] = 'S';
	transmit_data_array[6] = 'e';
	transmit_data_array[7] = 'n';
	transmit_data_array[8] = 's';
	transmit_data_array[9] = 'o';
	transmit_data_array[10] = 'r';
	transmit_data_array[11] = 's';

	name_length = 10;
	// check that the name length is 10 or less
	// TODO: add #define for manufacturer name length, once values are confirmed
	if (name_length > 10)
	{
		name_length = 10;
	}
	// size = length byte + command byte + manufacturer name (can be up to 10 ascii characters)
	command_received_global.reply_size =  2 + name_length;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_external_baud_rate(void)
{
	transmit_data_array[1] = COMMAND_TEXT_EXTERNAL_BAUD_RATE;
	transmit_data_array[2] = eeprom_read_external_baud_rate_setting();
	command_received_global.reply_size = 3;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_scan_n_number(void)
{
	transmit_data_array[1] = COMMAND_TEXT_SCAN_N_NO_OF_PH_ELECTRODES;
	transmit_data_array[2] = eeprom_read_scan_n_no_of_ph_electrodes();
	command_received_global.reply_size = 3;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_sensor_communication_style(void)
{
	transmit_data_array[1] = COMMAND_TEXT_SENSOR_COMMUNICATION_STYLE;
	transmit_data_array[2] = eeprom_read_sensor_communication_style();
	command_received_global.reply_size = 3;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_sensor_mode(void)
{
	transmit_data_array[1] = COMMAND_TEXT_SENSOR_MODE;
	transmit_data_array[2] = eeprom_read_sensor_mode_setting();
	command_received_global.reply_size = 3;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_autonomous_start_delay(void)
{
	// command identifier
	transmit_data_array[1] = COMMAND_TEXT_SENSOR_AUTON_DELAY;

	// get sensor auton delay from eeprom
	uint16_t temp_hours_minutes = eeprom_read_sensor_autonomous_start_delay_setting();
	transmit_data_array[2] = temp_hours_minutes >> 8;
	transmit_data_array[3] = temp_hours_minutes;

	// size = length byte + command byte + 0xnn (hour) 0xnn (minutes) / 2 bytes
	// valid values are 0 minutes, 5 minutes to 24 hours
	command_received_global.reply_size =  4;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_sensor_array_interval(void)
{
	transmit_data_array[1] = COMMAND_TEXT_SENSOR_ARRAY_INTERVAL;
	transmit_data_array[2] = eeprom_read_sensor_array_interval_setting();
	command_received_global.reply_size = 3;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_driver_version(void)
{
	int name_length;
	// command identifier
	transmit_data_array[1] = COMMAND_TEXT_DRIVER_VERSION;
	// TODO: get driver version from eeprom and delete the following being sent now
	uint8_t temp[] = {FIRMWARE_VERSION};
	name_length = sizeof(temp);

	// max number of characters allowed is 16 (which includes the carriage return 0x0d thats
	//		needed as the last character of any length firmware version)
	// TODO: add #define for length
	if (name_length >= 16)
	{
		// if over the max length truncate it
		name_length = 16;
		// also set last char to 0x0d
		temp[15] = 0x0d;
	}

	// transmitted reply starts with length and command byte, so variable starts at 2
	int transmit_count = 2;
	for (int i = 0; i < name_length; i++)
	{
		// check if characters only between 32 and 126, for displaying on terminal
		if (temp[i] > 31)
		{
			if (temp[i] < 127)
			{
				transmit_data_array[transmit_count] = temp[i];
				transmit_count = transmit_count + 1;
			}
		}
	}

	if (transmit_count < ((16 - 1) + 2))
	{
		// add the carriage return at the end
		transmit_data_array[transmit_count] = 0x0d;
		transmit_count = transmit_count + 1;
	}

	// size = length byte + command byte + driver board firmware version (can be up to 16 ascii characters)
	command_received_global.reply_size = transmit_count;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_system_modbus_address(void)
{
	transmit_data_array[1] = COMMAND_TEXT_SYSTEM_MODBUS_ADDRESS;
	transmit_data_array[2] = eeprom_read_system_modbus_address();
	command_received_global.reply_size = 3;

	// transmit data has been assembled, send it
	send_reply();
}

void set_system_modbus_address(void)
{
	uint8_t data;
	data = received_data_array[2];
	eeprom_write_system_modbus_address(data);
}

void reply_system_immersion_rule(void)
{
	transmit_data_array[1] = COMMAND_TEXT_SENSOR_IMMERSION_RULE;
	transmit_data_array[2] = eeprom_read_immersion_rule();
	transmit_data_array[3] = eeprom_read_immersion_rule_interval();
	command_received_global.reply_size = 4;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_sensor_ph_output_style(void)
{
	transmit_data_array[1] = COMMAND_TEXT_PH_OUTPUT_STYLE;
	transmit_data_array[2] = eeprom_read_ph_output_style_setting();
	command_received_global.reply_size = 3;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_array_health(void)
{
	transmit_data_array[1] = COMMAND_TEXT_ARRAY_HEALTH;
	transmit_data_array[2] = eeprom_read_array_health();
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_stop_scanning(void)
{
	transmit_data_array[1] = COMMAND_TEXT_SCAN_STOP;
	transmit_data_array[2] = 0;
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_square_wave_data_save_status(void)
{
	// command identifier
	transmit_data_array[1] = COMMAND_TEXT_SQUAREWAVE_DATA;
	transmit_data_array[2] = eeprom_read_square_wave_data_save_status();

	if (transmit_data_array[2] == COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_ON)
		command_received_global.square_wave_data_save_flag = true;
	else if (transmit_data_array[2] == COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_OFF)
		command_received_global.square_wave_data_save_flag = false;

	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_ph_heartbeat_style_status(void)
{
	// command identifier
	transmit_data_array[1] = COMMAND_TEXT_PH_HEARTBEAT_STYLE;
	// get square wave data save status from eeprom / bool in ram
	//		if using the bool in ram, that value gets read from eeprom on startup
	//		and gets updated when the status is changed and written to eeprom as well

	// get square wave data save status
	transmit_data_array[2] = eeprom_read_heartbeat_output_style_setting();
	if (transmit_data_array[2] == COMMAND_OPTION_TEXT_ALL_HEARTBEAT)
	{
		// on / save
		command_received_global.ph_heartbeat_style_flag = true;
	}
	else if (transmit_data_array[2] == COMMAND_OPTION_TEXT_PH_ONLY)
	{
		// off / dont save
		command_received_global.ph_heartbeat_style_flag = false;
	}

	// size = length byte + command byte + square wave data save status / 1 byte
	command_received_global.reply_size = 3;

	// transmit data has been assembled, send it
	send_reply();
}

void reply_qaqc_data_ready_to_be_read(void)
{
	// command identifier
	transmit_data_array[1] = COMMAND_TEXT_QAQC_DATA_READY_TO_BE_READ;

	// this is supposed the return anything from 0x01 to 0x04, to signify having up to 4 old qaqc data available
	//		until that is implemented, Brandon wanted it to return 0x01 (hard coded)
	//		ie now wouldn't remember the last 4, always the return the last and only one available!!
	transmit_data_array[2] = 0x01;

	// size = length byte + command byte + immersion rule status / 1 byte
	command_received_global.reply_size = 3;

	// transmit data has been assembled, send it
	send_reply();

	// set the qaqc data waiting to be read flag
	command_received_global.qaqc_data_waiting_to_read_flag = true;
}

void get_qaqc_reply_data_address(qaqc_reply_struct * qaqc_reply)
{
	local_qaqc_reply = qaqc_reply;
}

void reverse_for_endianness(uint8_t* destination_start_address, int32_t size_n)
{
	//size_t i;
	for (uint8_t i = 0; i < (size_n / 2); ++i)
	{
		uint8_t tmp = destination_start_address[i];
		destination_start_address[i] = destination_start_address[size_n - 1 - i];
		destination_start_address[size_n - 1 - i] = tmp;
	}
}

void int32_to_byte_array(uint8_t* destination_start_address, int32_t value)
{
	union {
		int32_t int_value;
		uint8_t bytes[4];
	} int32_to_bytes;
	int32_to_bytes.int_value = value;
	memcpy(destination_start_address, int32_to_bytes.bytes, 4);

	// byte order is reversed
	reverse_for_endianness(destination_start_address, 4);
}

void float_to_byte_array(uint8_t* destination_start_address, float value)
{
	union {
		float float_value;
		uint8_t bytes[4];
		} float_to_bytes;
	float_to_bytes.float_value = value;
	memcpy(destination_start_address, float_to_bytes.bytes, 4);

	// byte order is reversed
	reverse_for_endianness(destination_start_address, 4);
}

void reply_qaqc_data(void)
{
	command_received_global.reply_size = TRANSMIT_BUFFER_SIZE - 2;	// not including the 2 crc bytes
	transmit_data_array[0] = command_received_global.reply_size;
	transmit_data_array[1] = COMMAND_TEXT_QAQC_DATA;
	transmit_data_array[2] = local_qaqc_reply->electrode_number;

	uint8_t electrode_health_and_gain = (uint8_t)(local_qaqc_reply->electrode_health << 4) + local_qaqc_reply->gain;
	transmit_data_array[3] = electrode_health_and_gain;

	bool data_not_ready = false;

	float temp_value = (float) local_qaqc_reply->running_average_ph;
	if((local_qaqc_reply->running_average_ph == PH_DEFAULT_OUT_OF_RANGE_VALUE) || (local_qaqc_reply->array_health >= 3))
	{
		transmit_data_array[4] = 0x42;
		transmit_data_array[5] = 0xc7;
		transmit_data_array[6] = 0xff;
		transmit_data_array[7] = 0x7d;
		data_not_ready = true;
	}	
	else		
		float_to_byte_array(&transmit_data_array[4], temp_value);
	
	temp_value = (float) local_qaqc_reply->calculated_temperature;
	float_to_byte_array(&transmit_data_array[8], temp_value);

	temp_value = (float) local_qaqc_reply->salinity;
	if(salinity_out_of_range) // || data_not_ready)
	{
		transmit_data_array[12] = 0x42;
		transmit_data_array[13] = 0xc7;
		transmit_data_array[14] = 0xff;
		transmit_data_array[15] = 0x7d;
	}
	else
		float_to_byte_array(&transmit_data_array[12], temp_value);

	temp_value = (float) local_qaqc_reply->specific_conductivity;

	if(salinity_out_of_range) // || data_not_ready)//if salinity out of range then conductivity not valid either
	{
		transmit_data_array[16] = 0x42;
		transmit_data_array[17] = 0xc7;
		transmit_data_array[18] = 0xff;
		transmit_data_array[19] = 0x7d;
	}
	else
		float_to_byte_array(&transmit_data_array[16], temp_value);

	transmit_data_array[20] = local_qaqc_reply->array_health;
	transmit_data_array[21] = local_qaqc_reply->sensor_diag;
	transmit_data_array[22] = local_qaqc_reply->data_status;

	uint8_t temp_value2[2];

	transmit_data_array[23] = (uint8_t) (((uint16_t)local_qaqc_reply->start_potential) >> 8);
	transmit_data_array[24] = (uint8_t) ((uint16_t)local_qaqc_reply->start_potential);

	convert_dac_potential_to_custom_bcd_2_byte_qaqc(&temp_value2[0], local_qaqc_reply->peak_potential);
	local_qaqc_reply->peak_potential_reply = (uint16_t) ((uint16_t) (temp_value2[0] << 8)) + temp_value2[1];
	transmit_data_array[25] = temp_value2[0];
	transmit_data_array[26] = temp_value2[1];

	if(iref_scan)
		parameters_diagnostic = peak_potential_3; //sg!
	convert_dac_potential_to_custom_bcd_2_byte_qaqc(&temp_value2[0], parameters_diagnostic);
	local_qaqc_reply->parameters_diagnostic_reply = (uint16_t) ((uint16_t) (temp_value2[0] << 8)) + temp_value2[1];
	transmit_data_array[27] = temp_value2[0];
	transmit_data_array[28] = temp_value2[1];

	convert_dac_potential_to_custom_bcd_2_byte_qaqc(&temp_value2[0], avg_iref_peak_potential);
	local_qaqc_reply->avg_iref_peak_potential_reply = (uint16_t) ((uint16_t) (temp_value2[0] << 8)) + temp_value2[1];
	transmit_data_array[29] = temp_value2[0];
	transmit_data_array[30] = temp_value2[1];

	local_qaqc_reply->avg_iref_peak_potential = suspect_iref_peak_potential;
	transmit_data_array[31] = (uint8_t) (((uint16_t)local_qaqc_reply->avg_iref_peak_potential) >> 8);
	transmit_data_array[32] = (uint8_t) ((uint16_t)local_qaqc_reply->avg_iref_peak_potential);

	local_qaqc_reply->failure_diagnostic_reply = failure_diagnostic;
	transmit_data_array[33] = (uint8_t) ((local_qaqc_reply->failure_diagnostic_reply) >> 8);
	transmit_data_array[34] = (uint8_t) local_qaqc_reply->failure_diagnostic_reply;

	local_qaqc_reply->max_value_reply = (local_qaqc_reply->max_value);
	transmit_data_array[35] = (uint8_t) ((local_qaqc_reply->max_value_reply) >> 8);
	transmit_data_array[36] = (uint8_t) local_qaqc_reply->max_value_reply;

	local_qaqc_reply->first_peak_potential_reply = peak_potential_2;
	transmit_data_array[37] = (uint8_t) ((local_qaqc_reply->first_peak_potential_reply) >> 8);
	transmit_data_array[38] = (uint8_t) local_qaqc_reply->first_peak_potential_reply;

	local_qaqc_reply->qaqc_2_reply = (local_qaqc_reply->qaqc_2);
	transmit_data_array[39] = (uint8_t) ((local_qaqc_reply->qaqc_2_reply) >> 8);
	transmit_data_array[40] = (uint8_t) local_qaqc_reply->qaqc_2_reply;

	transmit_data_array[41] = local_qaqc_reply->useful_customer_data;

	temp_value = (float) local_qaqc_reply->actual_conductivity;

	if(salinity_out_of_range) // || data_not_ready)
	{
		transmit_data_array[42] = 0x42;
		transmit_data_array[43] = 0xc7;
		transmit_data_array[44] = 0xff;
		transmit_data_array[45] = 0x7d;
	}
	else
		float_to_byte_array(&transmit_data_array[42], temp_value);

	transmit_data_array[command_received_global.reply_size - 1] = local_qaqc_reply->last_qaqc; //so last byte before crc

	// transmit data has been assembled, send it
	send_reply();

	// set qaqc data waiting to be read flag to false
	command_received_global.qaqc_data_waiting_to_read_flag = false;
}

bool get_qaqc_data_waiting_to_read_flag_status(void)
{
	return command_received_global.qaqc_data_waiting_to_read_flag;
}

void set_square_wave_data_save_status(void)
{
	uint8_t data;

	data = received_data_array[2];
	// set square wave data save status to eeprom
	//		also put the updated value in bool in ram
	if (data == COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_ON)
	{
		// set it to on / save
		eeprom_write_square_wave_data_save_status(COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_ON);
	}
	else if (data == COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_OFF)
	{
		// set it to off / dont save
		eeprom_write_square_wave_data_save_status(COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_OFF);
	}
}

void set_ph_heart_beat_style_status(void)
{
	uint8_t data;
	// get the data / status byte
	// size = length byte + command byte + square wave save data status / 1 byte
	data = received_data_array[2];
	// set square wave data save status to eeprom
	//		also put the updated value in bool in ram
	if (data == COMMAND_OPTION_TEXT_ALL_HEARTBEAT)
	{
		// set it to on / save
		eeprom_write_heartbeat_output_style_setting(COMMAND_OPTION_TEXT_ALL_HEARTBEAT);
	}
	else if (data == COMMAND_OPTION_TEXT_PH_ONLY)
	{
		// set it to off / dont save
		eeprom_write_heartbeat_output_style_setting(COMMAND_OPTION_TEXT_PH_ONLY);
	}
	// everything else ignore!

	// read back the value to the global struct to know the status after write
	get_stored_heart_beat_values_from_eeprom_to_global();
}

bool get_ph_heart_beat_style_status(void)
{
	return command_received_global.ph_heartbeat_style_flag;
}

void set_external_baud_rate_setting(void)
{
	uint8_t data;
	// get the data / external baud rate setting
	// size = length byte + command byte + external baud rate setting / 1 byte
	data = received_data_array[2];
	// set external baud rate setting to eeprom
	// condition check
	//		1-9600, 2-14400, 3-19200, 4-28800, 5-38400, 6-56000, 7-57600, 8-115200
	//		so has to be 1 to 8, 8 is default
	if (data >= 1)
	{
		if (data <= 8)
		{
			// set it to eeprom
			eeprom_write_external_baud_rate_setting(data);
		}
	}
	// everything else ignore!
}

void set_sensor_array_interval(void)
{
	uint8_t data;
	// get the data / external baud rate setting
	// size = length byte + command byte + sensor array interval setting / 1 byte
	data = received_data_array[2];
	// set to eeprom
	// condition check
	//		valid values are 0, 15 to 255 minutes
	//		0 is used by communication board for reading the value,
	//		so any value from 1 to 14 received from the communication board is to set 0 minutes
	//		when read back, only 0 and 15 to 240 are returned
	//		0 is default
	if (data >= 0)
	{
		if (data < 10)
		{
			// set it to eeprom
			// default is 0, so used here - if that changes this needs changing to 0
			eeprom_write_sensor_array_interval_setting(DEVICE_ARRAY_INTERVAL_DEFAULT_VALUE);
		}
		else if (data <= 240)
		{
			// set it to eeprom
			eeprom_write_sensor_array_interval_setting(data);
		}
	}
	// everything else ignore!
}

void set_sensor_mode(int mode)
{
	eeprom_write_sensor_mode_setting(mode);
}

/**
* @brief Set autonomous start delay values.
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202302211528 created - PV
*/
void set_autonomous_start_delay(void)
{
	// size = length byte + command byte + 0xnn (hour) 0xnn (minutes) / 2 bytes

	// set sensor autonomous start delay
	eeprom_write_sensor_autonomous_start_delay_setting(received_data_array[2], received_data_array[3]);
}

void set_sensor_serial_number(void)
{
	// size = length byte + command byte + serial number / 6 bytes

	// set sensor serial number to eeprom
	eeprom_write_sensor_serial_number(&received_data_array[2]);
}

void set_sensor_ph_output_style(void)
{
	uint8_t data;

	data = received_data_array[2];
	// set sensor mode to eeprom
	if (data == COMMAND_OPTION_TEXT_PH_OUTPUT_LONG_ON)
	{
		// set it to long ph
		eeprom_write_ph_output_style_setting(COMMAND_OPTION_TEXT_PH_OUTPUT_LONG_ON);
	}
	else if (data == COMMAND_OPTION_TEXT_PH_OUTPUT_LONG_OFF)
	{
		// set it to short ph
		eeprom_write_ph_output_style_setting(COMMAND_OPTION_TEXT_PH_OUTPUT_LONG_OFF);
	}
}

void set_system_immersion_rule(void)
{
	uint8_t rule;
	uint8_t interval;

	rule = received_data_array[2];
	interval = received_data_array[3];

	eeprom_write_immersion_rule(rule);
	eeprom_write_immersion_rule_interval(interval);
}

void set_fast_profiling(void)
{
	eeprom_write_fast_profiling(received_data_array[2]);
}

void set_use_sdcard(void)
{
	eeprom_write_use_sdcard(received_data_array[2]);
}

void set_power(void)
{
	eeprom_write_power(received_data_array[2]);
}

void set_salinity_range(void)
{
	eeprom_write_salinity_range(received_data_array[2]);
}

void set_phs_per_loop(void)
{
	eeprom_write_phs_per_loop(received_data_array[2]);
}

void get_stored_values_from_eeprom_to_global(void)
{
	uint8_t value = eeprom_read_square_wave_data_save_status();
	if (value == COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_ON)
	{
		command_received_global.square_wave_data_save_flag = true;
	}
	else
	{
		command_received_global.square_wave_data_save_flag = false;
	}
}

void set_sensor_temperature_calibration(void)
{
	eeprom_write_temperature_calibration_number(&received_data_array[2]);
}

void set_sensor_name(void)
{
	eeprom_write_sensor_name(&received_data_array[2]);
}

void set_sensor_sub_name(void)
{
	eeprom_write_sensor_sub_name(&received_data_array[2]);
}

void get_stored_heart_beat_values_from_eeprom_to_global(void)
{
	uint8_t value = eeprom_read_heartbeat_output_style_setting ();
	if (value == COMMAND_OPTION_TEXT_ALL_HEARTBEAT)
	{
		command_received_global.ph_heartbeat_style_flag = true;
	}
	else
	{
		command_received_global.ph_heartbeat_style_flag = false;
	}
}

void set_anbsensors_or_alternate_head(void)
{
	int head = received_data_array[2];

	eeprom_write_anbsensors_or_alternate_head(head);

	if(head == COMMAND_OPTION_ANBSENSORS_HEAD)
		number_of_ph_electrodes = NUMBER_OF_PH_ELECTRODES_ANB;
	if(head == COMMAND_OPTION_VALEPORT_HEAD)
		number_of_ph_electrodes = NUMBER_OF_PH_ELECTRODES_VALEPORT;
}

void set_interface(void)
{
	eeprom_write_interface(received_data_array[2]);
}

void reply_anbsensors_or_alternate_head(void)
{
	transmit_data_array[1] = COMMAND_TEXT_ANB_OR_ALTERNATE_HEAD;
	transmit_data_array[2] = eeprom_read_anbsensors_or_alternate_head();
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_interface(void)
{
	transmit_data_array[1] = COMMAND_TEXT_INTERFACE;
	transmit_data_array[2] = eeprom_read_interface();
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_get_temperature(void)
{
	float_to_byte_array(&transmit_data_array[2], qaqc_reply.calculated_temperature);
	transmit_data_array[1] = COMMAND_TEXT_TEMPERATURE;
	command_received_global.reply_size = 2 + COMMAND_TEMPERATURE_LENGTH;

	send_reply();
}

float system_get_temperature(void)
{
	uint8_t anb_or_alt = eeprom_read_byte(EEPROM_ADDR + DEVICE_SCAN_ANB_OR_ALTERNATE_HEAD_ADDRESS);

	// get ADC reading
	int32_t adc_value = get_temperature(anb_or_alt);

	if (anb_or_alt == COMMAND_OPTION_VALEPORT_HEAD)
		qaqc_reply.calculated_temperature = VP_calculate_temperature(adc_value);
	else
		qaqc_reply.calculated_temperature = calculate_temperature(adc_value);

	return(qaqc_reply.calculated_temperature);
}

void reply_immersed_dry_status(void)
{
	transmit_data_array[1] = COMMAND_TEXT_IMMERSED_DRY_STATUS;
	transmit_data_array[2] = check_immersed_dry_status();
	command_received_global.reply_size = 3;

	// transmit data has been assembled, send it
	send_reply();
	immersion_status_requested_flag = true;
}

void reply_scan_n_no_of_electrodes(void)
{
	transmit_data_array[1] = COMMAND_TEXT_SCAN_N_NO_OF_PH_ELECTRODES;
	transmit_data_array[2] = eeprom_read_scan_n_no_of_ph_electrodes();
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_need_more_time(void)
{
	transmit_data_array[1] = COMMAND_TEXT_NEED_MORE_TIME;
	transmit_data_array[2] = 1;
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
	// set the qaqc data waiting to be read flag
	command_received_global.qaqc_data_waiting_to_read_flag = true;
}

void reply_firmware_version(void)
{
	uint8_t version_changed_value = 0;
	uint8_t last_version[DEVICE_DRIVER_FIRMWARE_VERSION_LENGTH];
	bool firmware_version_changed = false;

	eeprom_read_driver_version(last_version);
	for (int i = 0; i < strlen(FIRMWARE_VERSION); i++)
		if(last_version[i] != *(FIRMWARE_VERSION + i))
			firmware_version_changed = true;

	if(firmware_version_changed)
	{
		eeprom_write_driver_version((uint8_t*) FIRMWARE_VERSION);
		version_changed_value = 0xff;
	}

	transmit_data_array[1] = COMMAND_TEXT_FIRMWARE_VERSION;
	transmit_data_array[2] = version_changed_value;
	transmit_data_array[3] = eeprom_read_byte_with_checks(DEVICE_INTERFACE_VERSION_ADDRESS, 0, 255, 255);
	transmit_data_array[4] = eeprom_read_byte_with_checks(DEVICE_INTERFACE_VERSION_ADDRESS + 1, 0, 255, 255);
	command_received_global.reply_size = 5;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_fast_profiling(void)
{
	transmit_data_array[1] = COMMAND_TEXT_FAST_PROFILING;
	transmit_data_array[2] = eeprom_read_fast_profiling();
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_use_sdcard(void)
{
	transmit_data_array[1] = COMMAND_TEXT_USE_SDCARD;
	transmit_data_array[2] = eeprom_read_use_sdcard();
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_power(void)
{
	transmit_data_array[1] = COMMAND_TEXT_POWER;
	transmit_data_array[2] = eeprom_read_power();
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_salinity_range(void)
{
	transmit_data_array[1] = COMMAND_TEXT_SALINITY_RANGE;
	transmit_data_array[2] = eeprom_read_salinity_range();
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}

void reply_phs_per_loop(void)
{
	transmit_data_array[1] = COMMAND_TEXT_PHS_PER_LOOP;
	transmit_data_array[2] = eeprom_read_phs_per_loop();
	command_received_global.reply_size = 3;
	// transmit data has been assembled, send it
	send_reply();
}
