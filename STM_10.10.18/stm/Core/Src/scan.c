#include <scan.h>

double ph_values[PH_ARRAY_SIZE_MAX];

uint16_t adc_mark_data_global[ADC_DATA_ARRAY_SIZE];
uint16_t adc_space_data_global[ADC_DATA_ARRAY_SIZE];
float adc_mark_space_array_1_global[ADC_DATA_ARRAY_SIZE];
float adc_mark_space_array_2_global[ADC_DATA_ARRAY_SIZE];
float dac_output_voltage_data_global[ADC_DATA_ARRAY_SIZE];

volatile bool anb_sensors_interface_board_connected = true;
bool store_last_electrode_number = true;
uint8_t ph_start_electrode_number = 1;
uint8_t next_ph_electrode_number = 1;
uint8_t ph_electrodes_completed = 0;
uint8_t loop_interval = 0;
bool event_passed = false; //true; //default to normal behaviour
double previous_iref_peak_potential = 0, previous_ph_peak_potential = 0, previous_ph = 0;
int loops_since_start = 0;
double last_good_peak_potential = 0;

task current_task;

void scan(int scan_type)
{
	bool finished_last_scan_of_last_task = false;

	int adc_data_size_for_current_scan = 0;
	bool send_qaqc_data = false;
	double salinity_temperature;
	int successive_ph_scan_failures = 0;
	int ph_scans_left = 0;
	int gain_change = 0;
	int current_gain, new_gain;
	bool square_waves_on = false;
	int loops_since_last_correction;
	int loops_since_last_event;
	int candidate_next_set_electrode = 0;
	int candidate_next_set_electrode_gain = 0;
	bool scan_n = false;
	int good_ph_scans_completed = 0;
	int scan_n_electrodes = 0;

	if(scan_type == SCAN_TYPE_OCEAN)
	{
		ocean_scan = true;
		fresh_scan = false;
		vf_scan = false;
	}
	else if(scan_type == SCAN_TYPE_FRESH)
	{
		ocean_scan = false;
		fresh_scan = true;
		vf_scan = false;
	}
	else if(scan_type == SCAN_TYPE_VF)
	{
		ocean_scan = false;
		fresh_scan = false;
		vf_scan = true;
	}

	//reassess from last loop
	previous_iref_peak_potential = eeprom_read_last_iref_peak_potential();
	if(dynamic_salinity)
	{
		if((previous_iref_peak_potential > 0) && (previous_iref_peak_potential < IREF_CROSSOVER_POTENTIAL))
		{
			ocean_scan = true;
			fresh_scan = false;
			vf_scan  = false;
		}
		else if((previous_iref_peak_potential > 0) && (previous_iref_peak_potential >= IREF_CROSSOVER_POTENTIAL))
		{
			ocean_scan = false;
			fresh_scan = true;
			vf_scan  = false;
		}
	}

	if(ocean_scan)
		last_good_peak_potential = IREF_OCEAN_SCAN_PEAK_POTENTIAL_DEFAULT;
	else
		last_good_peak_potential = IREF_FRESH_SCAN_PEAK_POTENTIAL_DEFAULT;

	if(eeprom_read_fast_profiling() == 2)
		fast_profiling = false;
	if(eeprom_read_fast_profiling() == 1)
		fast_profiling = true;

	eeprom_read_clipped_at_gain();
	eeprom_read_array_health();

	loops_since_last_correction = eeprom_read_loops_since_last_correction();
	loop_interval = eeprom_read_sensor_array_interval_setting();
	int loop_count = eeprom_read_loop_count();
	if((loop_count % 50 == 0))
		reset_clipped_at_gain();

	avg_ph_peak_potential_stored = (bool)eeprom_read_byte_with_checks(DEVICE_AVG_PH_PEAK_POTENTIAL_STORED, 0, 1, 0);
	preconditioning_done = (bool)eeprom_read_byte_with_checks(DEVICE_PRECONDITIONING_DONE, 0, 1, 0);

	bool powered = (eeprom_read_power() == 1);

	if(!powered) //can't trust first scan flag, it will always say first. go by preconditioning_done instead
		scan_first_flag = false;

	loops_since_last_event = eeprom_read_loops_since_last_event();
	if(loops_since_last_event > 2) //put this number in a variable if we use it
		event_passed = true;

	//establish whether we have intervals
	if((scan_internal_interval_flag) && (loop_interval >= 60)) //continuous, with interval >=60 mins
	{
		scan_long_interval_flag = true;
		scan_interval_flag = true;
	}
	else if((scan_internal_interval_flag) && (loop_interval >= 10)) //continuous, with interval >=10 mins
	{
		scan_short_interval_flag = true;
		scan_interval_flag = true;
	}

	if(scan_short_interval_flag || scan_long_interval_flag)
		scan_interval_flag = true;

	if(scan_interval_flag && ocean_scan)
		fast_profiling = true;

	if(scan_first_flag && scan_long_interval_flag) //we are counting loops for long intervals
	{
		eeprom_write_loops_since_last_event(0);
		event_passed = false;
		loops_since_last_event = 0;
	}

	if(scan_long_interval_flag) //long interval, either controlled or external, always do precon and clear data
	{
		precon_diagnostic = 10000;

		if(event_passed)
			perform_precondition_ph_electrodes_flag = true;

		precondition_ph_case = 1;
		//avg_ph_peak_potential_stored = false; //don't want old data
		//eeprom_save_avg_ph_peak_potential();
		clear_ph_array();

		if(!preconditioning_done) //so this applies on first loop
		{
			remember_preconditioning_done = true;
			eeprom_write_loops_since_start(0);
		}
	}
	else if(scan_external_interval_flag && scan_short_interval_flag) //external short interval, so don't know where we are in the sequence, so check if we've preconditioned already
	{
		precon_diagnostic = 20000;
		if(!preconditioning_done)
		{
			perform_precondition_ph_electrodes_flag = true;
			precondition_ph_case = 2;
			avg_ph_peak_potential_stored = false; //don't want old data
			eeprom_save_avg_ph_peak_potential();
			clear_ph_array();
			remember_preconditioning_done = true;
			eeprom_write_loops_since_start(0);
		}
	}
	else if(scan_internal_interval_flag && scan_first_flag)
	{
		if(!preconditioning_done)
		{
			precon_diagnostic = 30000;
			perform_precondition_ph_electrodes_flag = true;
			precondition_ph_case = 1;
			remember_preconditioning_done = true;
		}
		avg_ph_peak_potential_stored = false; //don't want old data
		eeprom_save_avg_ph_peak_potential();
		clear_ph_array();
		eeprom_write_loops_since_start(0);
	}
	else if(scan_internal_interval_flag && !scan_first_flag)
	{
		if(!preconditioning_done)
		{
			precon_diagnostic = 40000;
			perform_precondition_ph_electrodes_flag = true;
			precondition_ph_case = 1;
			avg_ph_peak_potential_stored = false; //don't want old data
			eeprom_save_avg_ph_peak_potential();
			clear_ph_array();
			remember_preconditioning_done = true;
			eeprom_write_loops_since_start(0);
		}
	}

	loops_since_start = eeprom_read_loops_since_start();

	if(preconditioning_done)
		precon_diagnostic += 1000;
	if(avg_ph_peak_potential_stored)
		precon_diagnostic += 10;
	if(perform_precondition_ph_electrodes_flag)
		precon_diagnostic += 100;

	//work out how many pH electrodes are needed
	if(scan_interval_flag)
	{
		scan_n_electrodes = eeprom_read_scan_n_no_of_ph_electrodes();
		scan_n_electrodes = 8; //number_of_ph_electrodes; //8; //sg! how do we want to leave this?

		if(scan_n_electrodes != number_of_ph_electrodes)
			scan_n = true;

		if(fresh_scan)
			ph_scans_left = number_of_ph_electrodes / 2 + 2;
		else if(vf_scan)
			ph_scans_left = number_of_ph_electrodes / 3 + 3;
		else if(ocean_scan)
			ph_scans_left = number_of_ph_electrodes; //do all electrodes once

		loop_complete = false;
	}
	else
		ph_scans_left = 3*eeprom_read_phs_per_loop();

	for(int i = 0; i < number_of_ph_electrodes; i++)
		precondition_select[i] = true;
		
	scan_init();
	current_task = get_first_iref_scan(scan_type);

	if(eeprom_read_square_wave_data_save_status() == COMMAND_OPTION_TEXT_SQUAREWAVE_DATA_ON)
		square_waves_on = true;

	eeprom_read_ph_array();
	eeprom_read_peak_potential_offsets();
	eeprom_read_peak_potentials();
	eeprom_read_qaqcs();
	eeprom_read_ph_peaks_loops();
	eeprom_read_iref_peaks_loops();

	if(reset_electrode_gains_flag)
		set_default_gains();

	for(int i = 0; i < number_of_ph_electrodes; i++)
		if(electrode_health_values[i] >= BAD_ELECTRODE_HEALTH_THRESHOLD)
			bad_electrode[i] = true; //for use when pairing up in fresh_2_mode

	enable_analog_power_all();

	condition_ref();
	HAL_Delay(2000); //to be seen what this needs to be. don't want to scan immediately after condition ref?

	//main loop until all scans are done
	while(!finished_last_scan_of_last_task)
	{
		send_qaqc_data = false;
		reset_qaqc_reply_data();

		if(1) //this keeps code in line with ATX version for easier comparison
		{
			//do immersion before first iref and before scan 1 of every pH
			if(iref_scan & (current_task.scan_number_current == 1))
			{
				if(check_immersed_dry_status() == 2)  //dry. tell the interface and exit function if rule enabled
				{
					bool use_immersion_rule = (eeprom_read_immersion_rule() == 1);
					if(use_immersion_rule)
					{
						bool still_dry = true;

						for(int extra_tries = 0; extra_tries < 2; extra_tries++)
						{
							if(check_immersed_dry_status() == 1) //now immersed
							{
								still_dry = false;
								break;
							}
						}
						if(still_dry)
						{
							qaqc_reply.electrode_number = current_task.sensor_number;
							qaqc_reply.array_health = 4;
							qaqc_reply.first_peak_potential_reply = immersion_adc_difference;
							qaqc_reply.useful_customer_data = QAQC_REPLY_USEFUL_CUSTOMER_DATA;
							get_qaqc_reply_data_address(&qaqc_reply);
							reply_qaqc_data_ready_to_be_read();
							for (int i = 0; (i < 50) & get_qaqc_data_waiting_to_read_flag_status(); i++)
								HAL_Delay(10);

							clear_ph_array();
							return;
						}
					}
				}
			}
		}

		// calculate the array sizes based on span and step size, and set them all to zero
		adc_data_size_for_current_scan = (int) (current_task.span_mv / current_task.step_size);
		memset(adc_mark_data_global, 0, ADC_DATA_ARRAY_SIZE * sizeof(uint16_t));
		memset(adc_space_data_global, 0, ADC_DATA_ARRAY_SIZE * sizeof(uint16_t));
		memset(adc_mark_space_array_1_global, 0, ADC_DATA_ARRAY_SIZE * sizeof(float));
		memset(adc_mark_space_array_2_global, 0, ADC_DATA_ARRAY_SIZE * sizeof(float));
		memset(dac_output_voltage_data_global, 0, ADC_DATA_ARRAY_SIZE * sizeof(float));
		set_adc_data_address(adc_mark_data_global, adc_space_data_global,
		adc_mark_space_array_1_global, dac_output_voltage_data_global, adc_data_size_for_current_scan);
			
		//do we need to precondition
		if(immersed_flag & perform_precondition_ph_electrodes_flag & (current_task.multielectrode_scan) & (current_task.first_scan))
		{
			precondition_ph_electrodes(last_good_peak_potential);
			perform_precondition_ph_electrodes_flag = false;
			qaqc_reply.data_status = 1; //to indicate to the interface that preconditioning has been done
		}

		failure_diagnostic = 0;

		measurement_dac_settings();

		timer_calculate_counts(current_task.frequency);

		conditioning_timer_start();	// it will start the measurement once conditioning is finished

		while(scanning_in_progress())
		{
			// Do nothing until the scan finishes
		}

		if(!continue_measurement_flag) //stop command sent, exit loop
		{
			reply_stop_scanning();
			return;
		}

		parameters_diagnostic = 0;

		//some qaqc replies
		qaqc_reply.gain = current_task.gain_resistor_setting;
		if(!ocean_scan && (current_task.second_sensor_number != 0))
			qaqc_reply.electrode_number = current_task.sensor_number *16 + current_task.second_sensor_number;
		else
			qaqc_reply.electrode_number = current_task.sensor_number;
		qaqc_reply.salinity = SALINITY_DEFAULT_OUT_OF_RANGE_VALUE;
		qaqc_reply.specific_conductivity = VALUE_UNKNOWN;
		qaqc_reply.actual_conductivity = VALUE_UNKNOWN;
		qaqc_reply.start_potential = current_task.start_mv;
		qaqc_reply.end_potential = abs(current_task.start_mv - current_task.span_mv);

		//gain change checks
		if(immersed_flag)
		{
			uint16_t max_value = 0;
			current_gain = current_task.gain_resistor_setting;
			bool act_on_clip = current_task.regular_electrode || iref_scan;

			if(ph_scan)
			{
				if(ocean_scan && (current_task.pulse_amplitude != PH_OCEAN_PULSE_AMPLITUDE_MV_DEFAULT)) //might have overshot
					act_on_clip = false;
				else if(!ocean_scan && (current_task.frequency != PH_FRESH_FREQUENCY_DEFAULT)) //might have overshot
					act_on_clip = false;
			}

			new_gain = check_signal_size(adc_data_size_for_current_scan, current_gain, current_task.sensor_number, act_on_clip, &max_value);

			qaqc_reply.max_value = max_value;

			if(!current_task.multielectrode_scan)
				task_set_electrode_gain(new_gain);
			else
				multielectrode_gain = new_gain;
		}

		smoothing_function(adc_data_size_for_current_scan, &qaqc_reply);

		//get temperature
		if(ph_scan && ((current_task.scan_number_current > 1) || current_task.first_ph_scan_after_iref || current_task.last_scan))
		{
			qaqc_reply.read_temperature = get_temperature(anb_or_alternate_head);

			if (anb_or_alternate_head == COMMAND_OPTION_VALEPORT_HEAD)
				qaqc_reply.calculated_temperature = VP_calculate_temperature(qaqc_reply.read_temperature);
			else
				qaqc_reply.calculated_temperature = calculate_temperature(qaqc_reply.read_temperature);

			last_temperature = qaqc_reply.calculated_temperature;

			if(current_task.first_ph_scan_after_iref)
				salinity_temperature = last_temperature;
		}

		//some actions relating to iref health
		if(iref_scan)
		{
			if(iref_flags_ok)
			{
				last_good_peak_potential = qaqc_reply.peak_potential;

				iref_failures--;

				if(iref_failures < 0)
					iref_failures = 0;

			}
			else
				iref_failures++;

			qaqc_reply.electrode_health = iref_failures;
				
			if(current_task.first_scan)
				peak_potential_displayed_2 = immersion_adc_difference; //show immersion on first scan
		}
		else if(ph_scan && !current_task.multielectrode_scan) //some actions relating to pH health
		{
			if(ph_flags_ok)
			{
				last_good_peak_potential = qaqc_reply.peak_potential;
				successive_ph_scan_failures = 0;
				if(perfect_pass)
				{
					candidate_next_set_electrode = current_task.sensor_number;
					candidate_next_set_electrode_gain = current_task.gain_resistor_setting;
				}
			}

			if(no_peaks_found)
				successive_ph_scan_failures++;
		}

		if(!scan_interval_flag && current_task.regular_electrode && current_task.last_scan)
			qaqc_reply.useful_customer_data = QAQC_REPLY_USEFUL_CUSTOMER_DATA;
		else
			qaqc_reply.useful_customer_data = QAQC_REPLY_NOT_USEFUL_CUSTOMER_DATA;

		//get some measurements
		if(ph_scan && !current_task.multielectrode_scan)
		{
			qaqc_reply.salinity = calculate_salinity(avg_iref_peak_potential, salinity_temperature, scan_type);
			qaqc_reply.specific_conductivity = calculate_specific_conductivity(qaqc_reply.salinity);
			qaqc_reply.actual_conductivity = calculate_actual_conductivity(qaqc_reply.specific_conductivity, qaqc_reply.calculated_temperature, scan_type);
			qaqc_reply.calculated_ph = calculate_ph(qaqc_reply.peak_potential, qaqc_reply.calculated_temperature, avg_iref_peak_potential);

			if((current_task.include_in_ph_array) || (current_task.set_electrode && ph_flags_ok)) //sg! get agreement. set electrodes are good, should use them.
				qaqc_reply.running_average_ph = calculate_ph_running_average_scan(current_task.sensor_number, current_task.set_electrode, qaqc_reply.calculated_ph);

			qaqc_reply.electrode_health = get_electrode_health_value(current_task.sensor_number, current_task.second_sensor_number, qaqc_reply.calculated_ph, gain_change);
			ph_scans_left--;
			ph_electrodes_completed++;

			if(ph_flags_ok && !current_task.set_electrode)
				good_ph_scans_completed++;
		}

		if(current_task.multielectrode_scan)
			HAL_Delay(5000);

		//act on the peak potentials, and prepare for next scan
		process_peak_potentials_after_scan();

		if(iref_scan && (current_task.last_scan))
			last_good_peak_potential = avg_ph_peak_potential; //if we are not using the eeprom stored value then this be its default

		//now we have the information for get_transducer_health
		qaqc_reply.array_health = get_transducer_health(current_task.sensor_number);

		calculate_peak_potential_offsets(false); //just to calculate median peak potential for display

		//done all that, now send some data to the interface
		//have we finished scanning altogether?
		bool last_scan_of_loop = false;
		last_scan_of_loop = last_scan_of_loop || (scan_n && (good_ph_scans_completed == scan_n_electrodes));
		last_scan_of_loop = last_scan_of_loop || ((ph_scans_left <= 0) && current_task.last_scan);
		last_scan_of_loop = last_scan_of_loop || loop_complete;

		if (last_scan_of_loop)
		{
			finished_last_scan_of_last_task = true;
			electrode_array_looped = true; //so next time around the transducer health looks at all electrodes
			eeprom_save_ph_array();
			eeprom_save_last_temperature();
			eeprom_save_last_passed_ph_qaqc();

			if(first_good_ph_data_found || (good_peak_count > 0)) //this will indicate whether the stored value is from a successful measurement
			{
				//at this  point reassess iref
				if(suspect_iref)
				{
					if(

						(fabs((avg_ph_peak_potential + suspect_iref_peak_potential) - (previous_ph_peak_potential + previous_iref_peak_potential)) < 10) //the changes compensated

						||

						(fabs(avg_ph_peak_potential - previous_ph_peak_potential) > 10) //flavs have moved, iref didn't compensate, might as well trust iref

					)

					{
						//add in temperature correction to calculation below
						qaqc_reply.running_average_ph += 0.018*(avg_iref_peak_potential - suspect_iref_peak_potential);//make a desperate late lunge to get the pH back where it should be
						avg_iref_peak_potential = suspect_iref_peak_potential; //so we lost a loop with the wrong iref. those pH values will remain in the array.
						// check if new calculation is within 0 and 14
						if((qaqc_reply.running_average_ph < 0.0) || (qaqc_reply.running_average_ph > 14.0))
							qaqc_reply.running_average_ph = PH_DEFAULT_OUT_OF_RANGE_VALUE;
					}
				}
				avg_ph_peak_potential_stored = true;
			}
			else
				avg_ph_peak_potential_stored = false; //if the whole loop went by without a successful measurement the stored value will be out of date

			if(trust_iref)
				eeprom_save_avg_iref_peak_potential();

			//eeprom_save_avg_ph_peak_potential();

			if(scan_long_interval_flag) //we need a set electrode for next loop. do this before saving gains below.
			{
				eeprom_write_next_set_electrode(candidate_next_set_electrode);
				if(candidate_next_set_electrode > 0)
					electrode_gain_values[candidate_next_set_electrode - 1] = candidate_next_set_electrode_gain; //this is the gain at which it passed
			}

			eeprom_write_last_electrode_number(current_task.sensor_number);

			if(immersed_flag)
			{
				eeprom_save_electrode_health();
				eeprom_save_electrode_gains();
			}

			send_qaqc_data = true;

			if((qaqc_reply.array_health == 6) || !first_good_ph_data_found)
			{
				eeprom_write_loops_since_last_good_ph(eeprom_read_loops_since_last_good_ph() + 1);
				clear_ph_array(); //array is out of date
			}
			else
				eeprom_write_loops_since_last_good_ph(0);

			eeprom_save_avg_ph_peak_potential();

			eeprom_write_loops_since_last_correction(loops_since_last_correction + 1);
			eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_ABRADED, 0);

			if(!event_passed)
				eeprom_write_loops_since_last_event(loops_since_last_event + 1);

			if(loops_since_last_correction >= 10)
			{
				calculate_peak_potential_offsets(true);
				eeprom_write_loops_since_last_correction(0);
			}

			eeprom_save_peak_potentials();
			eeprom_save_qaqcs();
			add_ph_peak_to_loop_array(avg_ph_peak_potential);
			add_iref_peak_to_loop_array(avg_iref_peak_potential);

			loops_since_start++;
			if(loops_since_start > 254)
				loops_since_start = 254;
			eeprom_write_loops_since_start(loops_since_start);

			eeprom_write_loop_count(eeprom_read_loop_count() + 1);
			qaqc_reply.useful_customer_data = QAQC_REPLY_USEFUL_CUSTOMER_DATA;

			if(ph_scan && vf_scan)
				parameters_diagnostic = current_task.third_sensor_number; //on last reply this won't be updated because update_task_after_scan isn't called. do it here every time instead
		}
		else //if its not the last scan
		{
			update_task_after_scan(iref_flags_ok, ph_flags_ok, &successive_ph_scan_failures);
			qaqc_reply.last_qaqc = QAQC_REPLY_NOT_LAST_SCAN;
		}
			
		if(square_waves_on)
		{
			// send data through slave spi
			send_data_as_spi_slave(adc_mark_data_global, adc_space_data_global,
				adc_mark_space_array_1_global, adc_mark_space_array_2_global,
				dac_output_voltage_data_global,
				adc_data_size_for_current_scan, finished_last_scan_of_last_task);
		}

		if (anb_sensors_interface_board_connected)
			send_qaqc_data = true;

		// send qaqc data available message
		if (send_qaqc_data)
		{
			get_qaqc_reply_data_address(&qaqc_reply);
			// send qaqc reply ready to be transmitted if requested by the interface board. the reply is handled by the usart isr within the delay period
			reply_qaqc_data_ready_to_be_read();
			// wait up to 500 ms for the qaqc data to be read (in 10 ms steps). after that regardless of the qaqc read status continue to next scan (if its the last scan stop)
			for (int i = 0; (i < 50) & get_qaqc_data_waiting_to_read_flag_status(); i++)
				HAL_Delay(10);
		}
	}

	disable_analog_power_all();
}
