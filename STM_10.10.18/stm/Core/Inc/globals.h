#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdint.h>
#include <stdbool.h>

#define FIRMWARE_VERSION "STM_10.10.21"

//STM_10.8 release is STM_10.7.18
//STM_10.7.01 has new global scan variables, randomized numbering, adaptive scanning based on iref
//STM_10.5.1 is STM_10.0.43
//STM_10.0.25 has randomised fresh pairs
//STM_10.0.23 has new calibration, and long interval conditions
//STM_10.0.03 has fresh 302 from the start, goes to 2 electrodes at 50% failures and clears health. on successive failure limit, stored peak potential cleared
//#define FIRMWARE_VERSION "STM_9.99.50" renamed to STM_10.0 and released
//STM_9.99.26 has version number redesignated. main changes are in gain calculation, dynamic frequencies and pulse amplitudes,
//STM10.0 has loop count and new electrode sequencing, and corrections after 10 loops
//STM9.42 is STM9.41B with pH correction, new salinity equation, peak picking end point and condition ref changed
//STM9.41 is STM9.40 with higher iref gain down trigger and span change waits for good data. immersion threshold changed
//STM9.40 is STM9.39 with reduced condition ref time
//STM9.39 is STM9.38 with iref gain down rejection and ph outlier health increment
//STM9.38 is STM9.37 with better peak picking again
//STM9.37 is STM9.36 with better peak choosing for pH
//STM9.36 has condition_ref loop counter
//STM9.35 has lower condition_ref times, new pH equation, and omits first 50 points in peak picking
//STM9.34A is STM9.33 with max gain up to 9, qaqc limits lowered, reordering of information for new peak picking
//STM9.33is STM32 with peak picking improvements, loop counting for transducer health
//STM9.32 is STM30A with bux fixes in peak picking
//STM30A is STM29A with alternate peak picking and arrays, and some changes from STM9.31A
//STM9.29A has second peak picking pass back in, 2 fresh electrodes, corrected iref qaqc gain bump omission
//STM9.28 has sdcard command
//STM9.27 doesn't have second peak picking pass for pH
//STM9.26B is for Bjarni with amended fresh parameters
//STM9.26 has pH outlier rejection mod
//STM9.25 has different iref step sizes, 1 fresh electrode, iref avg amendment
//STM9.24 has increased number of iref scan for fresh
//STM9.23B has 25Hz iref fresh, C has no pp algo check
//STM9.23 has iref health change
//STM9.22 has some parameter changes, rejects pH outliers, resets span and start after multiple failures
//STM9.19 redirects scanf commands to fresh option
//STM9.18 has some parameter changes
//STM9.17 has 3 fresh electrodes, parameter changes
//STM9.15 is STM9.14B with some bug fixes
//STM9.14 has 6 double irefs for each scan type, other parameter changes
//STM9.13 has scano, scanf, scanfresh all in one routine
//STM9.12 has many small changes
//STM9.11 has Rahul's better peak picking routine, with scano removed because program size too large
//STM9.10.4 has rearranged code for correct iref health reporting
//STM9.10 is STM9.9 with some small amendments and fixes
//STM9.9 is STM9.8fresh with no ignored electrodes after second loop in scanfresh
//STM9.8fresh is STM9.0.1 with fresh scan and 5 averages in fast mode
//STM9.8 is STM9.7 with some bug fixes
//STM9.7 is STM9.7 with PH_FRESH_SCAN_AFTER_IREF_SCAN_SHIFT 1350
//STM9.0 is STM9.7 renamed for release
//STM9.7 is STM9.5B with bug fix in scanf if salinity out of range, pH was still being calculated. also, single set electrode scan wasn't taking temperature for salinity calculation
//STM9.5B is STM9.5 with bug fix in set electrode early exit
//STM9.5 is STM9.4 with amended scanf pH and conductivity calculation
//STM9.4 is STM9.2 with scanf salinity above 7.5 giving out of range value
//STM9.3 is STM9.2 with scanf pH above 7.5 out of range - this was not what was wanted, see above
//STM9.2 is STM9.1 with scanf set electrode changes, new scanf pH calculation
//STM9.1 is STM9.0 with 8 scanf scans, new scanf parameters, conductivity output, scanf precon all electrodes
//STM9.0 is STM8.60 with corrected scan_type / sensor_type, salinity calculation
//STM8.60 is STM8.44 renamed
//STM8.44 is STM8.42A with #define MAX_GAIN_SETTING 6
//STM8.42A is STM8.42 with #define PH_FRESH_SCAN_PULSE_AMPLITUDE_MV_DEFAULT 300
//STM8.42 is STM.41 with, scano1
//STM8.37 is STM.36 with amended pH calculation
//STM8.36 is STM.35 with code fix in pH calculation
//STM8.35 is STM8.34 with amended iref failure count
//STM8.34A has no immersion check nor condition ref
//STM8.34 is STM8.33 with no gain change, health change, or preconditioning if not immersed
//STM8.33 is STM8.32 with big fixed in gain setting
//STM8.32 is STM8.31 with new pH calculation, iref health reassessments, and stop command reply
//STM8.31 is STM8.27 with some bug fixes. version number gap is because of unconfirmed peak picking and DO code
//STM8.27 and 8.26 have some minor bug fixes
//STM8.25 is STM8.24 with 64 qaqc reinstated, no deliberate iref flipping, version number exchange with interface, eeprom save optimisation
//STM8.24 is STM8.23 with last electrode scanned saved, so next power on doesn't necessarily start at electrode 1
//STM8.23 is STM8.22 with some gain change conditions, 64 qaqc temporarily removed
//STM8.22 is STM8.21 with extended qaqc reply (64 elements)
//STM8.21 is STM8.20 with gain level changes and some parameter changes
//STM8.19 is STM8.18 with gain change conditional on qaqc
//STM8.18 is STM8.17 with gain change on ph qaqc lower limit fail
//STM8.17 reverts to 12 iref scans
//STM8.16.1 has 8 iref scans (from 12) and eeprom load default command
//STM8.14 is STM8.13.1 with fix in update_after_scan for start potentials. removed frequency constraints in timer_calculate_counts. fix saving to gain array
//STM8.13.1 is STM8.13 with no gain up change on iref space data
//STM8.13 is STM8.12 with saved avg pH and temperature
//STM8.12 is STM8.11 with 12 irefs, start end reporting
//STM8.11 is STM8.10 with reordered transducer health, added iref failure to qaqc, last pH and temperature remembered
//STM8.10 is STM8.9 with condition_ref at end of every loop and before first look if preconditioning requested
//STM8.9 is STM8.7 with precautions when artificial sensor number is wild (STM8.8 was a transition of this)
//STM8.7 is STM8.6 with immersion retry and minor mods
//STM8.6 is STM8.3 with multielectrode scans inserted, and electrodes 3 and 4 swapped
//STM8.3 is STM8.2 with some bug fixes and parameter changes
//STM8.2 is STM8.1 with electrode 1 scans not included in array unless looped, precondition done after electrode 1
//STM8.1 is STM8.0 with im_adc always closed
//STM8.0 is STM7.13 with large pH array, immersion, amended precondition and some parameters. 3s hold done at DAC = 0, sensor enabled.
//STM7.12 is STM7.11 with gain change fixed for typo
//STM7.11 is STM7.10 with gain change fixed again
//STM7.10 is STM7.7A with triple save and check of serial number, and fix to gain change routine
//STM7.7A is STM7.7 with triple save and check of temperature calibration data
//STM7.7 is STM7.6 with change of scan_start_mv only if electrode health is zero
//STM7.6 is STM7.5 with no change of scan_start_mv if electrode health is too high
//STM7.5 is STM7.4 with electrodes 3, 4, 5, 7 ignored in pH running average
//STM7.4 is STM7.2 with 3s hold but without electrscanoodes enabled
//STM7.2 is STM.7.1 with quadratic temperature calibration, fixed 3s hold at end of third scan
//STM7.1 is STM7 with water immersion and condition_ref command response, although not implemented
//STM7 is 29.12.10
//29.12.10 fixes ph array not being read from eeprom
//29.12.9 adds salinity and conductivity and 3s hold after end of scan
//29.12.8 adds averaging temperature code and calibration, shutdown procedure, iref averaging
//29.12.7 fixes electrode numbering

