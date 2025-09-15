#include "tasktable_init.h"

uint8_t iref_health_bad_count = 0;
uint8_t iref_secondary_health_bad_count = 0;
volatile bool reset_electrode_gains_flag = false;

uint8_t electrode_gain_values[GAIN_ARRAY_SIZE] = {0};
uint8_t multielectrode_gain = 1;
bool electrode_gain_bumped[GAIN_ARRAY_SIZE] = {false};
bool gain_bump_failed[GAIN_ARRAY_SIZE] = {false};

double peak_potential_offsets[NUMBER_OF_PH_ELECTRODES_MAX] = {0};
double peak_potentials[NUMBER_OF_PH_ELECTRODES_MAX] = {0};
double qaqcs[NUMBER_OF_PH_ELECTRODES_MAX] = {0};
double ph_peaks_loops[PEAKS_LOOPS_ARRAY_SIZE] = {0};
double iref_peaks_loops[PEAKS_LOOPS_ARRAY_SIZE] = {0};
double peak_shift_start = PH_FRESH_PEAK_SHIFT_START;
uint8_t electrode_pulse_amplitudes[NUMBER_OF_PH_ELECTRODES_MAX] = {0};
uint8_t electrode_frequencies[NUMBER_OF_PH_ELECTRODES_MAX] = {0};
uint8_t electrode_clipped_at_gain[NUMBER_OF_PH_ELECTRODES_MAX] = {0};

bool bad_electrode[NUMBER_OF_PH_ELECTRODES_MAX] = {false};
int electrodes_left_in_this_loop;
bool scanned_in_this_loop[NUMBER_OF_PH_ELECTRODES_MAX] = {false};
bool healthwatch = false;

int fresh_concurrent_electrodes = 2; //1;  start with fresh_2_mode even in continuous
bool fresh_2_mode = false;
uint8_t anb_or_alternate_head = COMMAND_OPTION_ANBSENSORS_HEAD;
bool ph_4_energy_boost = false;

bool ph_scan = false, iref_scan = true;
bool ocean_scan = false, fresh_scan = true, vf_scan = false, dynamic_salinity = false;
int electrodes_scanned_in_this_loop = 0;
int post_multielectrode_electrode_number = 0;
bool fast_profiling = false;
bool loop_complete = false;

int ringer_electrode_1 = 11;
int ringer_electrode_2 = 5;
bool stu_debug = true;

task task_iref_ocean = {
	IREF_PRIMARY_SENSOR_NUMBER,
	IREF_SCAN_NUMBER_OCEAN,

	IREF_OCEAN_FREQUENCY_DEFAULT,
	IREF_OCEAN_PULSE_AMPLITUDE_MV_DEFAULT,
	IREF_OCEAN_STEPSIZE_MV_INITIAL,
	IREF_OCEAN_START_MV_DEFAULT,
	IREF_OCEAN_SPAN_MV_INITIAL,

	SCAN_NUMBER_CURRENT_DEFAULT,
	FIRST_PH_SCAN_AFTER_IREF_DEFAULT,
	FIRST_SCAN_DEFAULT,
	LAST_SCAN_DEFAULT,

	IREF_GAIN_RESISTOR_SETTING_OCEAN,

	MULTIELECTRODE_SCAN_DEFAULT,
	SET_ELECTRODE_DEFAULT,
	INCLUDE_IN_PH_ARRAY_DEFAULT,
};

task task_iref_fresh = {
	IREF_PRIMARY_SENSOR_NUMBER,
	IREF_SCAN_NUMBER_FRESH,

	IREF_FRESH_FREQUENCY_DEFAULT,
	IREF_FRESH_PULSE_AMPLITUDE_MV_DEFAULT,
	IREF_FRESH_STEPSIZE_MV_DEFAULT,
	IREF_FRESH_START_MV_DEFAULT,
	IREF_FRESH_SPAN_MV_INITIAL,

	SCAN_NUMBER_CURRENT_DEFAULT,
	FIRST_PH_SCAN_AFTER_IREF_DEFAULT,
	FIRST_SCAN_DEFAULT,
	LAST_SCAN_DEFAULT,

	IREF_GAIN_RESISTOR_SETTING_FRESH,

	MULTIELECTRODE_SCAN_DEFAULT,
	SET_ELECTRODE_DEFAULT,
	INCLUDE_IN_PH_ARRAY_DEFAULT,
};

