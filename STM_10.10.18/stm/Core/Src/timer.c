#include "timer.h"
#include "math.h"

volatile scantimer_struct scan_timer_global;

static spi_device_t  dac;

volatile bool adc_space_data_greater_than_adc_mark_data;
int8_t iref_failures = 0, secondary_iref_failures = 0;
bool no_good_iref_scans = true, trust_iref = false, using_previous_iref = false, suspect_iref = false;
int cond_ref_hold_time_code = 10;

void scan_init()
{
	scan_settings_reset_flags();

	if(fast_profiling && ocean_scan)
		scan_timer_global.conditioning_loop_count = 3;
	else
		scan_timer_global.conditioning_loop_count = 5;
	scan_timer_global.conditioning_timer_count = 62500;
	scan_timer_global.conditioning_loop_count_current_value = 0;
	
	scan_timer_global.scan_cycle_timer_count = 0;
	scan_timer_global.scan_positive_dac_timer_count = 0;
	scan_timer_global.scan_negative_dac_timer_count = 0;
	scan_timer_global.scan_negative_dac_timer_init_count = 0;

	scan_timer_global.scan_positive_adc_timer_count = 0;
	scan_timer_global.scan_negative_adc_timer_count = 0;
	scan_timer_global.scan_positive_adc_timer_init_count = 0;
	scan_timer_global.scan_negative_adc_timer_init_count = 0;
	
	// following values default to iREF value, as usually measurement cycle start with this
	//		anyway would be set by task table values obtained dynamically
	scan_timer_global.dac_scan_start_mv = IREF_OCEAN_START_MV_DEFAULT;
	scan_timer_global.dac_scan_current_mv = IREF_OCEAN_START_MV_DEFAULT;
	scan_timer_global.dac_scan_span_mv = IREF_OCEAN_SPAN_MV_INITIAL;
	scan_timer_global.dac_last_scan_mv = IREF_OCEAN_START_MV_DEFAULT - IREF_OCEAN_SPAN_MV_INITIAL;
	scan_timer_global.dac_scan_pulse_amplitude = IREF_OCEAN_PULSE_AMPLITUDE_MV_DEFAULT;
	scan_timer_global.dac_scan_half_pulse_amplitude = IREF_OCEAN_PULSE_AMPLITUDE_MV_DEFAULT / 2;
	scan_timer_global.dac_step_size = IREF_OCEAN_STEPSIZE_MV_INITIAL;
	
	scan_timer_global.dac_a_non_inverting_positive_current_value = 0;
	scan_timer_global.dac_b_inverting_positive_current_value = 0;
	scan_timer_global.dac_a_non_inverting_negative_current_value = 0;
	scan_timer_global.dac_b_inverting_negative_current_value = 0;
	
	scan_timer_global.dac_a_non_inverting_current_code = 0;
	scan_timer_global.dac_b_inverting_current_code = 0;
	scan_timer_global.dac_a_non_inverting_previous_code = 0;
	scan_timer_global.dac_b_inverting_previous_code = 0;
	
	scan_timer_global.dac_a_non_inverting_first_value_set_flag = true;	// set to false after first value is generated and reset to true on finishing a scan cycle for a single sensor
	scan_timer_global.dac_b_inverting_first_value_set_flag = true;		// Set to false after first value is generated and reset to true on finishing a scan cycle for a single sensor

	scan_timer_global.reached_last_dac_setting_for_current_scan = false;
	scan_timer_global.processfurther = true;
	
	iref_failures = 0;
	secondary_iref_failures = 0;
	continue_measurement_flag = true;
	immersion_status_requested_flag = false;
	ph_electrodes_completed = 0;
	electrodes_scanned_in_this_loop = 0;
	iref_health_bad_count = eeprom_read_iref_health_bad_count();
	//iref_secondary_health_bad_count = eeprom_read_iref_secondary_health_bad_count();
	new_start_mV_set = false;
	emergency_iref_peak_found = false;
	first_good_ph_data_found = false;
	first_good_iref_data_found = false;
	iref_clipped = false;
	ph_clipped = false;
	using_previous_iref = false;
	trust_iref = false;
	suspect_iref = false;
	
	scan_timer_global.adc_mark_data = NULL;
	scan_timer_global.adc_space_data = NULL;
	scan_timer_global.adc_max_data_count = 0;
	scan_timer_global.adc_current_data_count = 0;
	
	disable_afe_all_gain_resistors();
	
	dac8562_init(&dac, dac_chip_select);

	adc_utility_init();
	
	disable_sensor_electrodes();
}