#define ADC_DATA_ARRAY_SIZE 1400

extern uint16_t adc_mark_data_global[ADC_DATA_ARRAY_SIZE];
extern uint16_t adc_space_data_global[ADC_DATA_ARRAY_SIZE];
extern float adc_mark_space_array_1_global[ADC_DATA_ARRAY_SIZE];
extern float adc_mark_space_array_2_global[ADC_DATA_ARRAY_SIZE];
extern float dac_output_voltage_data_global[ADC_DATA_ARRAY_SIZE];
extern volatile bool anb_sensors_interface_board_connected;
extern uint8_t ph_start_electrode_number;
extern uint8_t ph_electrodes_completed;
extern uint8_t loop_interval;
extern bool event_passed;
extern bool ph_scan, iref_scan, ocean_scan, fresh_scan, vf_scan, dynamic_salinity;
extern bool fast_profiling, loop_complete;
extern int electrodes_scanned_in_this_loop;
extern int post_multielectrode_electrode_number;
extern int loops_since_start;
extern double last_good_peak_potential;
extern int ringer_electrode_1;
extern int ringer_electrode_2;

#define QAQC_REPLY_LAST_SCAN 0x00
#define QAQC_REPLY_NOT_LAST_SCAN 0xFF
#define QAQC_REPLY_NOT_USEFUL_CUSTOMER_DATA 0x00
#define QAQC_REPLY_USEFUL_CUSTOMER_DATA 0xFF


typedef struct
{
	uint8_t electrode_number;
	uint8_t gain;
	double calculated_ph;
	double calculated_temperature;
	uint32_t read_temperature;
	double salinity;
	double actual_conductivity;
	double specific_conductivity;
	uint8_t electrode_health;
	uint8_t array_health;
	uint8_t sensor_diag;
	uint8_t data_status;
	double peak_potential;
	uint16_t peak_potential_reply;		// bcd
	double parameters_diagnostic;
	uint16_t parameters_diagnostic_reply;	// bcd
	double avg_iref_peak_potential;
	uint16_t avg_iref_peak_potential_reply;	// bcd
	uint16_t peak_value;
	uint16_t peak_value_reply;
	uint16_t failure_diagnostic;
	uint16_t failure_diagnostic_reply;
	uint16_t max_value;
	uint16_t max_value_reply;
	uint16_t first_peak_potential;
	uint16_t first_peak_potential_reply;
	uint16_t qaqc_2;
	uint16_t qaqc_2_reply;
	uint8_t useful_customer_data;
	uint8_t last_qaqc;				// 0x00 is last one in the current measurement cycle, else 0xFF
	double running_average_ph;
	double start_potential;
	double end_potential;
	uint16_t temperature_adc;
	uint16_t temperature_adc_range;
	uint16_t immersion_adc_difference;

	float adc_value;

} qaqc_reply_struct;


#endif /* GLOBALS_H_ */
