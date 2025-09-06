#include "calculate_ph.h"

volatile uint8_t electrode_health_values[NUMBER_OF_PH_ELECTRODES_MAX] = {0};
bool ph_array_index_looped = false;
int8_t ph_array_index = 0;
bool electrode_array_looped = false;
bool ph_failed_only_as_outlier = false;
double last_ph_std_dev = PH_DEFAULT_OUT_OF_RANGE_VALUE;
double last_temperature = VALUE_UNKNOWN;
double ph_alt_1 = PH_DEFAULT_OUT_OF_RANGE_VALUE, ph_alt_2 = PH_DEFAULT_OUT_OF_RANGE_VALUE;
double last_passed_ph_qaqc = 0;
double scan_health = 0;
int good_electrodes;
bool array_cleared = false;
int suspect_ph_count = 0;
bool ph_increasing = false, ph_decreasing = false;
double displayed_ph_running_average = PH_DEFAULT_OUT_OF_RANGE_VALUE;
double time_ordered_array[PH_ARRAY_SIZE_MAX] = {0};

double reading_avg(double *readings, int num_readings)
{
    double  sum;
    int     i;

    sum = 0.0;

    for (i = 0; i < num_readings; i++)
        sum += readings[i];

    return (sum / num_readings);
}

double calculate_ph(double vpeak_mV, double temp, double Iref_mV)
{
	bool calculated_ph_out_of_range = false;
    double  calculated_ph = PH_DEFAULT_OUT_OF_RANGE_VALUE;
    double iref_factor_min = 0.77, iref_factor_max = 0.8, iref_factor_o = 1.0, iref_transition_point = 860, k_steepness = 0.2;
    double flav_e_zero_o = 530, flav_e_zero_f = 500, iref_e_zero_f = 905, iref_e_zero_o = 805;
    double iref_factor = iref_factor_max, gradient = -0.018, iref_factor_nudge = 1000;
    double iref_cutoff_high_t, iref_cutoff_low_t, iref_cutoff_high_298 = 970, iref_cutoff_low_298 = 755, iref_cutoff_zero = 800;
    double iref_hard_cap_high = 920, iref_hard_cap_low_f = 720, iref_hard_cap_low_o = 705, used_iref;

    double temperature_correction = 298.0 / (temp + 273.0);
    double a3_o = 0.0481, a2_o = -1.033, a1_o = 8.1377, a0_o = -15.578;

	if(!ph_flags_ok)
		return PH_DEFAULT_OUT_OF_RANGE_VALUE;

    iref_cutoff_high_t = (iref_cutoff_high_298 + iref_cutoff_zero)/temperature_correction - iref_cutoff_zero;
    iref_cutoff_low_t = (iref_cutoff_low_298 + iref_cutoff_zero)/temperature_correction - iref_cutoff_zero;

    used_iref = median_of_three(iref_cutoff_low_t, Iref_mV, iref_cutoff_high_t); //clamp if outside temp adjusted limits
    used_iref = median_of_three(iref_hard_cap_low_f, used_iref, iref_hard_cap_high); //clamp if outside hard limits

    iref_factor = logistic(used_iref, iref_factor_min, iref_factor_max, k_steepness, iref_transition_point);

	calculated_ph = temperature_correction*(gradient*( (vpeak_mV - flav_e_zero_f) + iref_factor*(used_iref - iref_e_zero_f) + (iref_factor - iref_factor_min)*iref_factor_nudge));

	if(ocean_scan)
	{
		used_iref = median_of_three(iref_hard_cap_low_o, Iref_mV, iref_hard_cap_high);
		gradient = 1.0/55.0;
		calculated_ph = temperature_correction * gradient * ((flav_e_zero_o - vpeak_mV) + iref_factor_o*(iref_e_zero_o - used_iref));
		calculated_ph = a3_o*calculated_ph*calculated_ph*calculated_ph + a2_o*calculated_ph*calculated_ph + a1_o*calculated_ph + a0_o;
	}

	// check if calculated pH is within 0 and 14
	if (calculated_ph < 0.0)
		calculated_ph_out_of_range = true;
	if (calculated_ph > 14.0)
		calculated_ph_out_of_range = true;

	// put fault value
	if (calculated_ph_out_of_range)
		calculated_ph = PH_DEFAULT_OUT_OF_RANGE_VALUE;

    return calculated_ph;
}