void scan_settings_reset_flags()
{
	scan_timer_global.first_scan_dac_flag = true;			// after every first scan in a measurement scan cycle this is set to false, used to start the DAC negative cycle

	scan_timer_global.first_scan_positive_adc_flag = true;	// after every first scan in a measurement scan cycle this is set to false, used to start the ADC positive cycle
	scan_timer_global.first_scan_negative_adc_flag = true;	// after every first scan in a measurement scan cycle this is set to false, used to start the ADC negative cycle
	
	scan_timer_global.conditioning_timer_initialised = false;	// set to true after initialisation, as used to check if the timers are setup before starting them in the timer start function
	scan_timer_global.scan_timer_initialised = false;			// set to true after initialisation, as used to check if the timers are setup before starting them in the timer start function

	scan_timer_global.still_scanning = false;
	scan_timer_global.positive_mark_of_scan = true;
}

void dac_positive_scan_cycle_processor(void)
{
	if(!continue_measurement_flag)
		measurement_timer_stop();
	calculate_dac_settings();
	measurement_set_dac();
}

void dac_negative_scan_cycle_processor(void)
{
	if (scan_timer_global.first_scan_dac_flag == true)
	{
		// disable interrupts before changing timer settings
		cpu_irq_disable();
		
		// clock off to stop counting
		stop_timer(SCAN_DAC_NEGATIVE_TIMER);

		// set the timer to the same value as the period for a mark and space as used in the positive cycle
		// can be updated anytime per spec sheet, has higher priority than other operations within the timer
		set_timer_period(SCAN_DAC_NEGATIVE_TIMER, scan_timer_global.scan_negative_dac_timer_count);
		
		// set the count to 0, so it starts from known correct state when started again
		set_timer_count(SCAN_DAC_NEGATIVE_TIMER, 0);
		
		// enable interrupts after changing timer settings
		cpu_irq_enable();
		
		// set the first scan flag to false
		scan_timer_global.first_scan_dac_flag = false;

		start_timer(SCAN_DAC_NEGATIVE_TIMER);
	}
	
	calculate_dac_settings();
	
	measurement_set_dac();

}

void adc_positive_scan_cycle_processor(void)
{
	if (scan_timer_global.first_scan_positive_adc_flag == true)
	{
		// disable interrupts before changing timer settings
		cpu_irq_disable();
		
		// clock off to stop counting
		stop_timer(SCAN_ADC_MEASUREMENT_POSITIVE_TIMER);

		// set the timer to the same value as the period for a mark and space as used in the positive cycle
		// can be updated anytime per spec sheet, has higher priority than other operations within the timer
		set_timer_period(SCAN_ADC_MEASUREMENT_POSITIVE_TIMER, scan_timer_global.scan_positive_adc_timer_count);
		
		// set the count to 0, so it starts from known correct state when started again
		set_timer_count(SCAN_ADC_MEASUREMENT_POSITIVE_TIMER, 0);
		
		// enable interrupts after changing timer settings
		cpu_irq_enable();
		
		// set the first scan flag to false
		scan_timer_global.first_scan_positive_adc_flag = false;

		start_timer(SCAN_ADC_MEASUREMENT_POSITIVE_TIMER);
	}
	
	// call ADC to get measurement voltage for the positive cycle
	scan_timer_global.adc_current_value = adc_conversion()>>8;
	*(scan_timer_global.adc_mark_data + scan_timer_global.adc_current_data_count) = scan_timer_global.adc_current_value;
	
}

void adc_negative_scan_cycle_processor(void)
{
	if (scan_timer_global.first_scan_negative_adc_flag == true)
	{
		// disable interrupts before changing timer settings
		cpu_irq_disable();
		
		// clock off to stop counting
		stop_timer(SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER);

		// set the timer to the same value as the period for a mark and space as used in the positive cycle
		// can be updated anytime per spec sheet, has higher priority than other operations within the timer
		set_timer_period(SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER, scan_timer_global.scan_negative_adc_timer_count);
		
		// set the count to 0, so it starts from known correct state when started again
		set_timer_count(SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER, 0);
		
		// enable interrupts after changing timer settings
		cpu_irq_enable();
		
		// set the first scan flag to false
		scan_timer_global.first_scan_negative_adc_flag = false;

		start_timer(SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER);
	}
	
	// call ADC to get measurement voltage for the negative cycle
	scan_timer_global.adc_current_value = adc_conversion()>>8;
	// place the value in the global array for Rahul's smoothing function to use
	*(scan_timer_global.adc_space_data + scan_timer_global.adc_current_data_count) = scan_timer_global.adc_current_value ;

	// calculate the difference of the adc space - mark values for this scan dc voltage
	*(scan_timer_global.adc_mark_space_array_1_data + scan_timer_global.adc_current_data_count) = abs(scan_timer_global.adc_current_value - *(scan_timer_global.adc_mark_data + scan_timer_global.adc_current_data_count));

	scan_timer_global.adc_current_data_count = scan_timer_global.adc_current_data_count + 1;

}

