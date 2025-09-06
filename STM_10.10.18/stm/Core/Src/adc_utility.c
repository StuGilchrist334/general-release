#include "adc_utility.h"
#include "data_smoothing.h"
#include <stdlib.h>

static spi_device_t  adc;

#define NUMBER_OF_GAIN_SETTINGS 9

void adc_utility_init(void)
{
	ad7766_init(&adc, adc_chip_select, adc_data_ready, adc_sync_pd);
}

/**
* @brief ADC data manipulation.
*
* Invert the 24th bit to read all values from 0 / ground instead of positive or negative
* values with respect to reference voltage (currently 1.25 V)
*
* @return int32_t manipulated adc value
*
* @author Palaniappan Valliappan
* @version v1.0
* @date 202301201245 created - PV
*/
int32_t adc_convert_manipulate()
{
	int32_t temp_adc_reading = ad7766_convert(&adc);
	// toggle 24th bit
	temp_adc_reading = temp_adc_reading ^ 0x00800000;
	
	return temp_adc_reading;
}

/**
* @brief ADC data manipulation.
*
* ignore inversion for valeport

* @return int32_t manipulated adc value
*
* @author RR
* @version v1.0
* @date 202301201245 created - PV
*/
int32_t adc_convert_manipulate_vp()
{
	int32_t temp_adc_reading = ad7766_convert(&adc);

	// toggle 24th bit
	temp_adc_reading = temp_adc_reading ^ 0x00800000;
	if (temp_adc_reading <= 0x007fffff)
	{
	temp_adc_reading =  0x007fffff - temp_adc_reading;
	}
	else{}

	return temp_adc_reading;
}


int32_t adc_conversion(void)
{
	// Brandon wants to take average of 4 samples (instead of 1)
	//		to smooth out the reading from testing in water
	// Also, divide the final reading by 2 to avoid it going over (overflow)
	//		when converting from uint32 to uint16
	uint8_t number_of_adc_readings = 4;
	int32_t temp_adc_reading = 0;
	for (int i = 0; i < number_of_adc_readings; i++)
	{
		temp_adc_reading = temp_adc_reading + adc_convert_manipulate();
	}
	// calculate the average
	return (temp_adc_reading / (number_of_adc_readings * 2)); // divide by another 2 as instructed by Brandon on 202301171435 to avoid overflow as the readings were close to uint16 limits
}

int check_signal_size(int array_size, int current_gain, int sensor_number, bool act_on_clip, uint16_t* max_value)
{
	int new_gain = 1, calculated_gain = 1;

	uint16_t signal_size = 0, max_so_far = 0, min_so_far = 16383;
	double half_range = 16383.0;
	//this ratio array is normalised to gain 5
	double gain_ratios[NUMBER_OF_GAIN_SETTINGS] = {0.0726, 0.1522, 0.2303, 0.4800, 1.0000, 1.5761, 2.1574, 2.7829, 3.1843};

	for(int i = 5 ; i < array_size - 5; i++ )
	{
		signal_size = abs(adc_mark_data_global[i] - half_range);
		if(signal_size > max_so_far)
			max_so_far = signal_size;
		if(signal_size < min_so_far)
			min_so_far = signal_size;

		signal_size = abs(adc_space_data_global[i] - half_range);
		if(signal_size > max_so_far)
			max_so_far = signal_size;
		if(signal_size < min_so_far)
			min_so_far = signal_size;
	}

	*max_value = max_so_far;

	this_scan_clipped = false;

	if(max_so_far > (0.99 * half_range))
	{
		this_scan_clipped = true;
	}
	else
	{
		double predicted_size;
		double target_factor = 0.9; //to calculate the new gain without overshooting, use target_factor <= 1

		int max_gain = MAX_GAIN_SETTING;
		if(iref_scan)
		{
			max_gain = MAX_GAIN_IREF;
			good_iref_gain_found = true; //if we clip after this, something is wrong, as we've seen, so log our success
			good_iref_gain = current_gain; //store this good value
		}
		else if(ph_scan)
			max_gain = MAX_GAIN_PH;

		for(int i = max_gain; i > 0; i--)
		{
			predicted_size = max_so_far * gain_ratios[i - 1] / gain_ratios[current_gain - 1];
			if(predicted_size < (half_range * target_factor))
			{
				calculated_gain = i;
				break; //found a gain that won't overshoot
			}
		}

		//we have seen that predicted gains sometimes overshoot around gain 6 and signal size >6000
		if(ph_scan)
		{
			if((current_gain >= 6) && (calculated_gain > current_gain) && (max_so_far > 6000))
				calculated_gain = current_gain + 1; //don't go up too much
		}
	}

	if(this_scan_clipped)
	{
		if(iref_scan && iref_clipped && good_iref_gain_found) //if we clipped again despite having a supposed good known value..
		{
			new_gain = current_gain - 1;
			good_iref_gain = new_gain;
		}
		else if(current_gain >= 7)
			new_gain = 6;
		else if(current_gain >= 6)
			new_gain = 4;
		else if(current_gain >= 4)
			new_gain = 2;
		else
			new_gain = current_gain - 1;

		if(iref_scan)
		{
			iref_clipped = true;
			if(current_gain == 1)
				use_single_iref = true;
		}
		else if(ph_scan)
		{
			if(act_on_clip && (electrode_health_values[sensor_number - 1] <= 3))
			{
				ph_clipped = true;
				post_clipped_gain = new_gain;
				failure_diagnostic += new_gain;
			}
			if(!current_task.multielectrode_scan)
			{
				electrode_clipped_at_gain[sensor_number - 1] = current_gain;
				eeprom_save_clipped_at_gain();
			}
		}

		failure_diagnostic += 1000;
	}
	else
		new_gain = calculated_gain;

	if(iref_scan && iref_clipped && good_iref_gain_found) //funny business has been going on, check we're not going to get into trouble again
	{
		if(calculated_gain > good_iref_gain)
			new_gain = good_iref_gain; //this will override anything calculated above
	}

	if(ph_scan && !current_task.multielectrode_scan)
		if(new_gain >= electrode_clipped_at_gain[sensor_number - 1])
			new_gain = electrode_clipped_at_gain[sensor_number - 1] - 1;

	if (new_gain < 1)
		new_gain = 1;

	return new_gain;
}
