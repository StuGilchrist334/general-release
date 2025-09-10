#include "data_smoothing.h"
peak_picking pp;

bool this_scan_clipped = false;
bool ph_flags_ok, perfect_pass = false, no_peaks_found = true;
int outlier_trend_count = 0;
bool outlier_increasing = true, force_start_change = false;

bool ph_peak_value_LL_flag;
bool ph_QAQC1_LL_flag;
bool ph_QAQC2_LL_flag;

bool ph_ocean_QAQC_parameter_change_LL_flag;
bool ph_fresh_QAQC_parameter_change_LL_flag;
bool ph_ocean_QAQC_parameter_change_flag;
bool ph_fresh_QAQC_parameter_change_flag;

bool iref_flags_ok;

bool iref_peak_value_LL_flag;
bool iref_QAQC1_LL_flag;
bool iref_QAQC2_LL_flag;

bool iref_QAQC_parameter_change_LL_flag;
bool iref_QAQC_parameter_change_flag; //for either qaqc

int median_filter_size = 9;
int quad_fit_filter_size = 15;
int box_filter_size = 33;

bool balanced_peak_only_found = false;

float all_ph_peaks[MAX_PEAKS][4];
int all_ph_peaks_count = 0, all_ph_peaks_trusted_index = -1;
int peaks_found = 0;
int peak_id_difference = 30;