void conditioning_timer_init(void)
{
	// disable interrupts before changing timer settings
	cpu_irq_disable();

	// set the scan step for positive part of a scan cycle
	// Enable timer
	start_timer(SCAN_DAC_POSITIVE_CONDITIONING_TIMER);
	
	//enable interrupts after changing timer settings
	cpu_irq_enable();

	scan_timer_global.conditioning_timer_initialised = true;
	scan_timer_global.conditioning_loop_count_current_value = 0;
}


void timer_init(void)
{
	// disable interrupts before changing timer settings
	cpu_irq_disable();
	
	start_timer(SCAN_DAC_POSITIVE_TIMER);
	start_timer(SCAN_DAC_NEGATIVE_TIMER);
	start_timer(SCAN_ADC_MEASUREMENT_POSITIVE_TIMER);
	start_timer(SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER);
	
	// enable interrupts after changing timer settings
	cpu_irq_enable();

	scan_timer_global.scan_timer_initialised = true;
}

void timer_calculate_counts(double frequency)
{
	// calculate the count from the duration
	double temp = (32.0 * 1000 * 1000) / 256;								// 32MHz system clock / 256 timer module prescaler
	double positive_duration_count = 0.5 * temp / frequency;	// duration * 32MHz system clock / 256 timer module prescaler
	double negative_duration_count = 0.5 * temp / frequency;	// duration * 32MHz system clock / 256 timer module prescaler
	
	// adjust the count for the variation seen in Brandon's scope, assumption the scope is accurate
	positive_duration_count = (int) positive_duration_count; // * percentage_diff;
	negative_duration_count = (int) negative_duration_count; // * percentage_diff;
	
	// get the total timer count for a complete measurement cycle
	scan_timer_global.scan_cycle_timer_count = positive_duration_count + negative_duration_count;
	// set the dac timer count for overflow to trigger the positive and negative part inversion
	scan_timer_global.scan_positive_dac_timer_count = scan_timer_global.scan_cycle_timer_count;
	scan_timer_global.scan_negative_dac_timer_count = scan_timer_global.scan_cycle_timer_count;
	// set the initial dac timer for the negative part is only started after the first positive part is finished
	scan_timer_global.scan_negative_dac_timer_init_count = positive_duration_count;
	// set the adc timer count for overflow to trigger the positive and negative part measurements
	scan_timer_global.scan_positive_adc_timer_count = scan_timer_global.scan_cycle_timer_count;
	scan_timer_global.scan_negative_adc_timer_count = scan_timer_global.scan_cycle_timer_count;
	// set the adc initial values to start the timer with relation to the total period of single scan cycle
	if(iref_scan && ocean_scan)
	{
		scan_timer_global.scan_positive_adc_timer_init_count = (uint16_t) positive_duration_count * SCAN_ADC_MEASUREMENT_RATIO_POSITIVE_DUTY_CYCLE_LOW;
		scan_timer_global.scan_negative_adc_timer_init_count = (uint16_t) (negative_duration_count * SCAN_ADC_MEASUREMENT_RATIO_NEGATIVE_DUTY_CYCLE_LOW) + positive_duration_count;
	}
	else
	{
		scan_timer_global.scan_positive_adc_timer_init_count = (uint16_t) positive_duration_count * SCAN_ADC_MEASUREMENT_RATIO_POSITIVE_DUTY_CYCLE_HIGH;
		scan_timer_global.scan_negative_adc_timer_init_count = (uint16_t) (negative_duration_count * SCAN_ADC_MEASUREMENT_RATIO_NEGATIVE_DUTY_CYCLE_HIGH) + positive_duration_count;
	}
}

void conditioning_timer_loop(void)
{
	// increment counter
	scan_timer_global.conditioning_loop_count_current_value = scan_timer_global.conditioning_loop_count_current_value + 1;

	if ((scan_timer_global.conditioning_loop_count_current_value >= scan_timer_global.conditioning_loop_count) || !continue_measurement_flag)
	{
		// if its looped the required number of times to achieve the conditioning time
		//		stop conditioning timer, and call the measurement / scan loop
		
		cpu_irq_disable();

		stop_timer(SCAN_DAC_POSITIVE_CONDITIONING_TIMER);
		
		// enable interrupts after changing timer settings
		cpu_irq_enable();

		scan_timer_global.conditioning_timer_initialised = false;
		
		// call measurement start
		measurement_timer_start();
	}
}