int get_iref_sensor_number(void)
{
	iref_last_used = eeprom_read_iref_last_used();
	
	if(iref_last_used == IREF_PRIMARY_SENSOR_NUMBER)
	{
		if(!use_single_iref)
			iref_last_used = IREF_PRIMARY_SENSOR_NUMBER;
		else
		{
			eeprom_write_iref_last_used(IREF_SECONDARY_SENSOR_NUMBER);
			iref_last_used = IREF_SECONDARY_SENSOR_NUMBER;
		}
	}
	else if(iref_last_used == IREF_SECONDARY_SENSOR_NUMBER)
	{
		if(!use_single_iref)
			iref_last_used = IREF_SECONDARY_SENSOR_NUMBER;
		else
		{
			eeprom_write_iref_last_used(IREF_PRIMARY_SENSOR_NUMBER);
			iref_last_used = IREF_PRIMARY_SENSOR_NUMBER;
		}
	}

	if(use_single_iref)
		return iref_last_used;
	else
		return IREF_PRIMARY_SENSOR_NUMBER; //SG 160524 we open both irefs for all scans so just use one sensor number for simplicity
}

int task_get_electrode_gain(int sensor_number)
{
	if(sensor_number > GAIN_ARRAY_SIZE) //probably multielectrode scan
		return multielectrode_gain;
	else
		return electrode_gain_values[sensor_number - 1];
}

void task_set_electrode_gain(int new_gain)
{
	if(current_task.sensor_number > GAIN_ARRAY_SIZE)
		return;

	if(ph_scan && (new_gain >= MAX_GAIN_PH))
		new_gain = MAX_GAIN_PH;

	if(iref_scan && (new_gain >= MAX_GAIN_IREF))
		new_gain = MAX_GAIN_IREF;

	if (new_gain < 1)
		new_gain = 1;

	//turning the gain down from maximum - a sign that the array has been abraded, do an extra scan
	if(ph_scan & (current_task.gain_resistor_setting >= MAX_GAIN_PH) & current_task.first_ph_scan_after_iref & (new_gain < current_task.gain_resistor_setting))
		current_task.number_of_scans += 1;

	current_task.gain_resistor_setting = new_gain;
	electrode_gain_values[current_task.sensor_number - 1] = current_task.gain_resistor_setting;
}

task get_first_iref_scan(int scan_type)
{
	task task_iref;

	eeprom_read_electrode_gains();
	eeprom_read_electrode_health();
	// check if the sensor is running on anbsensors head with different electrode number and thermocouple
	anb_or_alternate_head = eeprom_read_anbsensors_or_alternate_head();

	if(ocean_scan)
		task_iref = task_iref_ocean;
	else
		task_iref = task_iref_fresh;

	if(iref_health_bad_count >= 1) //last loop failed, try single irefs
	{
		use_single_iref = true;
		if(iref_health_bad_count > IREF_BAD_COUNT_THRESHOLD) //this will correspond to a transducer health of 3, we're not getting anywhere splitting the irefs
			if((iref_health_bad_count % 2) == 0)
				use_single_iref = false;
	}

	task_iref.sensor_number = get_iref_sensor_number();
	task_iref.gain_resistor_setting = task_get_electrode_gain(task_iref.sensor_number); //don't reset the iref gain, just retrieve it
	task_iref.second_sensor_number = 0;
	task_iref.third_sensor_number = 0;

	iref_scan = true;
	ph_scan = false;

	if(avg_ph_peak_potential_stored && ocean_scan) //override the start and span if we've got good data to refer to
	{
		task_iref.span_mv = IREF_OCEAN_SPAN_MV_REDUCED;
		task_iref.start_mv = previous_iref_peak_potential + 0.5 * task_iref.span_mv;
		if(task_iref.start_mv > IREF_OCEAN_START_MV_DEFAULT)
			task_iref.start_mv = IREF_OCEAN_START_MV_DEFAULT;
	}

	return task_iref;
}

