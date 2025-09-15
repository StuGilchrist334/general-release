#ifndef TASKTABLE_INIT_H_
#define TASKTABLE_INIT_H_

#include "tasktable.h"
#include "eeprom.h"

#define IREF_CROSSOVER_POTENTIAL 750

//scan numbers
#define IREF_SCAN_NUMBER_MAX 12
#define IREF_SCAN_NUMBER_OCEAN 7
#define IREF_SCAN_NUMBER_FRESH 8
#define PH_SCAN_NUMBER_DEFAULT 3
#define MULTIELECTRODE_SCAN_NUMBER_DEFAULT 10

//peak potential defaults
#define IREF_OCEAN_SCAN_PEAK_POTENTIAL_DEFAULT 730
#define IREF_FRESH_SCAN_PEAK_POTENTIAL_DEFAULT 900
#define PH_PEAK_POTENTIAL_DEFAULT 100
#define IREF_CAP_HIGH 905
#define IREF_CAP_LOW 730

//start defaults
#define IREF_OCEAN_START_MV_DEFAULT 1150
#define IREF_FRESH_START_MV_DEFAULT 1200

//span defaults
#define IREF_OCEAN_SPAN_MV_INITIAL 700
#define IREF_OCEAN_SPAN_MV_REDUCED 500

#define IREF_FRESH_SPAN_MV_INITIAL 800
#define IREF_FRESH_SPAN_MV_REDUCED 700

#define PH_OCEAN_SPAN_MV_INITIAL 850
#define PH_FRESH_SPAN_MV_INITIAL 700

//peak shift defaults
#define PH_OCEAN_PEAK_SHIFT_START 275
#define PH_OCEAN_PEAK_SHIFT_END 225

#define PH_FRESH_PEAK_SHIFT_START 275
#define PH_FRESH_PEAK_SHIFT_END 225

//preconditioning
#define PRECONDITIONING_PH_PEAK_PLUS_CASE_1 225
#define PRECONDITIONING_PH_PEAK_PLUS_CASE_2 225
#define PRECONDITIONING_TIME_S 3

//types
#define IREF_SCAN_TYPE_DEFAULT IREF_SCAN_TYPE					// iREF scan type currently 4
#define PH_SCAN_TYPE_DEFAULT PH_SCAN_TYPE						// pH scan type currently 1

#define IREF_PRIMARY_SENSOR_NUMBER 15
#define IREF_SECONDARY_SENSOR_NUMBER 14

//frequencies
#define IREF_OCEAN_FREQUENCY_DEFAULT 140 //60
#define IREF_FRESH_FREQUENCY_DEFAULT 30 //25
#define IREF_FRESH_FREQUENCY_RUN 30

#define PH_OCEAN_FREQUENCY_DEFAULT 100
#define PH_FRESH_FREQUENCY_DEFAULT 30

//pulse amplitudes
#define IREF_OCEAN_PULSE_AMPLITUDE_MV_DEFAULT 80
#define IREF_FRESH_PULSE_AMPLITUDE_MV_DEFAULT 250

#define PH_OCEAN_PULSE_AMPLITUDE_MV_DEFAULT 40
#define PH_FRESH_PULSE_AMPLITUDE_MV_DEFAULT 80

#define MULTIELECTRODE_PULSE_AMPLITUDE 40

//step sizes
#define IREF_OCEAN_STEPSIZE_MV_INITIAL 1 //2
#define IREF_OCEAN_STEPSIZE_MV_RUN 1
#define IREF_FRESH_STEPSIZE_MV_DEFAULT 2 //3

#define PH_OCEAN_STEPSIZE_MV_DEFAULT 1
#define PH_FRESH_STEPSIZE_MV_DEFAULT 2

#define MULTIELECTRODE_STEPSIZE_MV 3

#define MV_TO_STEPS_1MV 26.25										// Brandon's measurement gave 1mV step = 26.25 - updated on 202212211503

//scan number defaults
#define SCAN_NUMBER_CURRENT_DEFAULT 1
#define FIRST_PH_SCAN_AFTER_IREF_DEFAULT false
#define FIRST_SCAN_DEFAULT true
#define LAST_SCAN_DEFAULT false
#define MULTIELECTRODE_SCAN_DEFAULT false
#define SET_ELECTRODE_DEFAULT false
#define INCLUDE_IN_PH_ARRAY_DEFAULT false

//gain resistor settings
#define IREF_GAIN_RESISTOR_SETTING_OCEAN 1
#define IREF_GAIN_RESISTOR_SETTING_FRESH 1

#define PH_GAIN_RESISTOR_SETTING_OCEAN 5
#define PH_GAIN_RESISTOR_SETTING_FRESH 6

extern volatile bool reset_electrode_gains_flag;
extern uint8_t anb_or_alternate_head;
extern bool ph_4_energy_boost;

int get_iref_sensor_number(void);
int task_get_electrode_gain(int sensor_number);
void task_set_electrode_gain(int new_gain);
task get_first_iref_scan(int scan_type);
void get_first_ph_scan(void);
void update_electrode_numbers(void);
void reset_frequencies(void);
void reset_pulse_amplitudes(void);
void reset_qaqcs(void);
void reset_peaks_loops(void);
void reset_peak_potentials(void);
void reset_clipped_at_gain(void);
void reset_fresh_electrodes(void);
void add_ph_peak_to_loop_array(double peak);
void add_iref_peak_to_loop_array(double peak);

#endif /* TASKTABLE_INIT_H_ */