void conditioning_timer_start(void)
{
	// set the DAC outputs to zero
	scan_timer_global.dac_a_non_inverting_current_code = 0;
	scan_timer_global.dac_b_inverting_current_code = 0;
	measurement_set_dac();
	
	// switch on the DAC output to the counter electrode
	enable_dac_output_to_counter_electrode();

	if(current_task.multielectrode_scan)
		enable_multielectrode_scan(current_task.last_scan);
	else
	{
		if(ph_scan)
		{
			enable_single_electrode(current_task.sensor_number, true);
			if(current_task.second_sensor_number != 0)
				enable_single_electrode(current_task.second_sensor_number, false);
			if(current_task.third_sensor_number != 0)
				enable_single_electrode(current_task.third_sensor_number, false);
		}
		else if(iref_scan)
		{
			if(!use_single_iref)
			{
				enable_single_electrode(IREF_PRIMARY_SENSOR_NUMBER, true);
				enable_single_electrode(IREF_SECONDARY_SENSOR_NUMBER, false);
			}
			else
				enable_single_electrode(current_task.sensor_number, true);
		}
	}

	// set dac output
	if(iref_scan)
	{
		// for an iref, the inverting input is changed while the non inverting input stays at the start value
		
		// new DAC A (non inverting) value is set to the current (start mv) dc value for the first setting
		scan_timer_global.dac_a_non_inverting_positive_current_value = scan_timer_global.dac_scan_current_mv;
		scan_timer_global.dac_a_non_inverting_current_code = (uint16_t) (scan_timer_global.dac_a_non_inverting_positive_current_value * MV_TO_STEPS_1MV);
		// new DAC B (inverting) value starts from zero
		scan_timer_global.dac_b_inverting_positive_current_value = 0;
		scan_timer_global.dac_b_inverting_current_code = 0;
	}
	else if(ph_scan)
	{
		// for a pH electrode, the non inverting input is changed while the inverting input stays at the start value
		
		// new DAC A (non inverting) value starts from zero
		scan_timer_global.dac_a_non_inverting_positive_current_value = 0;
		scan_timer_global.dac_a_non_inverting_current_code = 0;
		// new DAC B (inverting) value is set to the current (start mv) dc value for the first setting
		scan_timer_global.dac_b_inverting_positive_current_value = scan_timer_global.dac_scan_current_mv;
		scan_timer_global.dac_b_inverting_current_code = (uint16_t) (scan_timer_global.dac_b_inverting_positive_current_value * MV_TO_STEPS_1MV);
	}
	
	measurement_set_dac();

	// call function to turn on any optional gain resistors, if needed
	switch(current_task.gain_resistor_setting)
	{
		case 0:
		disable_afe_all_gain_resistors();
		break;
		
		case 1:
		disable_afe_all_gain_resistors();
		enable_afe_2nd_gain_resistor();
		break;
		
		case 2:
		disable_afe_all_gain_resistors();
		enable_afe_3rd_gain_resistor();
		break;
		
		case 3:
		disable_afe_all_gain_resistors();
		enable_afe_2nd_gain_resistor();
		enable_afe_3rd_gain_resistor();
		break;
		
		case 4:
		disable_afe_all_gain_resistors();
		enable_afe_4th_gain_resistor();
		break;
		
		case 5:
		disable_afe_all_gain_resistors();
		enable_afe_3rd_gain_resistor();
		enable_afe_4th_gain_resistor();
		break;
		
		case 6:
		disable_afe_all_gain_resistors();
		enable_afe_2nd_gain_resistor();
		enable_afe_5th_gain_resistor();
		break;
		
		case 7:
		disable_afe_all_gain_resistors();
		enable_afe_2nd_gain_resistor();
		enable_afe_3rd_gain_resistor();
		enable_afe_5th_gain_resistor();
		break;
		
		case 8:
		disable_afe_all_gain_resistors();
		enable_afe_2nd_gain_resistor();
		enable_afe_4th_gain_resistor();
		enable_afe_5th_gain_resistor();
		break;
		
		case 9:
		enable_afe_all_gain_resistors();
		break;
		
		
		default:
		break;
	}
	
	if(scan_timer_global.conditioning_timer_initialised == false)
	{
		// initialises the timers used for measurement before starting it!!
		conditioning_timer_init();
	}

	// disable interrupts before changing timer settings
	cpu_irq_disable();
	
	// Set the timer period, when the overflow interrupt for the timer would be triggered
	set_timer_period(SCAN_DAC_POSITIVE_CONDITIONING_TIMER, scan_timer_global.conditioning_timer_count);
	
	// set the count to 0, so it starts from known correct state when started again
	set_timer_count(SCAN_DAC_POSITIVE_CONDITIONING_TIMER, 0);
	
	// enable interrupts after changing timer settings
	cpu_irq_enable();
	
	scan_timer_global.still_scanning = true;
}

