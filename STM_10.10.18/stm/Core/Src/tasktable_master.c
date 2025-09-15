#include "tasktable_master.h"

double iref_scan_pp_values[IREF_SCAN_NUMBER_MAX] = {0};	// to keep track of peak potential values sent, needed for averaging the iREF scans for first pH sensor
double pH_scan_pp_values[PH_SCAN_NUMBER_DEFAULT + 1] = {0};		// to keep track of peak potential values sent, needed for averaging the pH scans for next pH sensor

bool first_good_ph_data_found = false;
bool first_good_iref_data_found = false;
bool reset_start_and_span = false;
bool call_up_ringer = false;

bool new_start_mV_set = false; //indicates whether, for that electrode, the start potential has been changed by the first good scan.
double avg_iref_peak_potential = IREF_OCEAN_SCAN_PEAK_POTENTIAL_DEFAULT, avg_ph_peak_potential = PH_PEAK_POTENTIAL_DEFAULT, peak_potential_median = PH_PEAK_POTENTIAL_DEFAULT;;
double suspect_iref_peak_potential = IREF_OCEAN_SCAN_PEAK_POTENTIAL_DEFAULT;
double iref_loop_movement = 0;
double ph_peak_shift_start = PH_FRESH_PEAK_SHIFT_START;
volatile bool avg_ph_peak_potential_stored = false, remember_preconditioning_done = false, pp_cleared = false;
volatile bool preconditioning_done = false;
volatile bool abraded = false;

int iref_last_used = IREF_PRIMARY_SENSOR_NUMBER;

bool rolling_ph_electrode_mode = true;
bool ph_clipped = false, iref_clipped = false, use_single_iref = false;
bool good_iref_gain_found = false;
int good_iref_gain = MAX_GAIN_IREF;
int post_clipped_gain = MAX_GAIN_PH;
int good_peak_count = 0;

int suspect_peak_count = 0, suspect_qaqc_count = 0;
bool new_peak_established = false;
double emergency_iref_peak_potential = IREF_CAP_HIGH;
bool emergency_iref_peak_found = false;
double max_fresh_iref_start = IREF_FRESH_START_MV_DEFAULT;

bool precondition_select[NUMBER_OF_PH_ELECTRODES_MAX] = {true};