void smoothing_function(
	uint16_t size_of_array,
	qaqc_reply_struct *output_data)
{
	float qaqc2_used = 0, qaqc2_i1 = 0, qaqc2_i2 = 0, qaqc2_diff = 0;
	uint16_t peak_value_used = 0, peak_value_i1 = 0, peak_value_i2 = 0, peak_value_diff = 0;
	float dummy;

	int peaks_found_i2 = 0, peaks_found_i1 = 0, peaks_found_diff = 0;

	adc_mark_data_global[0] = adc_mark_data_global[2];
	adc_mark_data_global[1] = adc_mark_data_global[2];
	adc_space_data_global[0] = adc_space_data_global[1];

	//need to work out degree of smoothing to apply dependent on standard deviation
	signal_scatter = compute_local_scatter(adc_mark_space_array_1_global, size_of_array);

	//in the following, I1 is the mark data, I2 is the space. I1 is negative for flavs, positive for iref, I1 is positive for flavs, negative for iref

	//pre-smoothing
	box_filter_size = 33;

	//for I2, convert to floats
	for(int i = 0; i < size_of_array; i++)
		adc_mark_space_array_1_global[i] = (float)adc_space_data_global[i];
	//smooth the floats
	box_filter(adc_mark_space_array_1_global, adc_mark_space_array_2_global, box_filter_size, size_of_array);
	//place back into integer array
	for(int i = 0; i < size_of_array; i++)
		adc_space_data_global[i] = (uint16_t)adc_mark_space_array_2_global[i];

	//for I1, convert to floats
	for(int i = 0; i < size_of_array; i++)
		adc_mark_space_array_1_global[i] = (float)adc_mark_data_global[i];
	//now smooth the floats
	box_filter(adc_mark_space_array_1_global, adc_mark_space_array_2_global, box_filter_size, size_of_array);
	//place back into integer array
	for(int i = 0; i < size_of_array; i++)
		adc_mark_data_global[i] = (uint16_t)adc_mark_space_array_2_global[i];


	//now we are going to find the peaks associated with I1, I2, the difference, and use whatever is dictated by the scan type, ph or iref, vf or other

		//recreate the difference array from these smoothed arrays (albeit integers)
		for(int i = 0; i < size_of_array; i++)
			adc_mark_space_array_1_global[i] =  abs(adc_space_data_global[i] - adc_mark_data_global[i]);

		adc_mark_space_array_1_global[0] = abs(adc_space_data_global[0] - adc_mark_data_global[0]);
		adc_mark_space_array_1_global[1] = abs(adc_space_data_global[1] - adc_mark_data_global[1]);

		//throughout the following sequence of filters, adc_mark_space_array_1_global and adc_mark_space_array_2_global
		//hold the inputs and outputs of each procedure, with an output becoming the input to the next.
		//if any additional filters are used, be sure to change the correct array name in the next calculation.

		//perform a median filter to clean up the data
		median_filter(adc_mark_space_array_1_global, adc_mark_space_array_2_global, median_filter_size, size_of_array);

		//perform some smoothing
		box_filter_size = 65;
		box_filter(adc_mark_space_array_2_global, adc_mark_space_array_1_global, box_filter_size, size_of_array);

		box_filter_size = 33;
		box_filter(adc_mark_space_array_1_global, adc_mark_space_array_2_global, box_filter_size, size_of_array);

		peak_potential_diff_raw = find_peak(adc_mark_space_array_2_global, &peak_potential_diff_balanced, &qaqc2_diff, &peak_value_diff, size_of_array, true, (ph_scan && !current_task.multielectrode_scan)); //find peak of difference
		peaks_found_diff = peaks_found;

		//get I2 again into smoothed float format and find peak
		for(int i = 0; i < size_of_array; i++)
			adc_mark_space_array_1_global[i] = fabs((float)adc_space_data_global[i] - 16383.0);

		box_filter(adc_mark_space_array_1_global, adc_mark_space_array_2_global, box_filter_size, size_of_array);
		dummy = find_peak(adc_mark_space_array_2_global, &peak_potential_i2, &qaqc2_i2, &peak_value_i2, size_of_array, true, false);
		peaks_found_i2 = peaks_found;

		//get I1 again into smoothed float format and find peak
		for(int i = 0; i < size_of_array; i++)
			adc_mark_space_array_1_global[i] = fabs((float)adc_mark_data_global[i] - 16383.0);

		box_filter(adc_mark_space_array_1_global, adc_mark_space_array_2_global, box_filter_size, size_of_array);
		dummy = find_peak(adc_mark_space_array_2_global, &peak_potential_i1, &qaqc2_i1, &peak_value_i1, size_of_array, true, false); //I1 flav negative peak
		peaks_found_i1 = peaks_found;

		//having measured both I1 and I2, in vf we want the 'higher' peak. also check if I2 wasn't a default non-peak. also go back if I1 had no peaks but I2 did
		if(ph_scan && vf_scan)
		{
			if(((peak_potential_i2 > peak_potential_i1) && (peaks_found_i2 > 0)) || ((peaks_found_i2 > 0) && (peaks_found_i1 == 0)))
			{
				peak_potential_displayed_2 = peak_potential_i1; //this is now the 'lower' peak

				peak_potential_used = peak_potential_i2;
				qaqc2_used = qaqc2_i2;
				peak_value_used = peak_value_i2;
			}
			else
			{
				peak_potential_displayed_2 = peak_potential_i2;

				peak_potential_used = peak_potential_i1;
				qaqc2_used = qaqc2_i1;
				peak_value_used = peak_value_i1;
			}
		}
		else
		{
			peak_potential_used = peak_potential_diff_balanced;
			qaqc2_used = qaqc2_diff;
			peak_value_used = peak_value_diff;
			peak_potential_displayed_2 = peak_potential_diff_raw;
		}

	//apply offset to tighten results
	if(ph_scan && ocean_scan) //single pH electrodes
	{
		if(!current_task.multielectrode_scan)
		{
			peak_potential_displayed_2 = peak_potential_diff_balanced; //store the uncorrected (but balanced) value for display
			peak_potential_used += peak_potential_offsets[current_task.sensor_number - 1];
		}
	}

	output_data->peak_potential = peak_potential_used;

	output_data->peak_value = peak_value_used;
	output_data->qaqc_2 = qaqc2_used;

	no_peaks_found = (peaks_found == 0);

	// check values against limits
	if(ph_scan)
	{
		(pp.peak_value < ph_peak_value_LL) ? (ph_peak_value_LL_flag = true) : (ph_peak_value_LL_flag = false);

		if(ocean_scan)
		{
			(pp.QAQC1 < ph_ocean_QAQC1_LL) ? (ph_QAQC1_LL_flag = true) : (ph_QAQC1_LL_flag = false);
			(pp.QAQC2 < ph_ocean_QAQC2_LL) ? (ph_QAQC2_LL_flag = true) : (ph_QAQC2_LL_flag = false);
		}
		else
		{
			(pp.QAQC1 < ph_fresh_QAQC1_LL) ? (ph_QAQC1_LL_flag = true) : (ph_QAQC1_LL_flag = false);
			(pp.QAQC2 < ph_fresh_QAQC2_LL) ? (ph_QAQC2_LL_flag = true) : (ph_QAQC2_LL_flag = false);
		}

		if(ocean_scan)
			((pp.QAQC1 < ph_ocean_QAQC_parameter_change_LL) || (pp.QAQC2 < ph_ocean_QAQC_parameter_change_LL)) ? (ph_ocean_QAQC_parameter_change_LL_flag = true) : (ph_ocean_QAQC_parameter_change_LL_flag = false);
		else
			((pp.QAQC1 < ph_fresh_QAQC_parameter_change_LL) || (pp.QAQC2 < ph_fresh_QAQC_parameter_change_LL)) ? (ph_fresh_QAQC_parameter_change_LL_flag = true) : (ph_fresh_QAQC_parameter_change_LL_flag = false);

		ph_flags_ok = true;
		ph_flags_ok = ph_flags_ok && !this_scan_clipped;
		perfect_pass = ph_flags_ok;

		if(ph_peak_value_LL_flag || ph_QAQC1_LL_flag || ph_QAQC2_LL_flag) //failed limits
		{
			perfect_pass = false;

			if(peaks_found == 0)
			{
				failure_diagnostic += 50000;
				ph_flags_ok = false;
			}
			else if(balanced_peak_only_found)
			{
				if(scan_health >= 1.7) //we are in trouble, so use the peak sg! may want to change, being generous might throw start potential off
					failure_diagnostic += 20000;
				else
				{
					ph_flags_ok = false;
					failure_diagnostic += 40000;
				}
			}
			else //we found a peak that failed limits
			{
				if(scan_health >= 1.7) //we are in trouble, so use the peak
					failure_diagnostic += 10000;
				else
				{
					ph_flags_ok = false;
					failure_diagnostic += 30000;
				}
			}
		}
		else if(balanced_peak_only_found) //passed, but could be a balanced only peak
		{
			perfect_pass = false;

			if(scan_health >= 1.7) //we are in trouble, so use the peak sg! may want to change, being generous might throw start potential off
				failure_diagnostic += 20000;
			else
			{
				ph_flags_ok = false;
				failure_diagnostic += 40000;
			}
		}


		ph_ocean_QAQC_parameter_change_flag = false;
		ph_fresh_QAQC_parameter_change_flag = false;

		//set flag if all ok except qaqc
		if(ocean_scan)
		{
			if(!ph_peak_value_LL_flag && ph_ocean_QAQC_parameter_change_LL_flag)
			{
				if(peaks_found != 0)
					ph_ocean_QAQC_parameter_change_flag = true; //non-qaqc limits all pass, but qaqc is low
			}
		}
		else
		{
			if(!ph_peak_value_LL_flag && ph_fresh_QAQC_parameter_change_LL_flag)
			{
				if(peaks_found != 0)
					ph_fresh_QAQC_parameter_change_flag = true; //non-qaqc limits all pass, but qaqc is low
			}
		}

		ph_ocean_QAQC_parameter_change_flag = false; //action removed 1/10/24
	}
	else if(iref_scan)
	{

		(pp.peak_value < iref_peak_value_LL) ? (iref_peak_value_LL_flag = true) : (iref_peak_value_LL_flag = false);
		(pp.QAQC1 < iref_QAQC1_LL) ? (iref_QAQC1_LL_flag = true) : (iref_QAQC1_LL_flag = false);
		(pp.QAQC2 < iref_QAQC2_LL) ? (iref_QAQC2_LL_flag = true) : (iref_QAQC2_LL_flag = false);
		
		((pp.QAQC1 < iref_QAQC_parameter_change_LL) || (pp.QAQC2 < iref_QAQC_parameter_change_LL)) ? (iref_QAQC_parameter_change_LL_flag = true) : (iref_QAQC_parameter_change_LL_flag = false);

		if(!ocean_scan && (current_task.scan_number_current < 3))
		{
			iref_QAQC1_LL_flag = false;
		}

		iref_flags_ok = true;
		iref_flags_ok = iref_flags_ok && !this_scan_clipped;

		if(iref_peak_value_LL_flag || iref_QAQC1_LL_flag || iref_QAQC2_LL_flag)
		{
			if(!balanced_peak_only_found) //if we are desperate for any small peak, ignore limits
			{
				iref_flags_ok = false;
				if(peaks_found == 0)
					failure_diagnostic += 50000;
				else
				{
					failure_diagnostic += 10000;
					emergency_iref_peak_found = true;
					emergency_iref_peak_potential = peak_potential_used;
				}
			}
			else
			{
				emergency_iref_peak_found = true;
				emergency_iref_peak_potential = peak_potential_used;
				iref_flags_ok = false;
				failure_diagnostic += 40000;
			}
		}

		if(iref_flags_ok)
		{
			emergency_iref_peak_found = true; //always keep anything that passed. but this will be rejected in update_task_after_scan if in the first 3 scans
			emergency_iref_peak_potential = peak_potential_used;
		}

		iref_QAQC_parameter_change_flag = false;

		if(!iref_peak_value_LL_flag && iref_QAQC_parameter_change_LL_flag)
		{
			if(peaks_found != 0)
				iref_QAQC_parameter_change_flag = true; //non-qaqc limits all pass, but qaqc is low
		}
	}
}