void measurement_timer_start(void)
{
	if(scan_timer_global.scan_timer_initialised == false)
	{
		// initialises the timers used for measurement before starting it!!
		timer_init();
	}

	// disable interrupts before changing timer settings
	cpu_irq_disable();
	
	// Set the timer period, when the overflow interrupt for the timer would be triggered
	set_timer_period(SCAN_DAC_POSITIVE_TIMER, scan_timer_global.scan_positive_dac_timer_count);

	// Set the timer period, when the overflow interrupt for the timer would be triggered
	set_timer_period(SCAN_DAC_NEGATIVE_TIMER, scan_timer_global.scan_negative_dac_timer_init_count);

	// Set the timer period, when the overflow interrupt for the timer would be triggered
	set_timer_period(SCAN_ADC_MEASUREMENT_POSITIVE_TIMER, scan_timer_global.scan_positive_adc_timer_init_count);

	// Set the timer period, when the overflow interrupt for the timer would be triggered
	set_timer_period(SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER, scan_timer_global.scan_negative_adc_timer_init_count);

	
	// set the count to 0, so it starts from known correct state when started again
	set_timer_count(SCAN_DAC_POSITIVE_TIMER, 0);
	set_timer_count(SCAN_DAC_NEGATIVE_TIMER, 0);
	set_timer_count(SCAN_ADC_MEASUREMENT_POSITIVE_TIMER, 0);
	set_timer_count(SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER, 0);
	
	// enable interrupts after changing timer settings
	cpu_irq_enable();
}

void measurement_timer_stop(void)
{
	// disable interrupts before changing timer settings
	cpu_irq_disable();
	
	// set the count to 0, so it starts from known correct state when started again
	set_timer_count(SCAN_DAC_POSITIVE_CONDITIONING_TIMER, 0);
	set_timer_count(SCAN_DAC_POSITIVE_TIMER, 0);
	set_timer_count(SCAN_DAC_NEGATIVE_TIMER, 0);
	set_timer_count(SCAN_ADC_MEASUREMENT_POSITIVE_TIMER, 0);
	set_timer_count(SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER, 0);
	
	// disable the timers after stopping to conserve power
	stop_timer(SCAN_DAC_POSITIVE_CONDITIONING_TIMER);
	stop_timer(SCAN_DAC_POSITIVE_TIMER);
	stop_timer(SCAN_DAC_NEGATIVE_TIMER);
	stop_timer(SCAN_ADC_MEASUREMENT_POSITIVE_TIMER);
	stop_timer(SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER);
	
	// enable interrupts after changing timer settings
	cpu_irq_enable();

	// reset scan timer flags
	scan_settings_reset_flags();
	
	disable_afe_all_gain_resistors();
	
	// set the DAC outputs to zero
	scan_timer_global.dac_a_non_inverting_current_code = 0;
	scan_timer_global.dac_b_inverting_current_code = 0;
	measurement_set_dac();
}

int scanning_in_progress()
{
	return scan_timer_global.still_scanning;
}

void measurement_dac_settings()
{
	scan_timer_global.dac_scan_start_mv = current_task.start_mv;
	scan_timer_global.dac_scan_span_mv = current_task.span_mv;
	scan_timer_global.dac_scan_pulse_amplitude = current_task.pulse_amplitude;
	scan_timer_global.dac_step_size = current_task.step_size;
	scan_timer_global.dac_last_scan_mv = scan_timer_global.dac_scan_start_mv - scan_timer_global.dac_scan_span_mv;
	scan_timer_global.dac_scan_current_mv = scan_timer_global.dac_scan_start_mv + scan_timer_global.dac_step_size;
	scan_timer_global.dac_scan_half_pulse_amplitude = (scan_timer_global.dac_scan_pulse_amplitude) / 2;
}