void process_peak_potentials_after_scan(void)
{
	double pp_sum, pp_avg, std_dev, std_dev_sum;
	double bad_data = 0x7FFFFFFF; //hate this, come up with another way of flagging bad data SG05082024
	int count;

	if(iref_scan)
	{

		if(stu_debug)
		{
			iref_flags_ok = true;
			last_good_peak_potential = 825;
		}

		if(iref_flags_ok)
		{
			iref_scan_pp_values[current_task.scan_number_current - 1] = last_good_peak_potential;
			if(!current_task.first_scan)
				first_good_iref_data_found = true;
		}
		else //store a value to indicate this so not included in the final calculation. bit clunky.
		{
			iref_scan_pp_values[current_task.scan_number_current - 1] = bad_data;
		}
		
		//if it's the last iref scan, calculate the average peak potential. we look the last IREF_SCAN_AVERAGE_NUMBER scans and use the good ones
		if (current_task.last_scan)
		{
			int total_number_of_scans = current_task.number_of_scans;
			int average_start = total_number_of_scans - IREF_SCAN_AVERAGE_NUMBER;
			if(use_single_iref)
				average_start--;

			pp_sum = 0;
			count = 0;

			for (int i = average_start; i < total_number_of_scans; i++)
			{
				if(iref_scan_pp_values[i] < bad_data) //so only good data is used
				{
					count++;
					pp_sum += iref_scan_pp_values[i];
				}
			}

			if(count > 0) //we had at least one good value
			{
				no_good_iref_scans = false;
				pp_avg = pp_sum / count;
				std_dev_sum = 0;

				//find standard deviation
				for (int i = average_start; i < total_number_of_scans; i++)
					if(iref_scan_pp_values[i] < bad_data) //so only good data is used
						std_dev_sum += pow(iref_scan_pp_values[i] - pp_avg, 2);

				std_dev = sqrt(std_dev_sum / count);

				//now go back, reject any outliers, and calculate new average
				count = 0;
				pp_sum = 0;

				for (int i = average_start; i < total_number_of_scans; i++)
				{
					if(iref_scan_pp_values[i] < bad_data) //so only good data is used
					{
						double diff = fabs(iref_scan_pp_values[i] - pp_avg);
						if((diff <= (1.0*std_dev)) || (diff < 10)) //include this value
							if(iref_scan_pp_values[i] < bad_data) //so only good data is used
							{
								count++;
								pp_sum += iref_scan_pp_values[i];
							}
					}
				}
				pp_avg = pp_sum / count;
			}
			else //we didn't have any good data.
			{
				pp_avg = emergency_iref_peak_potential;

				if(emergency_iref_peak_found)
					no_good_iref_scans = false;
				else
				{
					no_good_iref_scans = true;
					if((previous_iref_peak_potential > 0))
					{
						pp_avg = previous_iref_peak_potential;
						using_previous_iref = true;
					}
				}
			}

			avg_iref_peak_potential = pp_avg;

			if(iref_failures < 3)
			{
				trust_iref = true;
				if(dynamic_salinity) //sg! we need vf in here too, and a decision on how to go about all this
				{
					//now is the time to reassess our scan parameters
					if(!ocean_scan && (avg_iref_peak_potential < IREF_CROSSOVER_POTENTIAL))
					{
						fresh_scan = false;
						vf_scan = false;
						ocean_scan = true;
					}
					else if(ocean_scan && (avg_iref_peak_potential >= IREF_CROSSOVER_POTENTIAL))
					{
						fresh_scan = true;
						ocean_scan = false;
						vf_scan = false;
					}
				}
			}
			else
			{
				if(!ocean_scan && (avg_iref_peak_potential < 750)) //we've had multiple failures, and the iref doesn't match the scan type. use previous value
					if(previous_iref_peak_potential >= IREF_CROSSOVER_POTENTIAL)
						avg_iref_peak_potential = previous_iref_peak_potential;

				if(dynamic_salinity && ocean_scan && (avg_iref_peak_potential >= IREF_CROSSOVER_POTENTIAL))
				{
					fresh_scan = true;
					ocean_scan = false;
					vf_scan = false;
				}

				if(!ocean_scan && !no_good_iref_scans)
					trust_iref = true;
			}

			if(avg_ph_peak_potential_stored)
			{
				iref_loop_movement = avg_iref_peak_potential - previous_iref_peak_potential;
				int data_start_index;
				if(loops_since_start < PEAKS_LOOPS_ARRAY_SIZE)
					data_start_index = PEAKS_LOOPS_ARRAY_SIZE - loops_since_start;
				else
					data_start_index = 0;
				double predicted_iref = quadratic_fit_end_value(iref_peaks_loops, data_start_index, PEAKS_LOOPS_ARRAY_SIZE, false);
				if(fabs(predicted_iref - avg_iref_peak_potential) > 10)
				{
					suspect_iref = true;
					suspect_iref_peak_potential = avg_iref_peak_potential;
					avg_iref_peak_potential = previous_iref_peak_potential; //use old one for now until we're sure.
					using_previous_iref = true;
				}
			}
		}
	}

	if(ph_scan)
	{
		if(!current_task.multielectrode_scan && ph_flags_ok && !scan_interval_flag) //this is to check if we've latched onto a rogue peak
		{
			double electrodes_previous_qaqc = qaqcs[current_task.sensor_number - 1];

			if(electrodes_previous_qaqc > 0)
			{
				if(((qaqc_reply.qaqc_2 < 0.25 * electrodes_previous_qaqc) && (electrodes_previous_qaqc >= 40)) || ((qaqc_reply.qaqc_2 < 5) && (electrodes_previous_qaqc >= 20)))
				{
					suspect_qaqc_count++;
					ph_flags_ok = false; //although already in the pH array..
					perfect_pass = false;
					failure_diagnostic += 200;
				}
				else
				{
					//double alpha = 0.3;
					//last_passed_ph_qaqc = alpha*qaqc_reply.qaqc_1+ (1.0-alpha)*last_passed_ph_qaqc; //in case of sudden changes, weight towards existing figure
					suspect_qaqc_count = 0;
				}
			}
		}

		if(suspect_qaqc_count >= 5) //we're looking in the wrong place, reset window
		{
			reset_start_and_span = true;
			suspect_qaqc_count = 0;
			last_passed_ph_qaqc = 0;
			reset_qaqcs();
		}
/*
		//we might have passed limits, but is the peak reliable? continuous data will be rejected as outlier anyway
		//so this only applies to an interval when the array has been cleared and we've found good data
		if(!current_task.multielectrode_scan && (ph_flags_ok || ph_failed_only_as_outlier) && (scan_interval_flag || !electrode_array_looped) && (avg_ph_peak_potential_stored || first_good_ph_data_found))
		{
			if(fabs(last_good_peak_potential - avg_ph_peak_potential) > 50) //uh oh, let's hold off using this yet
			{
				suspect_peak_count++;
				ph_flags_ok = false; //although already in the pH array..
				perfect_pass = false;

				failure_diagnostic += 400;
			}
//			else if(ph_failed_only_as_outlier && !new_peak_established) //we're in business sg! does this mean on a new loop the first outlier clears the array?
//			{
//				ph_flags_ok = true;
//				suspect_peak_count = 0;
//				clear_ph_array(); //now we can put this value back in as our new starting point, clearing any old data
//				qaqc_reply.calculated_ph = calculate_ph(qaqc_reply.peak_potential, qaqc_reply.calculated_temperature, avg_iref_peak_potential);
//				if(current_task.include_in_ph_array)
//					qaqc_reply.running_average_ph = calculate_ph_running_average_scan(current_task.sensor_number, current_task.set_electrode, qaqc_reply.calculated_ph);
//			}

			//sg! suspect peaks need to be in the same direction. put this in. in meantime extend number to stop it messing
			if(suspect_peak_count > 5) //we've got to go with something, and let's assume the new peaks are due to a change in pH or salinity
			{
				ph_flags_ok = true;
				suspect_peak_count = 0;
				new_peak_established = true;
			}
		}
*/
		//store the peak potential
		if(!current_task.multielectrode_scan)
		{
			if(ph_flags_ok)
			{
				pH_scan_pp_values[current_task.scan_number_current - 1] = last_good_peak_potential;
				peak_potentials[current_task.sensor_number - 1] = last_good_peak_potential;
				qaqcs[current_task.sensor_number - 1] = qaqc_reply.qaqc_2;
			}
			else //data invalid
			{
				pH_scan_pp_values[current_task.scan_number_current - 1] = bad_data;
				qaqcs[current_task.sensor_number - 1] = 0;
			}
		}

		if (current_task.last_scan)
		{
			count = 0;
			pp_sum = 0;

			//need to work out what happens in the fresh set electrode case
			if(!(current_task.set_electrode && !ocean_scan)) //unless we are a freshwater set electrode which only has 2 scans
			{
				int total_number_of_scans = current_task.number_of_scans;
				int average_start = total_number_of_scans - 2; //take the last two scans
				if(rolling_ph_electrode_mode)
					average_start = 0; //we just want to look at the first and only element of the array sg! if set electrode then scans == 3? in which case use most recent if it passes
				for (int i = average_start; i < total_number_of_scans; i++)
				{
					if(pH_scan_pp_values[i] < bad_data) //so only good data is used
					{
						count++;
						pp_sum += pH_scan_pp_values[i];
					}
				}
			}

			if(count > 0)
				pp_avg = pp_sum / count;
			else
				pp_avg = last_good_peak_potential; //all scans failed. use the good value provided

			if(ph_flags_ok && !current_task.multielectrode_scan)
			{
				avg_ph_peak_potential = pp_avg;
				new_peak_established = true;
			}
		}
	}
}