float quadratic_fit_centre_value(int sub_array[], int size)
{
	   float xi, yi, yi_norm, x_bar, x2_bar, y_bar;
	   float sumX = 0.0, sumY = 0.0, sumX2 = 0.0;
	   float Sxx = 0.0, Sxy = 0.0, Sxx2 = 0.0, Sx2x2 = 0.0, Sx2y = 0.0;
	   float yi_max = 32767.0;

	   if(size == 1)
		   return sub_array[0];

	   for (int i = 0; i < size; i++)
	   {
	       xi = i;
	       yi = sub_array[i];
	       yi_norm = yi / yi_max;

	       sumY += yi_norm;
	       sumX += xi;
	       sumX2 += xi * xi;
	   }
	   x_bar = sumX / size;
	   y_bar = sumY / size;
	   x2_bar = sumX2 / size;

	   // Calculate the sums of various terms
	   for (int i = 0; i < size; i++)
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

	   float centre_value = (size - 1) / 2;

	   float result = a + b * centre_value + c * centre_value * centre_value;
	   if (result > 1.0)
	       result = 1.0;
	   if (result < 0.0)
	       result = 0.0;

	   return (float)(result * yi_max);
}

bool find_maximum(float* input_array, int start_index, int end_index, int* peak_index, float* peak_value)
{
	float last_value = *(input_array + start_index);
	bool going_up = false;

	//if no maximum is found, set up defaults
	*peak_value = *(input_array + start_index);
	*peak_index = 0;

	for(int i = start_index; i < end_index; i++)
	{
		if(!going_up)
		{
			if(*(input_array + i) > last_value) //minimum found
				going_up = true;
		}
		else
		{
			if(*(input_array + i) < last_value) //peak found
			{
				*peak_index = i - 1;
				*peak_value = *(input_array + i - 1);
				going_up = false;
				return true;
			}
		}

		last_value = *(input_array + i);
	}

	return false;
}