void calculate_dac_settings(void)
{
	if (scan_timer_global.positive_mark_of_scan)
	{
		// set to false, so next call to this function would handle the negative / space of a scan cycle
		scan_timer_global.positive_mark_of_scan = false;
		
		// calculate the new dc value, only during the positive cycle
		//		by subtracting the scan step size at the start of each
		//		positive / mark of a measurement
		scan_timer_global.dac_scan_current_mv = scan_timer_global.dac_scan_current_mv - scan_timer_global.dac_step_size;
		// store the new dc value in the array used by the smoothing function
		*(scan_timer_global.dac_output_voltage_data + scan_timer_global.adc_current_data_count) = scan_timer_global.dac_scan_current_mv;
		
		// if the current value is equal or smaller than the last / limit
		//		value needed, set the flag to stop scan for this sensor
		//		/ electrode
		if (scan_timer_global.dac_scan_current_mv <= scan_timer_global.dac_last_scan_mv)
		{
			scan_timer_global.reached_last_dac_setting_for_current_scan = true;
			// if its smaller, stop the next cycle or measurement immediately
			//		for this sensor or electrode
			if (scan_timer_global.dac_scan_current_mv < scan_timer_global.dac_last_scan_mv)
			{
				// current voltage setting is beyond the range, so stop measurements immediately
				measurement_timer_stop();
				scan_timer_global.processfurther = false;
				if(ph_scan && current_task.last_scan)
				{
					scan_timer_global.dac_a_non_inverting_positive_current_value = 0;
					scan_timer_global.dac_a_non_inverting_current_code = 0;
					measurement_set_dac();
					if(fast_profiling && ocean_scan)
						HAL_Delay(500);
					else
						HAL_Delay(2500);
				}
				HAL_Delay(5); //have found that a delay between scan, set dac to zero, and disable electrode eliminates 300kHz oscillation we see SG 160724
				disable_sensor_electrodes();
			}
			else
			{
				// its reached the last cycle, so let this function generate the last dac setting
				scan_timer_global.processfurther = true;
			}
		}
		else
		{
			// still havent reached the last scan, so continue with dac calculations
			scan_timer_global.processfurther = true;
		}
		
		// check scan type, so the correct inverting / non inverting dac could be modified as needed
		if (scan_timer_global.processfurther && iref_scan)
		{
			// if its iref the inverting input is changed while the non inverting input stays at the start value
			if (scan_timer_global.dac_scan_current_mv == scan_timer_global.dac_scan_start_mv)
			{
				// the first setting flag is set to false after the negative / space part of the cycle! so leave it alone here
				// new DAC A (non inverting) value is set to the current (start mv) dc value + half the amplitude of the pulse
				//		for the first setting in a scan for a sensor / electrode
				scan_timer_global.dac_a_non_inverting_positive_current_value = scan_timer_global.dac_scan_current_mv + scan_timer_global.dac_scan_half_pulse_amplitude;
				// the code is calculated by multiplying the step size for each mV as measured by Brandon
				scan_timer_global.dac_a_non_inverting_current_code = (uint16_t) (scan_timer_global.dac_a_non_inverting_positive_current_value * MV_TO_STEPS_1MV);
				// new DAC B (inverting) value starts from zero
				scan_timer_global.dac_b_inverting_positive_current_value = 0;
				scan_timer_global.dac_b_inverting_current_code = 0;
			}
			else
			{
				// new DAC B (inverting) value is set to the current value + step size for each measurement (except the first)
				scan_timer_global.dac_b_inverting_positive_current_value = scan_timer_global.dac_b_inverting_positive_current_value + scan_timer_global.dac_step_size;
				// the code is calculated by multiplying the step size for each mV as measured by Brandon
				scan_timer_global.dac_b_inverting_current_code = (uint16_t) (scan_timer_global.dac_b_inverting_positive_current_value * MV_TO_STEPS_1MV);
			}
		}
		if (scan_timer_global.processfurther && ph_scan)
		{
			// if its pH sensor / electrode, the non inverting input is changed while the inverting input stays at the start value
			if (scan_timer_global.dac_scan_current_mv == scan_timer_global.dac_scan_start_mv)
			{
				// the first setting flag is set to false after the negative / space part of the cycle! so leave it alone here
				// new DAC A (non inverting) value starts from zero
				scan_timer_global.dac_a_non_inverting_positive_current_value = 0;
				scan_timer_global.dac_a_non_inverting_current_code = 0;
				// new DAC B (inverting) value is set to the current (start mv) dc value + half the amplitude of the pulse
				//		for the first setting in a scan for a sensor / electrode
				scan_timer_global.dac_b_inverting_positive_current_value = scan_timer_global.dac_scan_current_mv + scan_timer_global.dac_scan_half_pulse_amplitude;
				// the code is calculated by multiplying the step size for each mV as measured by Brandon
				scan_timer_global.dac_b_inverting_current_code = (uint16_t) (scan_timer_global.dac_b_inverting_positive_current_value * MV_TO_STEPS_1MV);
			}
			else
			{
				// new DAC A (non inverting) value is set to the current value + step size for each measurement (except the first)
				scan_timer_global.dac_a_non_inverting_positive_current_value = scan_timer_global.dac_a_non_inverting_positive_current_value + scan_timer_global.dac_step_size;
				// the code is calculated by multiplying the step size for each mV as measured by Brandon
				scan_timer_global.dac_a_non_inverting_current_code = (uint16_t) (scan_timer_global.dac_a_non_inverting_positive_current_value * MV_TO_STEPS_1MV);
			}
		}
	}
	else
	{
		// set to true, so next call to this function would handle the positive / mark of a scan cycle
		scan_timer_global.positive_mark_of_scan = true;
		
		if(iref_scan)
		{
			// if its iREF sensor / electrode, the inverting input is changed while the non inverting input stays at the
			//		start value as specified in the task table
			if (scan_timer_global.dac_scan_current_mv == scan_timer_global.dac_scan_start_mv)
			{
				// set the first setting flag to false
				scan_timer_global.dac_a_non_inverting_first_value_set_flag = 0;
				// new DAC B (inverting) value is set to the scan pulse amplitude
				//		for the first setting in a scan for a sensor / electrode in the negative / space part of a cycle
				scan_timer_global.dac_b_inverting_negative_current_value = scan_timer_global.dac_scan_pulse_amplitude;
				// the code is calculated by multiplying the step size for each mV as measured by Brandon
				scan_timer_global.dac_b_inverting_current_code = (uint16_t) (scan_timer_global.dac_b_inverting_negative_current_value * MV_TO_STEPS_1MV);
			}
			else
			{
				// new DAC B (inverting) value is set to the current value + step size for each measurement (except the first)
				scan_timer_global.dac_b_inverting_negative_current_value = scan_timer_global.dac_b_inverting_negative_current_value + scan_timer_global.dac_step_size;
				// the code is calculated by multiplying the step size for each mV as measured by Brandon
				scan_timer_global.dac_b_inverting_current_code = (uint16_t) (scan_timer_global.dac_b_inverting_negative_current_value * MV_TO_STEPS_1MV);
			}
		}
		else if(ph_scan)
		{
			// if its pH sensor / electrode, the non inverting input is changed while the inverting input stays at the
			//		start value as specified in the task table
			if (scan_timer_global.dac_scan_current_mv == scan_timer_global.dac_scan_start_mv)
			{
				// set the first setting flag to false
				scan_timer_global.dac_a_non_inverting_first_value_set_flag = 0;
				// new DAC A (non inverting) value is set to the current value + step size for each measurement (except the first)
				scan_timer_global.dac_a_non_inverting_negative_current_value = scan_timer_global.dac_scan_pulse_amplitude;
				// the code is calculated by multiplying the step size for each mV as measured by Brandon
				scan_timer_global.dac_a_non_inverting_current_code = (uint16_t) (scan_timer_global.dac_a_non_inverting_negative_current_value * MV_TO_STEPS_1MV);
			}
			else
			{
				// new DAC A (non inverting) value is set to the current value + step size for each measurement (except the first)
				scan_timer_global.dac_a_non_inverting_negative_current_value = scan_timer_global.dac_a_non_inverting_negative_current_value + scan_timer_global.dac_step_size;
				// the code is calculated by multiplying the step size for each mV as measured by Brandon
				scan_timer_global.dac_a_non_inverting_current_code = (uint16_t) (scan_timer_global.dac_a_non_inverting_negative_current_value * MV_TO_STEPS_1MV);
			}
		}
	}
}