void update_task_after_scan(bool iref_ok, bool ph_ok, int *successive_ph_scan_failures)
{
	int current_scan = current_task.scan_number_current;
	bool new_start_calculated = false;

	//iref changes
	if (iref_scan)
	{
		parameters_diagnostic = 10*current_task.frequency + current_task.step_size;

		//change the stepsize and frequency after scan 3
		if(current_scan == 3)
		{
			/*
			if(ocean_scan)
			{
				current_task.step_size = IREF_OCEAN_STEPSIZE_MV_RUN;

				if(iref_QAQC_parameter_change_flag)
				{
					current_task.frequency = 75;
					current_task.step_size = 2;
				}
			}
			*/
		}
		if(current_scan == 3)
		{
			if(fresh_scan)
			{
				current_task.step_size = 2;
				current_task.frequency = IREF_FRESH_FREQUENCY_RUN;
			}
		}

		//now look to see if we are struggling to find peaks
		if(!ocean_scan)
		{
			if(current_scan == 5)
				if(iref_failures == 5)
				{
					current_task.start_mv = 1300;
					max_fresh_iref_start = 1300;
				}

			if(current_scan == 4)
				if(iref_failures == 4)
				{
					//current_task.start_mv = 1350;
					//max_fresh_iref_start = 1350;
					//use_single_iref = true;
				}

			if(current_scan == 6)
				if(iref_failures == 6)
					current_task.pulse_amplitude = 300;

			if(current_scan <= 3)
				emergency_iref_peak_found = false; //ignore the first three scans
		}
		else
			if(current_scan == 3)
				if(iref_failures == 3)
				{
					current_task.start_mv = IREF_OCEAN_START_MV_DEFAULT;
					current_task.span_mv = IREF_OCEAN_SPAN_MV_INITIAL;
				}

		//change the span after scan 2 to a lower value if we have found a good peak
		if((current_task.scan_number_current >= 2) && first_good_iref_data_found)
		{
			if(ocean_scan)
				current_task.span_mv = IREF_OCEAN_SPAN_MV_REDUCED;
			else
				current_task.span_mv = IREF_FRESH_SPAN_MV_REDUCED;
		}
		
		bool allow_start_change = false;

		//change the start potential only at specific times
		if(ocean_scan)
			allow_start_change = ((current_scan == 2) || (current_scan == 3) || (current_scan == 4)); // || (current_scan == 5));
		else
			allow_start_change = ((current_scan == 2) || (current_scan == 3) || (current_scan == 4));

		//change the start potential based on previous peak potential. if the scan just performed was invalid this will use the last known good peak potential, as passed by the calling function
		if(first_good_iref_data_found && allow_start_change)
		{
			current_task.start_mv = last_good_peak_potential + 0.5 * current_task.span_mv;
			if(current_task.start_mv > max_fresh_iref_start)
				current_task.start_mv = max_fresh_iref_start;
		}

		if(current_task.last_scan)
		{
			update_iref_health_status();
			//update_iref_secondary_health_status();
			get_first_ph_scan();
			return;
		}
		else
			current_task.sensor_number = get_iref_sensor_number();
	}
	else if(ph_scan) 	//pH changes
	{	
		if(vf_scan)
			parameters_diagnostic = current_task.third_sensor_number;
		else
			parameters_diagnostic = current_task.frequency + current_task.step_size;

		current_task.first_ph_scan_after_iref = false; //done

		if(current_task.multielectrode_scan) //multielectrode scan, ensure no changes made
			ph_ok = false;
		
		bool electrode_health_acceptable = (electrode_health_values[current_task.sensor_number - 1] <= 3);

		if(current_task.set_electrode)
		{
			//if the set electrode scan has failed move on to the next to try that instead
			if(!ph_ok && !current_task.last_scan)
				update_electrode_numbers();
			else //got what we wanted, or we have to use a default if they all failed. move on to next set of scans (usually preconditioning)
			{
				current_task.last_scan = true;
				avg_ph_peak_potential = last_good_peak_potential;
				post_multielectrode_electrode_number = eeprom_read_last_electrode_number() + 1;
				if(post_multielectrode_electrode_number > number_of_ph_electrodes)
					post_multielectrode_electrode_number -= number_of_ph_electrodes;
				if(ph_ok)
				{
					force_start_change = true; //health might be over threshold but it's a good scan so use it
					first_good_ph_data_found = true;
				}
			}
		}

		//change the span to a lower value if we have found a good peak
		if((!first_good_ph_data_found) && ph_ok && electrode_health_acceptable)
		{
			if(ocean_scan)
				current_task.span_mv = ph_peak_shift_start + PH_OCEAN_PEAK_SHIFT_END;
			else
				current_task.span_mv = ph_peak_shift_start + PH_FRESH_PEAK_SHIFT_END;

			first_good_ph_data_found = true;
		}

		if(ph_ok && !electrode_health_acceptable)
			good_peak_count++;

		int good_peak_count_threshold = 5;
		if(scan_interval_flag)
			good_peak_count_threshold = 2;

		if(good_peak_count >= good_peak_count_threshold)
		{
			force_start_change = true;
			good_peak_count = 0;
			first_good_ph_data_found = true;
		}

		if(force_start_change || (ph_ok && electrode_health_acceptable))
		{
			if(ocean_scan)
				current_task.start_mv = last_good_peak_potential + ph_peak_shift_start;
			else
				current_task.start_mv = last_good_peak_potential + ph_peak_shift_start;

			new_start_calculated = true;

			force_start_change = false;

			good_peak_count = 0;

			avg_ph_peak_potential = qaqc_reply.peak_potential; //we are trusting the last potential
		}

		//if it's the last pH scan calculate the new start potential
		if (current_task.last_scan)
		{
			//if we're not getting anywhere, reset the start and span
			int successive_scan_failure_limit = 6;

			if(*successive_ph_scan_failures >= successive_scan_failure_limit)
				reset_start_and_span = true;

			if(reset_start_and_span)
			{
				double clamped_iref = avg_iref_peak_potential;
				if(clamped_iref > IREF_CAP_HIGH)
					clamped_iref = IREF_CAP_HIGH;
				current_task.start_mv = 620 - (clamped_iref - 680) * 0.80; //tie this 0.8 to the pH calculation variable
				if(current_task.start_mv > 720)
					current_task.start_mv = 720;

				if(ocean_scan)
					current_task.span_mv = PH_OCEAN_SPAN_MV_INITIAL;
				else
					current_task.span_mv = PH_FRESH_SPAN_MV_INITIAL;

				*successive_ph_scan_failures = 0;

				reset_start_and_span = false;

				if(!scan_interval_flag || !first_good_ph_data_found)
				{
					bool old_electrode_array_looped = electrode_array_looped; //clear_ph_array will clear this, but that would mess up transducer health
					clear_ph_array(); //otherwise for a pH change, will take a while for new data to outnumber old values
					first_good_ph_data_found = false;
					avg_ph_peak_potential_stored = false; //don't want this old data any more
					electrode_array_looped = old_electrode_array_looped;
					eeprom_save_avg_ph_peak_potential();
				}
			}

			//if we have just completed the set electrode and need to precondition, get the multielectrode number
			if(current_task.set_electrode && perform_precondition_ph_electrodes_flag)
			{
				current_task.sensor_number = 130;
				current_task.second_sensor_number = 0;
				current_task.third_sensor_number = 0;
				current_task.multielectrode_scan = true;
				current_task.set_electrode = false;
				current_task.step_size = MULTIELECTRODE_STEPSIZE_MV;
				current_task.pulse_amplitude = MULTIELECTRODE_PULSE_AMPLITUDE;
			}
			//if we have just done the multielectrode scans, move on to a regular electrode
			else if(current_task.multielectrode_scan)
			{
				update_electrode_numbers();
				current_task.multielectrode_scan = false;
				current_task.regular_electrode = true;

				if(ocean_scan)
				{
					double ocean_step_size = PH_OCEAN_STEPSIZE_MV_DEFAULT;
					if(fast_profiling)
						ocean_step_size = 2;
					else
						ocean_step_size = 1;
					current_task.step_size = ocean_step_size;
					current_task.frequency = PH_OCEAN_FREQUENCY_DEFAULT;
					current_task.pulse_amplitude = PH_OCEAN_PULSE_AMPLITUDE_MV_DEFAULT;
				}
				else if(fresh_scan)
				{
					current_task.step_size = PH_FRESH_STEPSIZE_MV_DEFAULT;
					current_task.frequency = PH_FRESH_FREQUENCY_DEFAULT;
					current_task.pulse_amplitude = PH_FRESH_PULSE_AMPLITUDE_MV_DEFAULT;
				}
				else if(vf_scan)
				{
					current_task.step_size = PH_FRESH_STEPSIZE_MV_DEFAULT;
					current_task.frequency = PH_FRESH_FREQUENCY_DEFAULT;
					current_task.pulse_amplitude = PH_FRESH_PULSE_AMPLITUDE_MV_DEFAULT;
				}
			}
			//else we want a regular electrode
			else
			{
				update_electrode_numbers();
				current_task.regular_electrode = true;
				current_task.set_electrode = false;
			}
		}

		//if this new task we are setting up is a regular electrode, decide if we want to use the pH - if it's scan 2 of 3 or the last scan, yes
		if(!current_task.set_electrode && !current_task.multielectrode_scan)
		{
			if(current_task.last_scan || (current_task.scan_number_current == 2))
				current_task.include_in_ph_array = true;
			else
				current_task.include_in_ph_array = false;
		}

		if(!current_task.multielectrode_scan && ph_4_energy_boost && (displayed_ph_running_average < 5) && ph_ok && electrode_health_acceptable)
		{
			current_task.start_mv += 50;
			//current_task.span_mv += 50;
		}

		if(scan_long_interval_flag && !event_passed && new_start_calculated)
			current_task.start_mv += 50;

		if(current_task.start_mv > 720)
			current_task.start_mv = 720;
	}

	if(ph_scan && current_task.last_scan)
	{
		//how many scans do we want.
		current_task.scan_number_current = 1;
		current_task.first_scan = true;

		uint8_t read_value = eeprom_read_byte_with_checks(DEVICE_ABRADED, 0, 1, 0);
		abraded = (bool)read_value;

		if(rolling_ph_electrode_mode && current_task.regular_electrode)
			current_task.number_of_scans = 1;
		else if(abraded && current_task.multielectrode_scan && !scan_interval_flag && (loop_interval == 0))
			current_task.number_of_scans = MULTIELECTRODE_SCAN_NUMBER_DEFAULT;
		else
			current_task.number_of_scans = PH_SCAN_NUMBER_DEFAULT;

		if(current_task.scan_number_current >= current_task.number_of_scans)
			current_task.last_scan = true;
		else
			current_task.last_scan = false;
	}
	else
	{
		current_task.first_scan = false;
		current_task.scan_number_current += 1;
		if(current_task.scan_number_current >= current_task.number_of_scans)
			current_task.last_scan = true;
	}

	current_task.gain_resistor_setting = task_get_electrode_gain(current_task.sensor_number);

	//set gain
	if(ph_scan && ph_clipped && (current_task.gain_resistor_setting == MAX_GAIN_PH))
	{
		current_task.gain_resistor_setting = post_clipped_gain;
		task_set_electrode_gain(current_task.gain_resistor_setting);
		failure_diagnostic += 10;
	}
}