bool find_minimum(float* input_array, int start_index, int end_index, int* peak_index, float* peak_value)
{
	float last_value = *(input_array + start_index);
	bool going_up = true;

	//if no maximum is found, set up defaults
	*peak_value = *(input_array + start_index);
	*peak_index = 0;

	for(int i = start_index; i < end_index; i++)
	{
		if(going_up)
		{
			if(*(input_array + i) < last_value) //maximum found
				going_up = false;
		}
		else
		{
			if(*(input_array + i) > last_value) //minimum found
			{
				*peak_index = i - 1;
				*peak_value = *(input_array + i - 1);
				going_up = true;
				return true;
			}
		}

		last_value = *(input_array + i);
	}

	return false;
}

void find_baseline_1(float* input_array, int start_index, int end_index, int* baseline_1_index, float* baseline_1_value)
{
	float last_value = *(input_array + start_index);
	bool minimum_found = false;
	int minimum_index = end_index;

	//if no minimum is found, set up defaults
	*baseline_1_value = *(input_array + end_index);
	*baseline_1_index = end_index;

	for(int i = start_index - 1; i > end_index; i--)
	{
		if(minimum_found && (abs(i - minimum_index) > 5)) //we've gone on a few points and nothing new, this minimum looks ok
			break;
		if(*(input_array + i) > last_value) //going up
		{
			if(!minimum_found) //save this minimum, otherwise we're just going up after the last one
			{
				minimum_index = i + 1;
				minimum_found = true;
			}
		}
		else if(minimum_found) //going down still, last one was a false alarm
			minimum_found = false; //so keep going

		last_value = *(input_array + i);
	}

	*baseline_1_index = minimum_index;
	*baseline_1_value = *(input_array + minimum_index);
}

void find_baseline_2(float *input_array, int start_index, int end_index, int* baseline_2_index, float* baseline_2_value)
{
	float last_value = *(input_array + start_index);
	bool minimum_found = false;
	int minimum_index = end_index;

	//if no minimum is found, set up defaults
	*baseline_2_value = *(input_array + end_index);
	*baseline_2_index = end_index;

	for(int i = start_index + 1; i < end_index; i++)
	{
		if(minimum_found && (abs(i - minimum_index) > 5)) //we've gone on a few points and nothing new, this minimum looks ok
			break;
		if(*(input_array + i) > last_value) //going up
		{
			if(!minimum_found) //save this minimum, otherwise we're just going up after the last one
			{
				minimum_index = i - 1;
				minimum_found = true;
			}
		}
		else if(minimum_found) //going down still, last one was a false alarm
			minimum_found = false; //so keep going

		last_value = *(input_array + i);
	}

	*baseline_2_index = minimum_index;
	*baseline_2_value = *(input_array + minimum_index);
}

uint16_t calculate_median(int sub_array[], int size)
{
	qsort(sub_array, size, sizeof(uint16_t), cmpfunc);

	return sub_array[size/2];
}

int cmpfunc (const void * a, const void * b)
{
   return ( *(uint16_t*)a - *(uint16_t*)b );
}

float get_signal_to_noise(float *adc_mark_space_array, int peak_index) //actually noise, not signal to noise. we look for the lowest value of this
{
	int half_spread = 32;
	float std_dev, std_dev_sum = 0;

	//find the deviation of the raw data from the fitted data
	for(int i = (peak_index - half_spread); i <= (peak_index + half_spread); i++)
	{
		float deviation = abs(adc_mark_data_global[i] - adc_space_data_global[i]) - adc_mark_space_array[i];
		std_dev_sum += pow(deviation, 2);
	}

	std_dev = sqrt(std_dev_sum / (2*half_spread + 1));

	return std_dev;
}

void median_filter(float *input_array, float *output_array, int filter_size_default, int size_of_array)
{
	int filter_size, half_filter_size;
	int half_filter_size_default = (filter_size_default - 1) / 2; //this is the number of elements on either side of the point of interest

	for(int i = 0; i < size_of_array; i++)
	{
		if(i < half_filter_size_default)
		{
			filter_size = 2*i + 1;
			half_filter_size = i;
		}
		else
		{
			filter_size = filter_size_default;
			half_filter_size = (filter_size_default - 1) / 2;
		}

		int sub_array[filter_size];
		//create a subarray around the point of interest
		for (int j = 0; j < filter_size; j++)
			sub_array[j] = *(input_array + i + j - half_filter_size);

		*(output_array + i) = calculate_median(sub_array, filter_size);
	}
}