void measurement_set_dac()
{
	// only set a DAC if the value has changed
	if (scan_timer_global.dac_a_non_inverting_previous_code != scan_timer_global.dac_a_non_inverting_current_code)
	{
		// since the calculated value has changed, update the previous value
		//		so that it could be used for the next check
		scan_timer_global.dac_a_non_inverting_previous_code = scan_timer_global.dac_a_non_inverting_current_code;
		// call DAC to set voltage for the scan
		DAC_SET_A(&dac, scan_timer_global.dac_a_non_inverting_current_code);
	}

	// only set a DAC if the value has changed
	if (scan_timer_global.dac_b_inverting_previous_code != scan_timer_global.dac_b_inverting_current_code)
	{
		// since the calculated value has changed, update the previous value
		//		so that it could be used for the next check
		scan_timer_global.dac_b_inverting_previous_code = scan_timer_global.dac_b_inverting_current_code;
		// call DAC to set voltage for the scan
		DAC_SET_B(&dac, scan_timer_global.dac_b_inverting_current_code);
	}
}

void set_adc_data_address(
		uint16_t * adc_mark_data,
		uint16_t * adc_space_data,
		float * adc_mark_space_array_1_data,
		float * dac_output_voltage_data,
		uint16_t adc_data_size)
{
	scan_timer_global.adc_mark_data = adc_mark_data;
	scan_timer_global.adc_space_data = adc_space_data;
	scan_timer_global.adc_mark_space_array_1_data = adc_mark_space_array_1_data;
	scan_timer_global.dac_output_voltage_data = dac_output_voltage_data;
	scan_timer_global.adc_max_data_count = adc_data_size;
	// reset it to zero
	scan_timer_global.adc_current_data_count = 0;
}