double calculate_ph_running_average_scan(int sensor_number, bool set_electrode, double scan_ph)
{
	double calculated_running_avg_ph = PH_DEFAULT_OUT_OF_RANGE_VALUE;
	double alpha = 0.2;

	bool ignored_electrode = false;
	bool use_array_calculation = false;

//	if(electrode_array_looped)
//		ignored_electrode = ((sensor_number == 4) || (sensor_number == 6) || (sensor_number == 9) || (sensor_number == 11)); //because of reordering in subsystem.c

//	ignored_electrode = ignored_electrode || set_electrode;

//condition removed SG/NL 220724
//	ignored_electrode = ignored_electrode || (electrode_health_values[sensor_number - 1] > 3); //3 is the maximum health value where we accept the pH. this avoids accidental passes

	if(scan_ph == PH_DEFAULT_OUT_OF_RANGE_VALUE)
	{
		calculate_scan_health();
		return displayed_ph_running_average;
	}

	if ((scan_ph < 14) && (scan_ph > 0) && !ignored_electrode)
	{
		ph_values[ph_array_index] = scan_ph;
		ph_array_index = ph_array_index + 1;

		for(int i = 0; i < PH_ARRAY_SIZE_MAX; i++)
		{
			int index_to_copy = ph_array_index + i;
			if(index_to_copy >= PH_ARRAY_SIZE_MAX)
				index_to_copy -= PH_ARRAY_SIZE_MAX;

			time_ordered_array[i] = ph_values[index_to_copy];
		}
	}
	else
	{
		failure_diagnostic += 900;
		ph_flags_ok = false;
		perfect_pass = false;
		return displayed_ph_running_average;
	}

	if((ph_array_index > 0) || (ph_array_index_looped)) //there is data in the array. pH_array_index has been incremented
	{
		//calculate the running average
		int index_limit;
		int total_values_to_average = PH_ARRAY_SIZE_MAX;

		if(fast_profiling)
			total_values_to_average = 5;
		else
			total_values_to_average = 8;

		//need decision on all of this. if we leave it like this, then fast profiling just reduces the delays before and after the scans
		total_values_to_average = 8;

		//total_values_to_average = PH_ARRAY_SIZE_MAX; //sg!

		if(ph_array_index_looped)
			index_limit = PH_ARRAY_SIZE_MAX;
		else
			index_limit = ph_array_index;

		double ph_sum = 0, std_dev_sum = 0;
		int count = 0;
		double std_dev;

		double ph_median, copied_array[PH_ARRAY_SIZE_MAX] = {0};
		//first, find the raw average
		for(int i = 0; i < index_limit; i++)
		{
			if(
					(
							(i < ph_array_index) && (i >= (ph_array_index - total_values_to_average))

					)

					||

					(
							(ph_array_index_looped && (i >= (ph_array_index + PH_ARRAY_SIZE_MAX - total_values_to_average)))
					)

				)
			{
				ph_sum = ph_sum + ph_values[i];
				copied_array[i] = ph_values[i]; //fill copied_array with useful values
				count++;
			}
		}

		qsort(copied_array, PH_ARRAY_SIZE_MAX, sizeof(double), cmpfunc_d);
		if (count % 2 != 0)
		   	ph_median = copied_array[count / 2];
		   // If the number of elements is even, return the average of the two middle elements
		else
		{
			int mid_index = count / 2;
			ph_median = (copied_array[mid_index - 1] + copied_array[mid_index]) / 2.0;
		}

		calculated_running_avg_ph = ph_median; //so that std_dev is the deviation from the median - this is not the final calculation

		//////////////////// this section looks at the progression of the values in the array, looking for scatter and real excursions
		if(count >= 6)
		{
			bool change_detected = false;
			int suspect_ph_count_threshold = 3;

			if(scan_long_interval_flag) //need to be responsive to any changes
			{
				suspect_ph_count_threshold = 1;
				alpha = 0.5;
			}

			if(displayed_ph_running_average == PH_DEFAULT_OUT_OF_RANGE_VALUE)
				displayed_ph_running_average = scan_ph;

			if((scan_ph > copied_array[1]) || ((scan_ph - displayed_ph_running_average) > 0.5))
			{
				if(ph_decreasing) //this pH is high, so previously decreasing indicates lots of scatter and we've changed direction
					suspect_ph_count = 1;
				else
				{
					suspect_ph_count++;
					ph_increasing = true;
				}
				ph_decreasing = false;
			}

			//this check is to see if new values coming in are at the edges of the spread
			if((scan_ph < copied_array[count - 2]) || ((scan_ph - displayed_ph_running_average) < -0.5))
			{
				if(ph_increasing) //this indicates lots of scatter and we've changed direction
					suspect_ph_count = 1;
				else
				{
					suspect_ph_count++;
					ph_decreasing = true;
				}
				ph_increasing = false;
			}

			if(suspect_ph_count >= suspect_ph_count_threshold) //let's act
			{
				change_detected = true;
				suspect_ph_count = 0;
			}

			int extreme_index = count / 6.0; //will represent the outer elements of the sorted array

			//this check is to see if the array has crept to a point where an unchanging average is now near the edges of the spread
			if((displayed_ph_running_average > copied_array[extreme_index]) || (displayed_ph_running_average < copied_array[count - 1 - extreme_index]))
			{
				change_detected = true;
				alpha = 0.5; //bring it into line quickly
			}

			if(change_detected)
			{
				double sub_array[8] = {0}; //minimum of 6 values in the array, but the code below will fill in any zeroes

				for(int i = 0; i < 8; i++)
				{
					//let's look at the time ordered array and create a subarray of the latest values
					sub_array[i] = time_ordered_array[PH_ARRAY_SIZE_MAX - i - 1];

					if(fabs(sub_array[i] - ph_median) > 1.0) //doesn't look like valid data
						sub_array[i] = ph_median;
					else if((sub_array[i] - ph_median) > 0.1)
						sub_array[i] = ph_median + 0.1; //cap it
					else if((sub_array[i] - ph_median) < -0.1)
						sub_array[i] = ph_median - 0.1; //cap it
				}

				//use the (smoothed) last value to update the average
				double fitted_value = quadratic_fit_end_value(sub_array, 0, 8, true);

				//sg! if interval and scattered data then this could give crazy swings
				displayed_ph_running_average = alpha * fitted_value + (1.0 - alpha) * displayed_ph_running_average;
			}

		}
		else
			use_array_calculation = true;
		//////////////////// if we don't have enough information in the array, just go ahead, remove outliers and do a raw calculation

		//find standard deviation
		for(int i = 0; i < index_limit; i++)
			if(((i < ph_array_index) && (i >= (ph_array_index - total_values_to_average))) || ((ph_array_index_looped && (i >= (ph_array_index + PH_ARRAY_SIZE_MAX - total_values_to_average)))))
				std_dev_sum += pow(ph_values[i] - calculated_running_avg_ph, 2);

		std_dev = sqrt(std_dev_sum / count);
		last_ph_std_dev = std_dev;

		//now reject any outliers, calculating new average along the way
		count = 0;
		ph_sum = 0;
		ph_failed_only_as_outlier = false;
		double tolerance_factor = 2.0;
		double def_include_limit = 0.15, hard_limit = 0.2;

		if(ocean_scan)
		{
			def_include_limit = 0.15;
			hard_limit = 0.2;
		}
		else if(fresh_scan)
		{
			def_include_limit = 0.3;
			hard_limit = 0.3;
		}
		else if(vf_scan)
		{
			def_include_limit = 0.3;
			hard_limit = 0.3;
		}

		if((ph_array_index == 3) && !ph_array_index_looped) //only 3 values in the array, get rid of the worst one
			tolerance_factor = 1.0;

		for(int i = 0; i < index_limit; i++)
			if(((i < ph_array_index) && (i >= (ph_array_index - total_values_to_average))) ||((ph_array_index_looped && (i >= (ph_array_index + PH_ARRAY_SIZE_MAX - total_values_to_average)))))
			{
				double diff = ph_values[i] - calculated_running_avg_ph;
				if(((fabs(diff) < (tolerance_factor*std_dev)) || (fabs(diff) < def_include_limit)) && (fabs(diff) < hard_limit)) //include this value
				{
					count = count + 1;
					ph_sum = ph_sum + ph_values[i];
				}
				else if(i == (ph_array_index - 1)) //last entry was an outlier, fail this scan
				{
					if(ph_flags_ok) //if this scan failed, this would refer to previous array entry
					{
						failure_diagnostic += 100;
						ph_flags_ok = false;
						perfect_pass = false;
						ph_failed_only_as_outlier = true;

						if(outlier_increasing && (diff > 0))
							outlier_trend_count++;
						if(!outlier_increasing && (diff < 0))
							outlier_trend_count++;

						if(outlier_increasing != (diff > 0)) //change of direction, start again
						{
							outlier_trend_count = 1;
							outlier_increasing = (diff > 0);
						}

						int outlier_trend_count_trigger = 1;
						if(scan_interval_flag)
							outlier_trend_count_trigger = 3;
						if(outlier_trend_count > outlier_trend_count_trigger)
							force_start_change = true;
					}
				}
			}

		if(ph_flags_ok) //passed, but not an outlier
			outlier_trend_count = 0;

		if(count > 0)
		{
			calculated_running_avg_ph = ph_sum / count;
		}
	}

	if(ph_array_index >= PH_ARRAY_SIZE_MAX)
	{
		ph_array_index = ph_array_index - PH_ARRAY_SIZE_MAX;
		ph_array_index_looped = true;
	}
	
	calculate_scan_health();

	if(use_array_calculation)
		displayed_ph_running_average = calculated_running_avg_ph;

	return displayed_ph_running_average;
}