void box_filter(float *input_array, float *output_array, int filter_size_default, int size_of_array)
{
	int filter_size, half_filter_size;
	int half_filter_size_default = (filter_size_default - 1) / 2; //this is the number of elements on either side of the point of interest

	for(int i = 0; i < size_of_array; i++)
	{
		if(i < half_filter_size_default)
		{
			filter_size = 2*i + 1;
			half_filter_size = i;
		}
		else if((size_of_array - 1 - i) < half_filter_size_default)
		{
			filter_size = 2*(size_of_array - 1 - i) + 1;
			half_filter_size = size_of_array - 1 - i;
		}
		else
		{
			filter_size = filter_size_default;
			half_filter_size = (filter_size_default - 1) / 2;
		}

		float filter_sum = 0;

		//sum the elements around the point of interest
		for (int j = 0; j < filter_size; j++)
			filter_sum += *(input_array + i + j - half_filter_size);

		*(output_array + i) = filter_sum / (float)filter_size;
	}
}

void quad_fit_filter(float *input_array, float *output_array, int filter_size_default, int size_of_array)
{
	int filter_size, half_filter_size;
	int half_filter_size_default = (filter_size_default - 1) / 2; //this is the number of elements on either side of the point of interest

	for(int i = 0; i < size_of_array; i++)
	{
		if(i < half_filter_size_default)
		{
			filter_size = 2*i + 1;
			half_filter_size = i;
		}
		else
		{
			filter_size = filter_size_default;
			half_filter_size = (filter_size_default - 1) / 2;
		}

		int sub_array[filter_size];

		//create a subarray around the point of interest
		for (int j = 0; j < filter_size; j++)
			sub_array[j] = *(input_array + i + j - half_filter_size);

		*(output_array + i) = quadratic_fit_centre_value(sub_array, filter_size);
	}
}

float compute_stddev(float *arr, int size)
{
    if (size <= 1) return 0.0;  // Avoid division by zero

    float mean = 0.0, variance = 0.0;

    // First pass: Compute mean
    for (int i = 0; i < size; i++) {
        mean += arr[i];
    }
    mean /= size;

    // Second pass: Compute variance
    for (int i = 0; i < size; i++) {
        float diff = arr[i] - mean;
        variance += diff * diff;
    }
    variance /= size;  // Use 'size' for population stddev, use 'size-1' for sample stddev

    return sqrt(variance);  // Standard deviation is sqrt of variance
}

float compute_local_scatter(float *arr, int size)
{
    if (size < 2) return 0.0;

    float sum_diff = 0.0;
    for (int i = 1; i < size; i++) {
        sum_diff += fabs(arr[i] - arr[i - 1]); // Absolute change
    }

    return sum_diff / (size - 1);  // Mean absolute difference
}

void gradient_filter(float *input_array, float *output_array, int step, int size_of_array)
{
    // If not enough data to apply central difference with the given step, fill with zeros
    if (size_of_array < 2 * step + 1) {
        for (int i = 0; i < size_of_array; i++) {
            output_array[i] = 0.0f;
        }
        return;
    }

    // Central difference using step distance
    for (int i = step; i < size_of_array - step; i++) {
        output_array[i] = (input_array[i + step] - input_array[i - step]) / (2.0f * step);
    }

    // Forward difference near the beginning
    for (int i = 0; i < step; i++) {
        output_array[i] = (input_array[i + 1] - input_array[i]) / 1.0f;
    }

    // Backward difference near the end
    for (int i = size_of_array - step; i < size_of_array - 1; i++) {
        output_array[i] = (input_array[i] - input_array[i - 1]) / 1.0f;
    }

    // Final edge point
    output_array[size_of_array - 1] = (input_array[size_of_array - 1] - input_array[size_of_array - 2]) / 1.0f;
}

