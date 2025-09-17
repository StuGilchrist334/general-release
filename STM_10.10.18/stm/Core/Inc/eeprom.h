#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>
#include <stdbool.h>
#include <eeprom_M95080.h>
#include <scan.h>
#include "globals.h"
#include "eeprom_address_definition.h"
#include "usart_commsboard.h"
#include "calculate_ph.h"

void eeprom_write_square_wave_data_save_status(uint8_t value);

uint8_t eeprom_read_square_wave_data_save_status(void);

void eeprom_write_external_baud_rate_setting(uint8_t value);

uint8_t eeprom_read_external_baud_rate_setting(void);

void eeprom_write_sensor_array_interval_setting(uint8_t value);

uint8_t eeprom_read_sensor_array_interval_setting(void);

void eeprom_write_sensor_mode_setting(uint8_t value);

uint8_t eeprom_read_sensor_mode_setting(void);

void eeprom_write_sensor_serial_number(uint8_t * buffer_start_address);

void eeprom_read_sensor_serial_number(uint8_t * buffer_start_address);

void eeprom_write_ph_output_style_setting(uint8_t value);

uint8_t eeprom_read_ph_output_style_setting(void);

void eeprom_write_immersion_rule(uint8_t value);

void eeprom_write_immersion_rule_interval(uint8_t value);

uint8_t eeprom_read_immersion_rule(void);

uint8_t eeprom_read_immersion_rule_interval(void);

void eeprom_write_array_health(uint8_t value);

uint8_t eeprom_read_array_health(void);
/**
* @brief Check if sensor autonomous start delay setting is valid.
*
* The valid values are 0 minutes, 5 minutes to 24 hours.
* This code would do the following:
*	1 to 4 minutes would be set to 5 minutes, the minimum allowed
*	anything working out to more than 24 hours would be set to 24 hours
*	eg:
* 	2 hours 75 minutes would be set as 3 hours and 15 minutes
*	67 minutes would be set as 1 hour and 7 minutes
*	60 minutes would be set as 1 hour and 0 minutes
*	23 hours 75 minutes would be set as 24 hours and 0 minutes, as
*		24 hours and 15 minutes exceeds maximum of 24 hours
*
* @author Palaniappan Valliappan
* @version 1.0
* @date 202302211250 created - PV
*/
uint16_t condition_check_sensor_autonomous_start_delay_setting(uint8_t hours, uint8_t minutes);

void eeprom_write_sensor_autonomous_start_delay_setting(uint8_t hours, uint8_t minutes);

uint16_t eeprom_read_sensor_autonomous_start_delay_setting(void);

void reset_eeprom_settings(void);

void eeprom_write_temperature_calibration_number(uint8_t * buffer_start_address);

void eeprom_write_sensor_name(uint8_t * buffer_start_address);

void eeprom_read_sensor_name(uint8_t * buffer_start_address);

void eeprom_write_sensor_sub_name(uint8_t * buffer_start_address);

void eeprom_read_sensor_sub_name(uint8_t * buffer_start_address);

uint8_t eeprom_read_heartbeat_output_style_setting(void);

void eeprom_write_heartbeat_output_style_setting(uint8_t value);

void eeprom_read_peak_potentials(void);

void eeprom_read_ph_array(void);

void eeprom_read_qaqcs(void);

void eeprom_read_ph_peaks_loops(void);

void eeprom_read_iref_peaks_loops(void);

void eeprom_read_peak_potential_offsets(void);

void eeprom_read_pulse_amplitudes(void);

void eeprom_read_clipped_at_gain(void);

void eeprom_read_frequencies(void);

void eeprom_save_ph_array(void);

void eeprom_save_peak_potentials(void);

void eeprom_save_qaqcs(void);

void eeprom_save_ph_peaks_loops(void);

void eeprom_save_iref_peaks_loops(void);

void eeprom_save_peak_potential_offsets(void);

void eeprom_read_electrode_health(void);

void eeprom_read_electrode_gains(void);

void eeprom_save_electrode_health(void);

void eeprom_save_pulse_amplitudes(void);