uint8_t get_transducer_health(int sensor_number)
{
    uint8_t failed_electrodes = 0, electrode_failure_threshold = BAD_ELECTRODE_HEALTH_THRESHOLD;
    uint8_t transducer_health, loops_limit_advise_abrade = 4;

    uint8_t loops_since_last_good_ph = eeprom_read_loops_since_last_good_ph();
    if(scan_interval_flag)
    	loops_limit_advise_abrade = 12; //only do 3 electrodes per loop, so 12 loops means electrodes have failed ~4 times

    bool both_irefs_bad = (iref_health_bad_count > IREF_BAD_COUNT_THRESHOLD); //since we're using double irefs now need to change the name SG070624

	if((sensor_number == 0) || (sensor_number > GAIN_ARRAY_SIZE))
		sensor_number = 1;
	
	good_electrodes = 0;

	for(int i = 0; i < number_of_ph_electrodes; i++)
	{
		if(electrode_health_values[i] >= electrode_failure_threshold)
			failed_electrodes++;
		else
			good_electrodes++;
	}
	
	if(!immersed_flag)
		transducer_health = 4;
	else if(both_irefs_bad) //both irefs have failed >= IREF_FAILURE_THRESHOLD scans
		transducer_health = 3;
	else if(no_good_iref_scans && !using_previous_iref)
		transducer_health = 5;
	else if(loops_since_last_good_ph >= loops_limit_advise_abrade)
		transducer_health = 2;
	else if((qaqc_reply.running_average_ph == PH_DEFAULT_OUT_OF_RANGE_VALUE) && (eeprom_read_loops_since_start() < 3))
		transducer_health = 6;
	else
	{
		//we have the option of combining scan_health which reflects the passes and fails and the overall health of the electrodes.
		//for example, a change of salinity might have led to lots of temporary failures, but we don't want to abrade yet
		transducer_health = scan_health;
		if((transducer_health == 2) && (good_electrodes >= 3)) //sg! get agreement. we don't want to abrade yet, still have enough working
			transducer_health = 1;

		if((qaqc_reply.running_average_ph == PH_DEFAULT_OUT_OF_RANGE_VALUE) && (transducer_health != 2))
			transducer_health = 6; //if we don't have a valid pH to show, and we're not advising abrade yet, just say to keep waiting
	}

	eeprom_write_array_health(transducer_health);

	if((loops_since_last_good_ph == loops_limit_advise_abrade) && !array_cleared)//data out of date. get rid of it
	{
		clear_ph_array();
		array_cleared = true; //so we don't clear the array every single time this function is called
	}

	return transducer_health;
}


