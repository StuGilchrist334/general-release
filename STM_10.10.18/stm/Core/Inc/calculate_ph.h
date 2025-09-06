/**
 * @file calculate_ph.h
 *
 * @brief Interface to pH calculations
 *
 * @author Dave Adshead
 * @version 1.2
 * @date 202212231757 created - DA
 * @date 202302161706 modified - PV - put header within ifndef, renamed file
 * @date 202302170925 modified - PV - changed from lookup table to calculated value based on Kay's calculations
 */


#ifndef CALCULATE_PH_H_
#define CALCULATE_PH_H_

#include <math.h>
#include <stdbool.h>
#include "tasktable_master.h"
#include "tasktable_init.h"
#include "tasktable.h"

#define PH_DEFAULT_OUT_OF_RANGE_VALUE 15 //when pH hasn't been successfully calculated yet, use 15 ie outside the valid range of 0 to 14
#define SALINITY_DEFAULT_OUT_OF_RANGE_VALUE 99.99
#define BAD_ELECTRODE_HEALTH_THRESHOLD 3 //6 SG!
#define MAX_HEALTH 7
#define IREF_BAD_COUNT_THRESHOLD 3 //iref deemed bad with this number of total loops having IREF_FAILURE_THRESHOLD scans failed
#define IREF_FAILURE_THRESHOLD 3 //scans failed in one loop to signify a bad iref

extern double ph_values[PH_ARRAY_SIZE_MAX];
extern int8_t ph_array_index;
extern bool electrode_array_looped;
extern bool ph_failed_only_as_outlier;
extern double displayed_ph_running_average;
extern double last_ph_std_dev;
extern double last_temperature;
extern double ph_alt_1, ph_alt_2;
extern double last_passed_ph_qaqc;
extern double scan_health;

extern uint8_t iref_health_bad_count;
extern uint8_t iref_secondary_health_bad_count;

extern volatile uint8_t electrode_health_values[NUMBER_OF_PH_ELECTRODES_MAX];
extern bool salinity_out_of_range;

double reading_avg(double *readings, int num_readings);

double calculate_ph(double vpeak_mV, double temps, double Iref_mV);

double calculate_ph_total_average_scan(int sensor_number);

double calculate_ph_running_average_scan(int sensor_number, bool set_electrode, double scan_average_ph);

uint8_t get_transducer_health(int sensor_number);
uint8_t get_electrode_health_value(int sensor_number, int second_sensor_number, double calculated_ph, int gain_change);

void clear_ph_array(void);
void clear_electrode_health(void);
void clear_peak_potential_offsets(void);
void set_default_gains(void);
double quadratic_fit_end_value(double sub_array[], int start, int size, bool end_or_next);
double logistic(double x, double min, double max, double k, double mid);
double median_of_three(double a, double b, double c);
void calculate_scan_health(void);

#endif /* CALCULATE_PH_H_ */