void eeprom_save_frequencies(void);

void eeprom_save_clipped_at_gain(void);

void eeprom_save_electrode_gains(void);

void eeprom_save_last_temperature(void);

void eeprom_save_highest_ph_qaqc(void);

void eeprom_save_ph_running_average(void);

void eeprom_save_avg_iref_peak_potential(void);

void eeprom_save_avg_ph_peak_potential(void);

double eeprom_read_last_temperature(void);

double eeprom_read_highest_ph_qaqc(void);

double eeprom_read_ph_running_average(void);

double eeprom_read_last_iref_peak_potential(void);

double eeprom_read_avg_ph_peak_potential(void);

uint8_t eeprom_read_sensor_communication_style(void);

void eeprom_write_sensor_communication_style(uint8_t value);

void eeprom_write_anbsensors_or_alternate_head(uint8_t value);

void eeprom_write_interface(uint8_t value);

void eeprom_write_fast_profiling(uint8_t value);

void eeprom_write_use_sdcard(uint8_t value);

void eeprom_write_power(uint8_t value);

void eeprom_write_salinity_range(uint8_t value);

void eeprom_write_phs_per_loop(uint8_t value);

void eeprom_write_fresh_electrodes(uint8_t value);

void eeprom_write_loops_since_last_good_ph(uint8_t value);

void eeprom_write_loops_since_last_event(uint8_t value);

void eeprom_write_loops_since_last_correction(uint8_t value);

void eeprom_write_loops_since_start(uint8_t value);

void eeprom_write_next_set_electrode(uint8_t value);

void eeprom_write_loop_count(uint32_t value);

uint8_t eeprom_read_anbsensors_or_alternate_head(void);

uint8_t eeprom_read_interface(void);

uint8_t eeprom_read_fast_profiling(void);

uint8_t eeprom_read_use_sdcard(void);

uint8_t eeprom_read_power(void);

uint8_t eeprom_read_salinity_range(void);

uint8_t eeprom_read_phs_per_loop(void);

uint8_t eeprom_read_fresh_electrodes(void);

uint8_t eeprom_read_loops_since_last_good_ph(void);

uint8_t eeprom_read_loops_since_last_event(void);

uint8_t eeprom_read_loops_since_last_correction(void);

uint8_t eeprom_read_loops_since_start(void);

uint8_t eeprom_read_next_set_electrode(void);

uint32_t eeprom_read_loop_count(void);

void eeprom_write_system_modbus_address(uint8_t value);

uint8_t eeprom_read_system_modbus_address(void);

void eeprom_write_scan_n_no_of_ph_electrodes(uint8_t value);

uint8_t eeprom_read_scan_n_no_of_ph_electrodes(void);

void eeprom_write_last_electrode_number(uint8_t value);

uint8_t eeprom_read_last_electrode_number(void);

void eeprom_write_driver_version(uint8_t * buffer_start_address);
void eeprom_write_interface_version(uint8_t byte_one, uint8_t byte_two);
void eeprom_write_iref_health(uint8_t value);
void eeprom_write_iref_secondary_health(uint8_t value);
void eeprom_write_iref_last_used(uint8_t value);
void eeprom_write_iref_health_bad_count(uint8_t value);
void eeprom_write_iref_secondary_health_bad_count(uint8_t value);

uint8_t eeprom_read_iref_health(void);
uint8_t eeprom_read_iref_secondary_health(void);
uint8_t eeprom_read_iref_last_used(void);
uint8_t eeprom_read_iref_health_bad_count(void);
uint8_t eeprom_read_iref_secondary_health_bad_count(void);
uint8_t read_interface_driver_version(void);
void eeprom_read_driver_version(uint8_t * buffer_start_address);

uint8_t eeprom_read_byte_with_checks(eeprom_addr_t address_definition, uint8_t lower_bound, uint8_t upper_bound, uint8_t default_value);
float eeprom_get_float_from_eeprom_bytes(eeprom_addr_t address_definition_start);

void eeprom_wait_for_write_to_complete(void);

#endif /* EEPROM_M95080_H_ */