void condition_ref(void)
{
	uint32_t hold_time_ms = 0;

	if(ocean_scan)
	{
		if(perform_precondition_ph_electrodes_flag)
			hold_time_ms = 100;
		else
			hold_time_ms = 20;
	}
	else if(fresh_scan)
	{
		if(perform_precondition_ph_electrodes_flag)
			hold_time_ms = 1000;
		else
			hold_time_ms = 50;
	}
	else if(vf_scan)
	{
		if(perform_precondition_ph_electrodes_flag)
			hold_time_ms = 1000;
		else
			hold_time_ms = 100;
	}

	ioport_set_pin_level(power_ground_in3_ctr_to_ctr_drv, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(power_ground_in1_ref_to_a, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(power_ground_in4_ctr_to_f, IOPORT_PIN_LEVEL_LOW);

	HAL_Delay(hold_time_ms);

	ioport_set_pin_level(power_ground_in3_ctr_to_ctr_drv, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(power_ground_in1_ref_to_a, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(power_ground_in4_ctr_to_f, IOPORT_PIN_LEVEL_LOW);
}

void precondition_ph_electrodes(double peak_ph_potential)
{
	double preconditioning_ph_peak_plus = PRECONDITIONING_PH_PEAK_PLUS_CASE_1;

	if(precondition_ph_case == 0x01)
		preconditioning_ph_peak_plus = PRECONDITIONING_PH_PEAK_PLUS_CASE_1;
	else if(precondition_ph_case == 0x02)
		preconditioning_ph_peak_plus = PRECONDITIONING_PH_PEAK_PLUS_CASE_2;

	enable_ph_electrodes();

	uint16_t code_to_DAC_A = 0;
	uint16_t code_to_DAC_B = (preconditioning_ph_peak_plus + peak_ph_potential) * MV_TO_STEPS_1MV; //may want to amend this for fresh/ocean water

	DAC_SET_A(&dac, code_to_DAC_A);
	DAC_SET_B(&dac, code_to_DAC_B);

	HAL_Delay(PRECONDITIONING_TIME_S * 1000);

	DAC_SET_A(&dac, 0);
	DAC_SET_B(&dac, 0);

	disable_sensor_electrodes();

	if(remember_preconditioning_done)
	{
		preconditioning_done = true;
		eeprom_write_byte_with_readback_check(EEPROM_ADDR + DEVICE_PRECONDITIONING_DONE, preconditioning_done);
	}
}

uint8_t check_immersed_dry_status(void)
{
	int step_height_mv = 500, num_samples = 500; //change samples up to increase accuracy, but with a time penalty
	int32_t adc_result_off, adc_result_on, difference, threshold = 10000; //changed from 50000 211223 because of false negatives
	uint64_t sum = 0;
	uint16_t code_to_DAC_B = 0;
	uint16_t code_to_DAC_A = 0;
	uint8_t status = 2;

//	if(anb_or_alternate_head == COMMAND_OPTION_VALEPORT_HEAD)
//		threshold = 50000; //ATX board only. R30 is 180 Ohms on ATX, so gain is different from STM. adjust threshold for lower signal size

	if(!ocean_scan)
		threshold = 10000; //immersion testing was 1

	if(iref_scan)
	{
		code_to_DAC_A = step_height_mv * MV_TO_STEPS_1MV;
		code_to_DAC_B = 0;
	}
	else if(ph_scan)
	{
		code_to_DAC_A = 0;
		code_to_DAC_B = step_height_mv * MV_TO_STEPS_1MV;
	}

	enable_analog_power_all();
	adc_utility_init();
	dac8562_init(&dac, dac_chip_select);

	enable_water_detection();

	DAC_SET_A(&dac, 0);
	DAC_SET_B(&dac, 0);

	sum = 0;
	for(int h = 0; h < num_samples; h++)
		sum = sum + adc_convert_manipulate();

	adc_result_off = sum / num_samples;

	DAC_SET_A(&dac, code_to_DAC_A);
	DAC_SET_B(&dac, code_to_DAC_B);

	sum = 0;
	for(int h = 0; h < num_samples; h++)
		sum = sum + adc_convert_manipulate();
	adc_result_on = sum / num_samples;

	DAC_SET_A(&dac, 0);
	DAC_SET_B(&dac, 0);

	disable_water_detection();

	HAL_Delay(500);

	difference = abs(adc_result_on - adc_result_off);
	immersion_adc_difference = difference / 1000.0;

	if(difference > threshold)
	{
		status = 1;
		immersed_flag = true;
	}
	else
	{
		status = 2;
		immersed_flag = false;
	}

	return status;
}