void get_first_ph_scan()
{
	bool eighty_two_test = false; //true; //false;

	//set some defaults before they are possibly overwritten>
	current_task.first_scan = true;
	current_task.last_scan = false;
	current_task.multielectrode_scan = false;
	current_task.set_electrode = false;
	current_task.regular_electrode = false;
	current_task.scan_number_current = 1;
	current_task.first_ph_scan_after_iref = true;
	current_task.second_sensor_number = 0;
	current_task.third_sensor_number = 0;

	memset(scanned_in_this_loop, 0, sizeof(scanned_in_this_loop));

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
		current_task.span_mv = PH_OCEAN_SPAN_MV_INITIAL;
		current_task.number_of_scans = PH_SCAN_NUMBER_DEFAULT;
	}
	else if(fresh_scan)
	{
		current_task.step_size = PH_FRESH_STEPSIZE_MV_DEFAULT;
		current_task.frequency = PH_FRESH_FREQUENCY_DEFAULT;
		current_task.pulse_amplitude = PH_FRESH_PULSE_AMPLITUDE_MV_DEFAULT;
		current_task.span_mv = PH_FRESH_SPAN_MV_INITIAL;
		current_task.number_of_scans = 2;
	}
	else if(vf_scan)
	{
		current_task.step_size = PH_FRESH_STEPSIZE_MV_DEFAULT;
		current_task.frequency = PH_FRESH_FREQUENCY_DEFAULT;
		current_task.pulse_amplitude = PH_FRESH_PULSE_AMPLITUDE_MV_DEFAULT;
		current_task.span_mv = PH_FRESH_SPAN_MV_INITIAL;
		current_task.number_of_scans = 2;
	}

	double clamped_iref = avg_iref_peak_potential;

	if(clamped_iref > IREF_CAP_HIGH)
		clamped_iref = IREF_CAP_HIGH;

	if(suspect_iref)
		clamped_iref = previous_iref_peak_potential; //rogue number might throw us off. however, if this causes failures then reset_start_and_span will use the actual measured value

	current_task.start_mv = 620 - (clamped_iref - 680) * 0.80; //tie this 0.8 to the pH calculation variable
	if(current_task.start_mv > 720)
		current_task.start_mv = 720;

	//<defaults set

	if(stu_debug) //check this works for both fresh and ocean
	{
		ph_peak_shift_start = 150;

		if((loops_since_start * loop_interval) > 360) //6 hours
			ph_peak_shift_start = 275;
		if((loop_interval == 0) && (loops_since_start > 36))
			ph_peak_shift_start = 275;

		if((loops_since_start * loop_interval) > 1440) //24 hours
			ph_peak_shift_start = 325;
	}
	//set start and span if we have previous data stored
	if(avg_ph_peak_potential_stored) //override the start and span if we've got good data to refer to
	{
		double iref_peak_diff = 0;

		if(suspect_iref)
			iref_peak_diff = 0;
		else
			iref_peak_diff = iref_loop_movement;

		precon_diagnostic += 1;

		if(ocean_scan)
		{
			current_task.span_mv = ph_peak_shift_start + PH_OCEAN_PEAK_SHIFT_END;
			current_task.start_mv = avg_ph_peak_potential + ph_peak_shift_start;
			current_task.start_mv -= 0.8*iref_peak_diff; //so if iref peak has gone up (fresher) we expect pH peak to come down, so reduce the start potential
		}
		else
		{
			current_task.span_mv = ph_peak_shift_start + PH_FRESH_PEAK_SHIFT_END;
			current_task.start_mv = avg_ph_peak_potential + ph_peak_shift_start;
			current_task.start_mv -= 0.8*iref_peak_diff; //so if iref peak has gone up (fresher) we expect pH peak to come down, so reduce the start potential
		}
	}

	//do we need a set ph electrode
	if(perform_precondition_ph_electrodes_flag)
	{
		if(ocean_scan)
		{
			int set_electrode_number = eeprom_read_next_set_electrode();

			if((set_electrode_number == 0) || scan_first_flag) //nothing stored or first loop, use best health
			{
				int lowest_health_yet = MAX_HEALTH;
				for(int i = 0; i < number_of_ph_electrodes; i++)
				{
					if(electrode_health_values[i] <= lowest_health_yet)
					{
						lowest_health_yet = electrode_health_values[i];
						set_electrode_number = i + 1;
					}
				}
				current_task.sensor_number = set_electrode_number;
			}
			else
				current_task.sensor_number = set_electrode_number;

			scanned_in_this_loop[set_electrode_number - 1] = true;
			electrodes_scanned_in_this_loop++;
		}
		else
			update_electrode_numbers();

		current_task.set_electrode = true;
		current_task.number_of_scans = PH_SCAN_NUMBER_DEFAULT;
		current_task.include_in_ph_array = false;

	}
	else
	{
		if(eighty_two_test && !scan_long_interval_flag)
		{
			current_task.sensor_number = 130;
			current_task.second_sensor_number = 0;
			current_task.third_sensor_number = 0;
			current_task.multielectrode_scan = true;
			current_task.step_size = MULTIELECTRODE_STEPSIZE_MV;
			current_task.pulse_amplitude = MULTIELECTRODE_PULSE_AMPLITUDE;
			if(scan_interval_flag && (loop_interval >= 15) && (loop_interval < 60))
				current_task.number_of_scans = 1;
			else
				current_task.number_of_scans = 1;

			post_multielectrode_electrode_number = eeprom_read_last_electrode_number() + 2;
			if(post_multielectrode_electrode_number > number_of_ph_electrodes)
				post_multielectrode_electrode_number -= number_of_ph_electrodes;
		}
		else
		{
			current_task.regular_electrode = true;
			current_task.include_in_ph_array = true;
			current_task.sensor_number = eeprom_read_last_electrode_number() + 1;
			if(current_task.sensor_number > number_of_ph_electrodes)
				current_task.sensor_number -= number_of_ph_electrodes;
			update_electrode_numbers();
		}
	}

	//now some other parameters
	if(rolling_ph_electrode_mode && current_task.regular_electrode)
	{
		current_task.number_of_scans = 1;
	}

	current_task.gain_resistor_setting = task_get_electrode_gain(current_task.sensor_number);
	if(current_task.scan_number_current == current_task.number_of_scans)
		current_task.last_scan = true;

	if(!current_task.multielectrode_scan && ph_4_energy_boost && (displayed_ph_running_average < 5))
		current_task.start_mv += 50;

	if(current_task.set_electrode)
		current_task.start_mv += 50;

	if(current_task.start_mv > 720)
		current_task.start_mv = 720;

	iref_scan = false;
	ph_scan = true;
}

