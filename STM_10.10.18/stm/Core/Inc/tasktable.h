#ifndef TASKTABLE_H_
#define TASKTABLE_H_

#include <stdbool.h>
#include <stdint.h>

#define NUMBER_OF_PH_ELECTRODES_ANB 11
#define NUMBER_OF_PH_ELECTRODES_VALEPORT 13
#define NUMBER_OF_PH_ELECTRODES 11
#define NUMBER_OF_PH_ELECTRODES_MAX NUMBER_OF_PH_ELECTRODES_VALEPORT
#define PEAKS_LOOPS_ARRAY_SIZE 13

#define PH_ARRAY_SIZE_MAX NUMBER_OF_PH_ELECTRODES_MAX*2
#define GAIN_ARRAY_SIZE 15

#define IREF_SCAN_TYPE 4
#define PH_SCAN_TYPE 1

#define SCAN_TYPE_OCEAN 1
#define SCAN_TYPE_FRESH 2
#define SCAN_TYPE_VF 3

#define MAX_GAIN_SETTING 9
#define MAX_GAIN_PH 9
#define MAX_GAIN_IREF 9

#define MAX_PRECON_ELECTRODES NUMBER_OF_PH_ELECTRODES //6

#define VALUE_UNKNOWN 99.999

extern uint8_t number_of_ph_electrodes;

typedef struct
{
	int sensor_number;
	int number_of_scans;
	double frequency;
	double pulse_amplitude;
	double step_size;
	double start_mv;
	double span_mv;
	int scan_number_current;
	bool first_ph_scan_after_iref;
	bool first_scan;
	bool last_scan;
	int gain_resistor_setting;
	bool multielectrode_scan;
	bool set_electrode;
	bool regular_electrode;
	bool include_in_ph_array;
	int second_sensor_number;
	int third_sensor_number;
} task;

extern task current_task;

extern uint8_t electrode_gain_values[GAIN_ARRAY_SIZE];
extern uint8_t multielectrode_gain;
extern bool electrode_gain_bumped[GAIN_ARRAY_SIZE];
extern bool gain_bump_failed[GAIN_ARRAY_SIZE];

extern double peak_potential_offsets[NUMBER_OF_PH_ELECTRODES_MAX];
extern double peak_potentials[NUMBER_OF_PH_ELECTRODES_MAX];
extern double qaqcs[NUMBER_OF_PH_ELECTRODES_MAX];
extern double ph_peaks_loops[PEAKS_LOOPS_ARRAY_SIZE];
extern double iref_peaks_loops[PEAKS_LOOPS_ARRAY_SIZE];
extern uint8_t electrode_pulse_amplitudes[NUMBER_OF_PH_ELECTRODES_MAX];
extern uint8_t electrode_frequencies[NUMBER_OF_PH_ELECTRODES_MAX];
extern uint8_t electrode_clipped_at_gain[NUMBER_OF_PH_ELECTRODES_MAX];

extern bool precondition_select[NUMBER_OF_PH_ELECTRODES_MAX];
extern volatile bool perform_precondition_ph_electrodes_flag;
extern volatile bool abraded;
extern bool ph_clipped, iref_clipped, use_single_iref;
extern bool good_iref_gain_found;
extern int good_iref_gain;
extern int post_clipped_gain;
extern int good_peak_count;
extern double emergency_iref_peak_potential;
extern bool emergency_iref_peak_found;

#endif /* TASKTABLE_H_ */