float find_peak(float *input_array, float *peak_potential, float *qaqc, uint16_t *value_at_peak, uint16_t size_of_array, bool maximum,  bool use_stored_peaks)
{
	bool normalise_baselines = true;
	int peak_index = 0, first_peak_index = 0, sub_array_peak_index = 0, baseline_1_index = 0,  baseline_2_index = 0;

	float QAQC1 = 0, QAQC2 = 0, corrected_QAQC = 0, first_peak_value = 0, sub_array_peak_value = 0, baseline_1_value = 0, baseline_2_value = 0;
	uint16_t peak_value = 0;
	int ignore_points_at_end_of_array = 50, ignore_points_at_start_of_array = 50;
	bool peak_found = true; //to start the loop off

	peaks_found = 0;

	memset(&pp, 0, sizeof(pp));

	balanced_peak_only_found = false;

	if(ph_scan)
		baseline_2_index = ignore_points_at_start_of_array;//index of 50 means we omit the first 50 points to avoid false peaks
	else if(iref_scan)
		baseline_2_index = 20; //index of 20 means we omit the first 20 points to avoid false peaks

	while(peak_found && (peaks_found < MAX_PEAKS))
	{
		if(maximum)
			peak_found = find_maximum(input_array, baseline_2_index, size_of_array - ignore_points_at_end_of_array, &first_peak_index, &first_peak_value);
		else
		{
			//too complicated to balance when looking for a negative peak, just find one and get out
			peak_found = find_minimum(input_array, baseline_2_index, size_of_array - ignore_points_at_end_of_array, &first_peak_index, &first_peak_value);

			peaks_found = 1;
			*peak_potential = dac_output_voltage_data_global[first_peak_index];
			*qaqc = 0;
			*value_at_peak = first_peak_value;

			return *peak_potential;
		}
/*
		bool blacklisted = false;
		if(peak_found && use_stored_peaks)
		{
			blacklisted = blacklisted_peak(dac_output_voltage_data_global[first_peak_index]);
			if(blacklisted)
				peak_found = false;
		}
*/
		bool try_balancing = false;

		//we tried to find a peak, either on the first pass or subsequent
		//if it's the first and we don't find anything, and there's something to relate to, try balancing
		try_balancing = (peaks_found == 0);
		try_balancing = try_balancing && (!peak_found);
		try_balancing = try_balancing && ph_scan;
		try_balancing = try_balancing && (avg_ph_peak_potential_stored || first_good_ph_data_found || (eeprom_read_loops_since_last_good_ph() > 0));

		if(try_balancing) //when we balance in the absence of any peak, use the last known good potential
		{
			for(int i = 0; i < size_of_array; i++)
			{
				if(dac_output_voltage_data_global[i] < avg_ph_peak_potential)
				{
					first_peak_index = i;
					baseline_1_index = i - 50;
					baseline_2_index = i + 50; //check to constrain this
					break;
				}
			}
		}

		if(peak_found || try_balancing)
		{
			bool second_peak_found = false;

			if(peak_found)
			{
				if(iref_scan && ocean_scan)
					first_peak_potential = (uint32_t)(abs(dac_output_voltage_data_global[first_peak_index]));

				find_baseline_1(input_array, first_peak_index, 1, &baseline_1_index, &baseline_1_value);
				find_baseline_2(input_array, first_peak_index, size_of_array - 1, &baseline_2_index, &baseline_2_value);

				peak_index = first_peak_index;
				peak_value = first_peak_value;
				QAQC1 = peak_value - baseline_1_value;
				QAQC2 = peak_value - baseline_2_value;
			}

			//if(ph_scan)
			//	normalise_baselines = false; //let's not confuse things during development

			if(normalise_baselines)
			{
				//attempt to balance the baselines and refind the peak. just do this once for now
				int correction_start_index = baseline_1_index;
				int correction_end_index = baseline_2_index;

				//it's possible baselines are at ends of array and not real minimima. keep the values closer to the peak
				if((first_peak_index - correction_start_index) > 100)
					correction_start_index = first_peak_index - 100;
				if((correction_end_index - first_peak_index) > 100)
					correction_end_index = first_peak_index + 100;

				float* peak_sub_array = (float*)malloc((correction_end_index - correction_start_index + 1) * sizeof(float));
				float correction_start_value = input_array[correction_start_index];
				float correction_end_value = input_array[correction_end_index];

				for(int i = correction_start_index; i <= correction_end_index; i++)
				{
					float correction = (correction_start_value - correction_end_value) * (i - correction_start_index) / (correction_end_index - correction_start_index);
					float value = correction + adc_mark_space_array_2_global[i];
					peak_sub_array[i - correction_start_index] = value;
				}

				second_peak_found = find_maximum(peak_sub_array, 0, correction_end_index - correction_start_index, &sub_array_peak_index, &sub_array_peak_value);
/*
				blacklisted = false;
				if(second_peak_found && use_stored_peaks) //check again for rogue peak
				{
					blacklisted = blacklisted_peak(dac_output_voltage_data_global[sub_array_peak_index + correction_start_index]);
					if(blacklisted)
						second_peak_found = false;
				}
*/
				if(second_peak_found)
				{
					peak_index = sub_array_peak_index + correction_start_index;
					peak_value = sub_array_peak_value; //this is a calculated value after baseline balancing
					corrected_QAQC = peak_value - correction_start_value;
					if(corrected_QAQC >= 0) //sg! look into way to get positive numbers each time
					{
						QAQC1 = corrected_QAQC;
						QAQC2 = QAQC1;
					}

					if(!peak_found && second_peak_found)
						balanced_peak_only_found = true;
				}

				free(peak_sub_array);
			}

			bool valid_peak = true;

			if(iref_scan && !ocean_scan)
				valid_peak = valid_peak && (dac_output_voltage_data_global[peak_index] >= 750);
			else if(iref_scan && ocean_scan)
				valid_peak = valid_peak && (dac_output_voltage_data_global[peak_index] >= 500);

			valid_peak = valid_peak && (abs(peak_index - baseline_1_index) > 5) && (abs(peak_index - baseline_2_index) > 5);
			valid_peak = valid_peak && (abs(baseline_2_index - baseline_1_index) > 20);

			if((peak_found || second_peak_found) && valid_peak)
			{
				pp.peaks[peaks_found][0] = peak_index;
				pp.peaks[peaks_found][1] = baseline_1_index;
				pp.peaks[peaks_found][2] = baseline_2_index;
				pp.peaks[peaks_found][3] = peak_value;
				pp.peaks[peaks_found][4] = QAQC1;
				pp.peaks[peaks_found][5] = QAQC2;
				pp.peaks[peaks_found][6] = get_signal_to_noise(input_array, peak_index); //sg! very specific to I1 and I2 difference
				pp.peaks[peaks_found][7] = 1; //to indicate array entry contains peak data
				pp.peaks[peaks_found][8] = first_peak_index;
				pp.peaks[peaks_found][9] = dac_output_voltage_data_global[peak_index];
				peaks_found++;
			}
		}
	}
/*
	//update the peaks array.
	if(use_stored_peaks)
	{
		float tune_factor = 0.3;
		for(int i = 0; i < MAX_PEAKS; i++)
		{
			bool got_a_match = false;
			if(pp.peaks[i][7] == 1) //array entry contains peak data
			{
				for(int j = 0; j < MAX_PEAKS; j++)
				{
					if(fabs(pp.peaks[i][9] - all_ph_peaks[j][0]) < peak_id_difference)
					{
						got_a_match = true;
						all_ph_peaks[j][0] = tune_factor*pp.peaks[i][9] + (1 - tune_factor)*all_ph_peaks[j][0]; //peak
						all_ph_peaks[j][1] = tune_factor*pp.peaks[i][9] + (1 - tune_factor)*all_ph_peaks[j][1]; //qaqc
					}
				}
				if(!got_a_match)
				{
					all_ph_peaks[all_ph_peaks_count][0] = pp.peaks[i][9];
					all_ph_peaks[all_ph_peaks_count][1] = pp.peaks[i][4];
					all_ph_peaks_count++;
					if(all_ph_peaks_count >= MAX_PEAKS)
						clean_all_ph_peaks(false);
				}
			}
		}
	}
*/
	//choose best peak
	int best_peak_index = -1;

	if(peaks_found > 0)
	{
		if(iref_scan)
		{
			//if((all_ph_peaks_trusted_index >= 0) && use_stored_peaks)
			//	best_peak_index = match_with_trusted_peak();

			if(best_peak_index < 0) //didn't find a good match, or we didn't use the established peaks
			{
				if(first_good_iref_data_found) //so we have a good peak potential to refer to. choose the closest
					best_peak_index = find_closest_to_last_peak();
				else
					best_peak_index = find_lowest_noise(); //if there's only one peak this is the default setup
			}

			//if((all_ph_peaks_trusted_index < 0) && use_stored_peaks) //make this the trusted peak
			//	create_trusted_peak(best_peak_index);
		}

		if(ph_scan)
		{
/*			if((all_ph_peaks_trusted_index >= 0) && use_stored_peaks)
				best_peak_index = match_with_trusted_peak();
*/
			if(best_peak_index < 0) //didn't find a good match, or we didn't use the established peaks
			{
				if(first_good_ph_data_found) //so we have a good peak potential to refer to. choose the closest
					best_peak_index = find_closest_to_last_peak();
				else if(peaks_found == 2) //choose the leftmost peak, as seen in pH 4
					best_peak_index = 0;
				else
					best_peak_index = find_highest_qaqc(); //if there's only one peak this is the default setup
/*
				if((all_ph_peaks_trusted_index < 0) && use_stored_peaks && first_good_ph_data_found) //we have a new found peak
					create_trusted_peak(best_peak_index);*/
			}

//			if((all_ph_peaks_trusted_index < 0) && use_stored_peaks && first_good_ph_data_found) //make this the trusted peak,
//				create_trusted_peak(best_peak_index);
		}

/*		if(best_peak_index >= 0)
		{
			if(use_stored_peaks)
				update_peaks(best_peak_index);
		}
		else
			peaks_found = 0; //nothing met criteria*/
	}

	if(peaks_found == 0) //at least provide some information to the user
	{
		pp.peak_index = 0;
		pp.baseline_1_index = 0;
		pp.baseline_2_index = size_of_array - 1;

		pp.peak_potential = dac_output_voltage_data_global[pp.peak_index];
		pp.baseline_1_potential = dac_output_voltage_data_global[pp.baseline_1_index];
		pp.baseline_2_potential = dac_output_voltage_data_global[pp.baseline_2_index];

		pp.peak_value =  input_array[pp.peak_index];
		pp.baseline_1_value = input_array[pp.baseline_1_index];
		pp.baseline_2_value = input_array[pp.baseline_2_index];

		first_peak_potential = 0;
	}
	else
	{
		pp.peak_value = pp.peaks[best_peak_index][3];
		pp.peak_potential = dac_output_voltage_data_global[(int)pp.peaks[best_peak_index][0]];
		pp.QAQC1 = pp.peaks[best_peak_index][4];
		pp.QAQC2 = pp.peaks[best_peak_index][5];

		first_peak_potential = (uint32_t)(abs(dac_output_voltage_data_global[(int)pp.peaks[best_peak_index][8]]));
	}

	*peak_potential = pp.peak_potential;
	*qaqc = pp.QAQC2;
	*value_at_peak = pp.peak_value;

	return first_peak_potential;
}