void update_electrode_numbers(void)
{
	bool electrode_found = false, take_anything = true;
	int first_electrode_number = 1, second_electrode_number = 0, third_electrode_number = 0;
	int good_electrodes_available = number_of_ph_electrodes, total_electrodes_to_be_grouped = number_of_ph_electrodes, total_singles_to_be_scanned = 0;
	bool randomising_ocean = false, using_ringers = false, healthwatching = false;
	bool ringer_loop = ((eeprom_read_loop_count() % 10) == 0);

	if(scan_first_flag)
		ringer_loop = true;

	if(vf_scan)
	{
		healthwatching = true;
		using_ringers = true;
	}

	if(stu_debug)
	{
		healthwatching = true; //post abrade version for diagnostics
		using_ringers = true;
	}

	int first_healthwatch_electrode_number, second_healthwatch_electrode_number;
	int loop_count = eeprom_read_loop_count();

	if(using_ringers) //set these aside, don't look for them unless needed
	{
		scanned_in_this_loop[ringer_electrode_1 - 1] = true; //set these aside so they don't get scanned in groups
		scanned_in_this_loop[ringer_electrode_2 - 1] = true; //set these aside so they don't get scanned in groups

		total_electrodes_to_be_grouped -= 2;
		if(ringer_loop)
			total_singles_to_be_scanned = 2;
	}

	if(healthwatching)
	{
		total_singles_to_be_scanned = 2;
		//we're setting the healthwatches aside, so reduce the pool to choose from
		total_electrodes_to_be_grouped -= 2;

		if(using_ringers && ringer_loop)
		{
			first_healthwatch_electrode_number = ringer_electrode_1;
			second_healthwatch_electrode_number = ringer_electrode_2;
		}
		else
		{
			first_healthwatch_electrode_number = 1 + (loop_count % 11); //first of two electrodes to be scanned on their own

			if(using_ringers)
			{
				if((first_healthwatch_electrode_number == ringer_electrode_1) || (first_healthwatch_electrode_number == ringer_electrode_2))
					first_healthwatch_electrode_number++;
				if(first_healthwatch_electrode_number > number_of_ph_electrodes)
					first_healthwatch_electrode_number -= number_of_ph_electrodes;
			}

			second_healthwatch_electrode_number = first_healthwatch_electrode_number + 5; //so it doesn't appear in consecutive loops
			if(second_healthwatch_electrode_number > number_of_ph_electrodes)
				second_healthwatch_electrode_number -= number_of_ph_electrodes;

			if(using_ringers)
			{
				if((second_healthwatch_electrode_number == ringer_electrode_1) || (second_healthwatch_electrode_number == ringer_electrode_2))
					second_healthwatch_electrode_number++;
				if(second_healthwatch_electrode_number > number_of_ph_electrodes)
					second_healthwatch_electrode_number -= number_of_ph_electrodes;
			}
		}

		scanned_in_this_loop[first_healthwatch_electrode_number - 1] = true; //set these aside so they don't get scanned in groups
		scanned_in_this_loop[second_healthwatch_electrode_number - 1] = true; //set these aside so they don't get scanned in groups
	}

	for(int i = 0; i < number_of_ph_electrodes; i++)
	{
		if(scanned_in_this_loop[i] || bad_electrode[i])
			good_electrodes_available--; //recalculate every time we enter this function, it's cleaner
		else
			first_electrode_number = i + 1; //candidate single electrode
	}

	srand(adc_convert_manipulate());

	current_task.second_sensor_number = 0;
	current_task.third_sensor_number = 0;

	if(ocean_scan)
	{
		if(randomising_ocean)
		{
			while(!electrode_found)
			{
				first_electrode_number = 1 + rand() % number_of_ph_electrodes;
				if(!scanned_in_this_loop[first_electrode_number - 1] && (!bad_electrode[first_electrode_number - 1] || take_anything))
					electrode_found = true;
			}
		}
		else if(!current_task.multielectrode_scan)
		{
			first_electrode_number = current_task.sensor_number + 1;
			if(first_electrode_number > number_of_ph_electrodes)
				first_electrode_number -= number_of_ph_electrodes;
		}
		else
			first_electrode_number = post_multielectrode_electrode_number;

		current_task.sensor_number = first_electrode_number;
		scanned_in_this_loop[first_electrode_number - 1] = true;
		electrodes_scanned_in_this_loop++;
	}
	else //either fresh or vf
	{
		if(vf_scan)
			healthwatch = false; //refers to this particular scan. start off false as default.
		else
			healthwatch = true;

		if(good_electrodes_available > 0) //look for good electrodes first
			take_anything = false;

		//now onto forming groups of 1, 2, or 3
		//get first electrode
		if(total_electrodes_to_be_grouped > electrodes_scanned_in_this_loop)
		{
			electrode_found = false;
			while(!electrode_found)
			{
				first_electrode_number = 1 + rand() % number_of_ph_electrodes;
				if(!scanned_in_this_loop[first_electrode_number - 1] && (!bad_electrode[first_electrode_number - 1] || take_anything))
					electrode_found = true;
			}

			current_task.sensor_number = first_electrode_number;
			scanned_in_this_loop[first_electrode_number - 1] = true;
			electrodes_scanned_in_this_loop++;
			if(!take_anything)
				good_electrodes_available--;

			if(total_electrodes_to_be_grouped > electrodes_scanned_in_this_loop) //more needed, but will they be added to this group?
			{
				if((!take_anything && (good_electrodes_available > 0)) || take_anything) //looking for a good one, and good ones are available, or we're taking anything
				{
					electrode_found = false;
					while(!electrode_found)
					{
						second_electrode_number = 1 + rand() % number_of_ph_electrodes;
						if(!scanned_in_this_loop[second_electrode_number - 1] && (!bad_electrode[second_electrode_number - 1] || take_anything))
							electrode_found = true;
					}

					current_task.second_sensor_number = second_electrode_number; //found when counting above
					scanned_in_this_loop[second_electrode_number - 1] = true;
					electrodes_scanned_in_this_loop++;
					if(!take_anything)
						good_electrodes_available--;

					if(vf_scan)
					{
						if(total_electrodes_to_be_grouped > electrodes_scanned_in_this_loop) //more needed, bust will they be added to this group?
						{
							if((!take_anything && (good_electrodes_available > 0)) || take_anything) //looking for a good one, and good ones are available, or we're taking anything
							{
								electrode_found = false;
								while(!electrode_found)
								{
									third_electrode_number = 1 + rand() % number_of_ph_electrodes;
									if(!scanned_in_this_loop[third_electrode_number - 1] && (!bad_electrode[third_electrode_number - 1] || take_anything))
										electrode_found = true;
								}

								current_task.third_sensor_number = third_electrode_number; //found when counting above
								scanned_in_this_loop[third_electrode_number - 1] = true;
								electrodes_scanned_in_this_loop++;
							}
						}
					}
				}
			}
		}
		else if(electrodes_scanned_in_this_loop == total_electrodes_to_be_grouped)
		{
			current_task.sensor_number = first_healthwatch_electrode_number;
			electrodes_scanned_in_this_loop++;
			healthwatch = true;
		}
		else if(electrodes_scanned_in_this_loop == (total_electrodes_to_be_grouped + 1))
		{
			current_task.sensor_number = second_healthwatch_electrode_number;
			electrodes_scanned_in_this_loop++;
			healthwatch = true;
			if(scan_interval_flag)
				loop_complete = true;
		}

		if(!ocean_scan && (current_task.second_sensor_number == 0)) //single electrode
		{
			current_task.pulse_amplitude = 200;
			healthwatch = true;
		}
		else
			current_task.pulse_amplitude = PH_FRESH_PULSE_AMPLITUDE_MV_DEFAULT;
	}

	if(electrodes_scanned_in_this_loop == (total_electrodes_to_be_grouped + total_singles_to_be_scanned)) //we have completed the set
	{
		memset(scanned_in_this_loop, 0, sizeof(scanned_in_this_loop));
		electrodes_scanned_in_this_loop = 0;
	}
}

