#ifndef TASKTABLE_MASTER_H_
#define TASKTABLE_MASTER_H_

#include "tasktable_init.h"

#define IREF_SCAN_AVERAGE_NUMBER 3 //4

extern double avg_iref_peak_potential, ph_peak_shift_start, iref_loop_movement, suspect_iref_peak_potential;
extern double avg_ph_peak_potential, peak_potential_median;
extern volatile bool avg_ph_peak_potential_stored, remember_preconditioning_done, pp_cleared;
extern volatile bool preconditioning_done;
extern bool first_good_ph_data_found;
extern bool first_good_iref_data_found;
extern bool reset_start_and_span;
extern int iref_last_used;
extern bool new_start_mV_set;
extern bool rolling_ph_electrode_mode;
extern double previous_iref_peak_potential, previous_ph_peak_potential, previous_ph;

void process_peak_potentials_after_scan(void);
void update_task_after_scan(bool iref_ok, bool ph_ok, int *successive_ph_scan_failures);

void update_iref_health_status(void);
void update_iref_secondary_health_status(void);
void calculate_peak_potential_offsets(bool store);
int cmpfunc_d(const void * a, const void * b);


#endif /* TASKTABLE_MASTER_H_ */