void update_iref_health_status(void)
{
	bool iref_health_bad = true;

	if(no_good_iref_scans)
	{
		iref_health_bad_count++;
		if(iref_health_bad_count > 254)
			iref_health_bad_count = 253; //we so this to indicate alternating loops
	}
	else
	{
		/*iref_health_bad_count--; //do we want to count on the edge or reset in case our number gets too high (in which case cap in the lines above
		if(iref_health_bad_count < 0)
			iref_health_bad_count = 0; //to stop overflowing*/
		iref_health_bad_count = 0;
		iref_health_bad = false;
	}
	
	eeprom_write_iref_health_bad_count(iref_health_bad_count);
	eeprom_write_iref_health(iref_health_bad); //we write zero if good scan found
}

void update_iref_secondary_health_status(void)
{
	bool iref_health_bad = true;

	if(secondary_iref_failures >= IREF_FAILURE_THRESHOLD)
	{
		iref_secondary_health_bad_count++;
		if(iref_secondary_health_bad_count > 255)
			iref_secondary_health_bad_count = 255; //to stop overflowing
	}
	else
	{
		iref_secondary_health_bad_count = 0;
		iref_health_bad = false;
	}

	eeprom_write_iref_secondary_health_bad_count(iref_secondary_health_bad_count);
	eeprom_write_iref_secondary_health(iref_health_bad); //we write zero if good scan found
}