uint8_t get_electrode_health_value(int sensor_number, int second_sensor_number, double calculated_ph, int gain_change)
{
	uint8_t max_failure_count = MAX_HEALTH;
	
	if(healthwatch || ocean_scan)
	{
		if((calculated_ph == PH_DEFAULT_OUT_OF_RANGE_VALUE) || !ph_flags_ok)//scan failed because of calculation or outlier
		{
			electrode_health_values[sensor_number - 1]++;
			if(second_sensor_number != 0)
				electrode_health_values[second_sensor_number - 1]++;
		}
		else //passed
		{
			if(electrode_health_values[sensor_number - 1] > 0)
				electrode_health_values[sensor_number - 1]--;
			if(second_sensor_number != 0)
				if(electrode_health_values[second_sensor_number - 1] > 0)
					electrode_health_values[second_sensor_number - 1]--;
		}

		if(electrode_health_values[sensor_number - 1] > max_failure_count)
			electrode_health_values[sensor_number - 1] = max_failure_count;

		if(second_sensor_number != 0)
			if(electrode_health_values[second_sensor_number - 1] > max_failure_count)
				electrode_health_values[second_sensor_number - 1] = max_failure_count;
	}

	return electrode_health_values[sensor_number - 1];
}

void clear_ph_array(void)
{
	for(int i = 0; i < PH_ARRAY_SIZE_MAX; i++)
		ph_values[i] = 0;
				
	ph_array_index = 0;
	ph_array_index_looped = false;

	displayed_ph_running_average = PH_DEFAULT_OUT_OF_RANGE_VALUE;
	last_temperature = VALUE_UNKNOWN;

	electrode_array_looped = false; //if clear_ph_array is called because all irefs fail then check if this line impacts anything badly SG070624
	
	eeprom_save_ph_array();
}

