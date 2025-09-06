#ifndef DATA_SMOOTHING_H_
#define DATA_SMOOTHING_H_

#include <math.h>
#include <stdlib.h>
#include "globals.h"
#include "usart_commsboard.h"
#include "tasktable.h"
#include "tasktable_master.h"

#define MAX_PEAKS 10

#define ph_peak_value_LL 400
#define ph_fresh_QAQC1_LL 10
#define ph_ocean_QAQC1_LL 10
#define ph_fresh_QAQC2_LL 10
#define ph_ocean_QAQC2_LL 10

#define ph_ocean_QAQC_parameter_change_LL 80
#define ph_fresh_QAQC_parameter_change_LL 25

#define iref_peak_value_LL 1000
#define iref_QAQC1_LL 10
#define iref_QAQC2_LL 10

#define iref_QAQC_parameter_change_LL 200

extern bool ph_flags_ok, perfect_pass, no_peaks_found;
extern int outlier_trend_count;
extern bool outlier_increasing, force_start_change;
extern bool iref_flags_ok;
extern bool this_scan_clipped;

extern bool ph_ocean_QAQC_parameter_change_flag;
extern bool ph_fresh_QAQC_parameter_change_flag;
extern bool iref_QAQC_parameter_change_flag;

extern float all_ph_peaks[MAX_PEAKS][4];

void smoothing_function(
	uint16_t size_of_array,
	qaqc_reply_struct *output_data);

typedef struct
{
	int peak_index;
	int first_peak_index;
	int baseline_1_index;
	int baseline_2_index;

	float peak_potential;
	float baseline_1_potential;
	float baseline_2_potential;

	uint16_t peak_value;
	uint16_t baseline_1_value;
	uint16_t baseline_2_value;

	uint16_t QAQC1;
	uint16_t QAQC2;
	uint16_t corrected_qaqc;

	float peaks[MAX_PEAKS][10];
}peak_picking;

bool find_maximum(float* input_array, int start_index, int end_index, int* peak_index, float* peak_value);
bool find_minimum(float* input_array, int start_index, int end_index, int* peak_index, float* peak_value);
void find_baseline_1(float *input_array, int start_index, int end_index, int* baseline_1_index, float* baseline_1_value);
void find_baseline_2(float *input_array, int start_index, int end_index, int* baseline_2_index, float* baseline_2_value);
float quadratic_fit_centre_value(int sub_array[], int size);
uint16_t calculate_median(int sub_array[], int size);
int cmpfunc (const void * a, const void * b);
float get_signal_to_noise(float *adc_mark_space_array, int peak_index);
void median_filter(float *input_array, float *output_array, int median_filter_size_default, int size_of_array);
void box_filter(float *input_array, float *output_array, int box_filter_size_default, int size_of_array);
void quad_fit_filter(float *input_array, float *output_array, int filter_size_default, int size_of_array);
float compute_stddev(float *arr, int size);
float compute_local_scatter(float *arr, int size);
void gradient_filter(float *input_array, float *output_array, int step, int size_of_array);
float find_peak(float *input_array, float *peak_potential, float *qaqc, uint16_t *value_at_peak, uint16_t size_of_array, bool maximum, bool use_stored_peaks);
void clean_all_ph_peaks(bool reset);
int match_with_trusted_peak(void);
void create_trusted_peak(int best_peak_index);
int find_highest_qaqc(void);
int find_closest_to_last_peak(void);
int find_lowest_noise(void);
void update_peaks(int best_peak_index);
bool blacklisted_peak(double peak_potential);

#endif /* DATA_SMOOTHING_H_ */