void calculate_peak_potential_offsets(bool store)
{
	double peak_potential_sum = 0; //, peak_potential_median;
	int count = 0;
	double copied_array[NUMBER_OF_PH_ELECTRODES_MAX];

	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
	{
		copied_array[i] = peak_potentials[i];
		peak_potential_sum += peak_potentials[i];
		if(peak_potentials[i] != 0)
			count++;
	}

	if(count > 0)
	{
		//here, qsort with cmpfunc_d sorts in descending order
		qsort(copied_array, NUMBER_OF_PH_ELECTRODES_MAX, sizeof(double), cmpfunc_d);
	    if (count % 2 != 0)
	    	peak_potential_median = copied_array[count / 2];
	    // If the number of elements is even, return the average of the two middle elements
	    else {
	        int mid_index = count / 2;
	        peak_potential_median = (copied_array[mid_index - 1] + copied_array[mid_index]) / 2.0;
	    }
	}
	else
		return;

	if(store)
	{
		for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		{
			if(peak_potentials[i] != 0)
			{
				peak_potential_offsets[i] += peak_potential_median - peak_potentials[i];
				if(abs(peak_potential_offsets[i]) > 20)
					peak_potential_offsets[i] = 0;
			}
		}

		eeprom_save_peak_potential_offsets();
	}
}

int cmpfunc_d(const void * a, const void * b)
{
  if(*(double*)a < *(double*)b)
    return 1;
  else if (*(double*)a > *(double*)b)
    return -1;
  else
    return 0;
}