void reset_frequencies(void)
{
	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		electrode_frequencies[i] = PH_FRESH_FREQUENCY_DEFAULT;

	eeprom_save_frequencies();
}

void reset_pulse_amplitudes(void)
{
	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		electrode_pulse_amplitudes[i] = PH_OCEAN_PULSE_AMPLITUDE_MV_DEFAULT;

	eeprom_save_pulse_amplitudes();
}

void reset_qaqcs(void)
{
	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		qaqcs[i] = 0;

	eeprom_save_qaqcs();
}

void reset_peaks_loops(void)
{
	for(int i = 0; i < PEAKS_LOOPS_ARRAY_SIZE; i++)
	{
		ph_peaks_loops[i] = 0;
		iref_peaks_loops[i] = 0;
	}

	eeprom_save_ph_peaks_loops();
	eeprom_save_iref_peaks_loops();
}

void reset_peak_potentials(void)
{
	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		peak_potentials[i] = 0;

	eeprom_save_peak_potentials();
}

void reset_clipped_at_gain(void)
{
	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		electrode_clipped_at_gain[i] = 255;

	eeprom_save_clipped_at_gain();
}

void reset_fresh_electrodes(void)
{
	eeprom_write_fresh_electrodes(2);
	fresh_concurrent_electrodes = 2;
}

void add_ph_peak_to_loop_array(double peak)
{
	for(int i = 0; i < (PEAKS_LOOPS_ARRAY_SIZE - 1); i++)
		ph_peaks_loops[i] = ph_peaks_loops[i + 1];

	ph_peaks_loops[PEAKS_LOOPS_ARRAY_SIZE - 1] = peak;

	eeprom_save_ph_peaks_loops();
}

void add_iref_peak_to_loop_array(double peak)
{
	for(int i = 0; i < (PEAKS_LOOPS_ARRAY_SIZE - 1); i++)
		iref_peaks_loops[i] = iref_peaks_loops[i + 1];

	iref_peaks_loops[PEAKS_LOOPS_ARRAY_SIZE - 1] = peak;

	eeprom_save_iref_peaks_loops();
}
