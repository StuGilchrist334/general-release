#ifndef TIMER_H_
#define TIMER_H_

#include "stdbool.h"
#include "tasktable_master.h"
#include "spi_device.h"
#include "dac8562.h"
#include "adc_utility.h"
#include "subsystem.h"

//limits removed. delete if definitely not needed
#define SCAN_TIMER_MINIMUM 0.002	// in seconds
									// calculation = 1 / (32 MHz system clock / 256 timer prescaler selection) * count (250 -> to get 500 Hz)
#define SCAN_TIMER_MAXIMUM 0.02		// in seconds
									// calculation = 1 / (32 MHz system clock / 256 timer prescaler selection) * count (2500 -> to get 50 Hz)

// ADC measurements currently set at 2/3 of the duration after DAC is set
// This code needs changing when ADC start changes are required
#define SCAN_ADC_MEASUREMENT_RATIO_VS_SCAN_CYCLE_HIGH 0.98	// ADC measurement done at fraction of the duration after setting the DAC
#define SCAN_ADC_MEASUREMENT_RATIO_VS_SCAN_CYCLE_LOW 0.98 //2/3
#define SCAN_ADC_MEASUREMENT_RATIO_POSITIVE_DUTY_CYCLE_HIGH SCAN_ADC_MEASUREMENT_RATIO_VS_SCAN_CYCLE_HIGH	// currently both positive and negative use same timing
#define SCAN_ADC_MEASUREMENT_RATIO_NEGATIVE_DUTY_CYCLE_HIGH SCAN_ADC_MEASUREMENT_RATIO_VS_SCAN_CYCLE_HIGH	// currently both positive and negative use same timing
#define SCAN_ADC_MEASUREMENT_RATIO_POSITIVE_DUTY_CYCLE_LOW SCAN_ADC_MEASUREMENT_RATIO_VS_SCAN_CYCLE_LOW	// currently both positive and negative use same timing
#define SCAN_ADC_MEASUREMENT_RATIO_NEGATIVE_DUTY_CYCLE_LOW SCAN_ADC_MEASUREMENT_RATIO_VS_SCAN_CYCLE_LOW	// currently both positive and negative use same timing

#define SCAN_DAC_POSITIVE_CONDITIONING_TIMER 6
#define SCAN_DAC_POSITIVE_TIMER 7
#define SCAN_DAC_NEGATIVE_TIMER 14
#define SCAN_ADC_MEASUREMENT_POSITIVE_TIMER 16
#define SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER 17

typedef struct
{
	bool first_scan_dac_flag;			// after every first scan in a measurement scan cycle this is set to false, used to start the DAC negative cycle
	bool first_scan_positive_adc_flag;	// after every first scan in a measurement scan cycle this is set to false, used to start the ADC positive cycle
	bool first_scan_negative_adc_flag;	// after every first scan in a measurement scan cycle this is set to false, used to start the ADC negative cycle
	
	bool scan_timer_initialised;		// Set to True after initialisation, as used to check if the timers are setup before starting them in the timer start function
	volatile bool still_scanning;
	bool positive_mark_of_scan;
	
	bool conditioning_timer_initialised;
	
	uint16_t conditioning_loop_count;
	uint16_t conditioning_loop_count_current_value;
	uint16_t conditioning_timer_count;

	uint16_t scan_cycle_timer_count;
	uint16_t scan_positive_dac_timer_count;
	uint16_t scan_negative_dac_timer_count;
	uint16_t scan_negative_dac_timer_init_count;

	uint16_t scan_positive_adc_timer_count;
	uint16_t scan_negative_adc_timer_count;
	uint16_t scan_positive_adc_timer_init_count;
	uint16_t scan_negative_adc_timer_init_count;
	
	double dac_scan_start_mv;
	double dac_scan_current_mv;
	double dac_scan_span_mv;
	double dac_last_scan_mv;
	double dac_scan_pulse_amplitude;
	double dac_scan_half_pulse_amplitude;
	double dac_step_size;

	double dac_a_non_inverting_positive_current_value;
	double dac_b_inverting_positive_current_value;
	double dac_a_non_inverting_negative_current_value;
	double dac_b_inverting_negative_current_value;
	
	uint16_t dac_a_non_inverting_current_code;
	uint16_t dac_b_inverting_current_code;
	uint16_t dac_a_non_inverting_previous_code;
	uint16_t dac_b_inverting_previous_code;
	
	bool dac_a_non_inverting_first_value_set_flag;		// 1 is first value, anything else not
	bool dac_b_inverting_first_value_set_flag;		// Set to False after first value is generated and reset to True on finishing a scan cycle for a single sensor

	bool reached_last_dac_setting_for_current_scan;
	bool processfurther;
	
	uint16_t adc_current_value;
	
	uint16_t * adc_mark_data;
	uint16_t * adc_space_data;
	float * adc_mark_space_array_1_data;
	float * dac_output_voltage_data;
	uint16_t adc_max_data_count;
	uint16_t adc_current_data_count;
} scantimer_struct;

void scan_init(void);

void scan_settings_reset_flags(void);

extern void dac_positive_scan_cycle_processor(void);

extern void dac_negative_scan_cycle_processor(void);

extern void adc_positive_scan_cycle_processor(void);

extern void adc_negative_scan_cycle_processor(void);

void conditioning_timer_init(void);

extern void timer_init(void);

extern void timer_calculate_counts(double frequency);

void conditioning_timer_loop(void);

void conditioning_timer_start(void);

extern void measurement_timer_start(void);

extern void measurement_timer_stop(void);

extern int scanning_in_progress(void);

void measurement_dac_settings(void);

void calculate_dac_settings(void);

void measurement_set_dac(void);

void set_adc_data_address(
		uint16_t * adc_mark_data,
		uint16_t * adc_space_data,
		float * adc_mark_space_array_1_data,
		float * dac_output_voltage_data,
		uint16_t adc_data_size);

void condition_ref(void);

void precondition_ph_electrodes(double peak_ph_potential);

uint8_t check_immersed_dry_status(void);

extern int8_t iref_failures, secondary_iref_failures;
extern bool no_good_iref_scans, using_previous_iref;
extern bool bad_electrode[NUMBER_OF_PH_ELECTRODES_MAX];
extern bool trust_iref, suspect_iref;
extern int cond_ref_hold_time_code;
extern bool healthwatch;

#endif /* TIMER_H_ */