void clean_all_ph_peaks(bool reset)
{
	//first, get rid of peaks that only got found a couple of times
	for(int i = 0; i < MAX_PEAKS; i++)
	{
		if(reset || ((all_ph_peaks[i][3] > 0) && (all_ph_peaks[i][3] < 3))) //this doesn't look worth remembering, reset
		{
			all_ph_peaks[i][0] = -1000; //peak
			all_ph_peaks[i][1] = 0.0; //qaqc
			all_ph_peaks[i][2] = 0.0; //trust
			all_ph_peaks[i][3] = 0.0; //count
			all_ph_peaks_count--;
			if(all_ph_peaks_count < 0)
				all_ph_peaks_count = 0;
		}
	}

	if(reset)
	{
		all_ph_peaks_count = 0;
		all_ph_peaks_trusted_index = -1;
		return;
	}

	//we are left with the trusted peak and others which occur frequently enough to save. close up the empty array entries
	for(int i = 0; i < MAX_PEAKS; i++)
	{
		if(all_ph_peaks[i][3] == 0) //empty_slot found, move next data here
		{
			for(int j = i+1; j < MAX_PEAKS; j++)
			{
				if(all_ph_peaks[j][3] != 0) //found non-zero data, move it into the empty slot
				{
					for(int k = 0; k < 4; k++)
					{
						all_ph_peaks[i][k] = all_ph_peaks[j][k];
						all_ph_peaks[j][k] = 0;
						if(k == 0)
							all_ph_peaks[j][k] = -1000;
					}
					break; //now that's done, go back into main loop
				}
			}
		}
	}

	//finally, make sure the trusted peak is the first entry
	for(int i = 1; i < MAX_PEAKS; i++) //if trusted peak is already in first slot, no need to check it
	{
		if((all_ph_peaks[i][2]) > 0) //trusted peak
		{
			//place first entry in the last array slot for safe keeping (probably unoccupied, if not, we've found a lot of false peaks
			for(int j = 0; j < 4; j++)
			{
				all_ph_peaks[MAX_PEAKS - 1][j] = all_ph_peaks[0][j];
				all_ph_peaks[0][j] = all_ph_peaks[i][j]; //now put trusted into zero
				all_ph_peaks[i][j] = all_ph_peaks[MAX_PEAKS - 1][j]; //put last into where trusted was
				all_ph_peaks[MAX_PEAKS - 1][j] = 0;
				if(j == 0)
					all_ph_peaks[MAX_PEAKS - 1][j] = -1000;
			}
		}
	}
}