void clear_peak_potential_offsets(void)
{
	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		peak_potential_offsets[i] = 0;

	eeprom_save_peak_potential_offsets();
}

void clear_electrode_health(void)
{
	for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++)
		electrode_health_values[i] = 0;
	
	eeprom_save_electrode_health();
}

void set_default_gains(void)
{
	int default_gain = 3;

    for(int i = 0; i < NUMBER_OF_PH_ELECTRODES_MAX; i++) //don't touch iref gains
    {
		if(ocean_scan)
			default_gain = PH_GAIN_RESISTOR_SETTING_OCEAN;
		else
			default_gain = PH_GAIN_RESISTOR_SETTING_FRESH;

		electrode_gain_values[i] = default_gain;
    }

    reset_electrode_gains_flag = false;
	eeprom_save_electrode_gains();
}

double quadratic_fit_end_value(double sub_array[], int start, int size, bool end_or_next)
{
	   float xi, yi, yi_norm, x_bar, x2_bar, y_bar;
	   float sumX = 0.0, sumY = 0.0, sumX2 = 0.0;
	   float Sxx = 0.0, Sxy = 0.0, Sxx2 = 0.0, Sx2x2 = 0.0, Sx2y = 0.0;
	   float yi_max = 32767.0;

	   if((size - start) == 0)
		   return sub_array[size - 1];

	   int num_elements = size - start;

	   for (int i = start; i < size; i++)
	   {
	       xi = i;
	       yi = sub_array[i];
	       yi_norm = yi / yi_max;

	       sumY += yi_norm;
	       sumX += xi;
	       sumX2 += xi * xi;
	   }
	   x_bar = sumX / num_elements;
	   y_bar = sumY / num_elements;
	   x2_bar = sumX2 / num_elements;

	   // Calculate the sums of various terms
	   for (int i = start; i < size; i++)
	   {
	       xi = i;
	       yi = sub_array[i];
	       yi_norm = yi / yi_max;

	       Sxx += (xi - x_bar) * (xi - x_bar);
	       Sxy += (xi - x_bar) * (yi_norm - y_bar);
	       Sxx2 += (xi - x_bar) * (xi * xi - x2_bar);
	       Sx2x2 += (xi * xi - x2_bar) * (xi * xi - x2_bar);
	       Sx2y += (xi * xi - x2_bar) * (yi_norm - y_bar);
	   }

	   // Formulate the normal equations for the least squares fit
	   float b = (Sxy * Sx2x2 - Sx2y * Sxx2) / (Sxx * Sx2x2 - Sxx2 * Sxx2);
	   float c = (Sx2y * Sxx - Sxy * Sxx2) / (Sxx * Sx2x2 - Sxx2 * Sxx2);
	   float a = y_bar - b * x_bar - c * x2_bar;

	   int index = size - 1;
	   if(end_or_next)
		   index = size - 1;
	   else
		   index = size;

	   float result = a + b * index + c * index * index;

	   //sg! consider cap on value - twice maximum? in case of crazy equation
	   if (result < 0.0)
	       result = 0.0;

	   return (result * yi_max);
}

double logistic(double x, double min, double max, double k, double mid)
{
    return min + (max - min) / (1.0 + exp(-k * (x - mid)));
}

double median_of_three(double a, double b, double c)
{
    if ((a >= b && a <= c) || (a <= b && a >= c))
        return a;
    else if ((b >= a && b <= c) || (b <= a && b >= c))
        return b;
    else
        return c;
}

void calculate_scan_health(void)
{
	double alpha  = 0.1; //original idea, but numbers below are better

	//scan_health is a double between 0 and 2.999, so that when (int) cast applied it resembles our usual transducer health
	if(!ph_flags_ok) //fail
		scan_health = scan_health + (3.0 - scan_health)*0.05;
	else if(failure_diagnostic != 0) //we passed but how good was the peak?
		scan_health = scan_health + (1.5 - scan_health)*alpha;
	else //perfect pass
		scan_health = scan_health - (scan_health - 0.0)*0.2;

	if(scan_health > 2.999)
		scan_health = 2.999;
}