int match_with_trusted_peak(void)
{
	int best_peak_index = -1;

	for(int i = 0; i < MAX_PEAKS; i++)
		if(pp.peaks[i][7] == 1) //array entry contains peak data
			for(int j = 0; j < MAX_PEAKS; j++)
				if(fabs(pp.peaks[i][9] - all_ph_peaks[j][0]) < peak_id_difference)
					if(all_ph_peaks[j][2] == 1) //this is the trusted peak
						best_peak_index = i;

	return best_peak_index;
}

void create_trusted_peak(int best_peak_index)
{
	for(int i = 0; i < MAX_PEAKS; i++)
	{
		if(pp.peaks[i][7] == 1) //array entry contains peak data
		{
			for(int j = 0; j < MAX_PEAKS; j++)
				if(fabs(pp.peaks[i][9] - all_ph_peaks[j][0]) < peak_id_difference)
				{
					all_ph_peaks[j][2] = 1; //this is now the trusted peak
					all_ph_peaks_trusted_index = j;
				}
		}
	}
}

int find_highest_qaqc(void)
{
	float max_so_far = -1;
	int best_peak_index = -1;

	for(int i = 0; i < peaks_found; i++)
	{
		float qaqc_product = pp.peaks[i][4] * pp.peaks[i][5];
		if(qaqc_product > max_so_far)
		{
			best_peak_index = i;
			max_so_far = qaqc_product;
		}
	}

	return best_peak_index;
}

int find_closest_to_last_peak(void)
{
	float min_so_far = 32767;
	int best_peak_index = -1;

	for(int i = 0; i < peaks_found; i++)
	{
		float distance_from_old_peak = abs(pp.peaks[i][9] - last_good_peak_potential);
		if(distance_from_old_peak < min_so_far)
		{
			best_peak_index = i;
			min_so_far = distance_from_old_peak;
		}
	}

	return best_peak_index;
}

int find_lowest_noise(void)
{
	float min_so_far = 32767;
	int best_peak_index = -1;

	for(int i = 0; i < peaks_found; i++)
	{
		float noise_level = pp.peaks[i][6];

		if(noise_level < min_so_far)
		{
			best_peak_index = i;
			min_so_far = noise_level;
		}
	}

	return best_peak_index;
}

void update_peaks(int best_peak_index)
{
	for(int i = 0; i < MAX_PEAKS; i++)
	{
		if(pp.peaks[i][7] == 1) //array entry contains peak data
			for(int j = 0; j < MAX_PEAKS; j++)
				if(fabs(pp.peaks[i][9] - all_ph_peaks[j][0]) < peak_id_difference) //got a matching entry
				{
					all_ph_peaks[j][3] = all_ph_peaks[j][3] + 1; //how many times it has been found
				}
	}
}

bool blacklisted_peak(double peak_potential)
{
	for(int i = 0; i < MAX_PEAKS; i++)
		for(int j = 0; j < MAX_PEAKS; j++)
			if(fabs(peak_potential - all_ph_peaks[j][0]) < peak_id_difference) //got a matching entry
				if(all_ph_peaks[j][2] == -1) //this is a rogue peak
					return true;

	return false;
}
