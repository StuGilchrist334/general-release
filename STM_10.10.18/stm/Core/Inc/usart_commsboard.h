#ifndef USART_COMMSBOARD_H_
#define USART_COMMSBOARD_H_

#include "main.h"
#include <math.h>
#include "commands.h"
#include "globals.h"
#include "eeprom.h"
#include "crc_util.h"


#define STM_BOARD_REVISION 0x01

#define TRANSMIT_BUFFER_SIZE 64
#define COMMAND_MAXIMUM_SIZE 40
#define SPI_SLAVE_DATA_FRAME_SIZE_IN_BYTES 12

#define SPI_DATA_FRAME_END_SYMBOL '/'
#define SPI_DATA_FRAME_APPEND_SYMBOL '+'

extern volatile bool perform_ocean_measurement_routine_flag;
extern volatile bool perform_fresh_measurement_routine_flag;
extern volatile bool perform_vf_measurement_routine_flag;
extern volatile bool perform_existing_measurement_routine_flag;
extern volatile bool continue_measurement_flag;
extern volatile bool scan_internal_interval_flag;
extern volatile bool scan_long_interval_flag;
extern volatile bool scan_short_interval_flag;
extern volatile bool scan_external_interval_flag;
extern volatile bool scan_first_flag;
extern volatile bool scan_interval_flag;
extern volatile int precondition_ph_case;
extern volatile bool immersion_status_requested_flag;
extern volatile bool immersed_flag;
extern volatile bool already_scanning;

extern uint32_t immersion_adc_difference;
extern volatile uint16_t precon_diagnostic, failure_diagnostic, parameters_diagnostic;
extern uint32_t first_peak_potential;
extern float peak_potential_i1, peak_potential_i2, peak_potential_diff_raw, peak_potential_diff_balanced;
extern float peak_potential_displayed_2, peak_potential_displayed_3, peak_potential_used;
extern int16_t temperature_adc, temperature_adc_range;
extern float signal_scatter;



typedef struct
{
	bool first_byte_received_flag;
	bool timeout_timer_initialised;
	bool still_waiting_for_next_byte;
	bool timer_running;
	bool qaqc_data_waiting_to_read_flag;

	uint8_t command_size;
	uint8_t command_size_with_crc;
	uint8_t reply_size;
	uint8_t reply_size_with_crc;
	uint8_t command_byte;
	uint8_t command_option_byte;
	uint8_t received_count;
	uint8_t transmit_count;

	uint16_t timeout_timer_count;

	bool square_wave_data_save_flag;
	bool ph_heartbeat_style_flag;
	bool all_heart_beat_data_flag;
} usart_timer_struct;


void usart_initialise(void);

void usart_timer_init(void);

void clear_received_buffer(void);

void usart_timer_start(void);

void usart_timer_stop(void);

void process_commands(void);

void send_one_dataframe_as_spi_slave(uint8_t * dataframe);

void convert_dac_potential_to_custom_bcd(uint8_t * output_data, double value);

void convert_dac_potential_to_custom_bcd_2_byte_qaqc(uint8_t * output_data, double value);

void send_data_as_spi_slave(
		uint16_t * adc_mark_data,
		uint16_t * adc_space_data,
		float * adc_mark_space_array_1_data,
		float * adc_mark_space_array_2_data,
		float * dac_output_voltage_data,
		uint16_t adc_data_size,
		bool last_scan_of_last_electrode);

void reply_echo_data(void);

void send_reply(void);

void reply_serial_number(void);

void reply_sensor_name(void);

void reply_sensor_sub_name(void);

void reply_manufacturer_name(void);

void reply_external_baud_rate(void);

void reply_scan_n_number(void);

void reply_sensor_communication_style(void);

void reply_sensor_mode(void);

void reply_autonomous_start_delay(void);

void reply_sensor_array_interval(void);

void reply_driver_version(void);

void reply_system_modbus_address(void);

void reply_system_immersion_rule(void);

void reply_sensor_ph_output_style(void);

void reply_array_health(void);

void reply_stop_scanning(void);

void reply_square_wave_data_save_status(void);

void reply_qaqc_data_ready_to_be_read(void);

void get_qaqc_reply_data_address(qaqc_reply_struct * qaqc_reply);

void reverse_for_endianness(uint8_t* destination_start_address, int32_t size_n);

void int32_to_byte_array(uint8_t* destination_start_address, int32_t value);

void float_to_byte_array(uint8_t* destination_start_address, float value);

void reply_qaqc_data(void);

bool get_qaqc_data_waiting_to_read_flag_status(void);

void set_square_wave_data_save_status(void);

void set_external_baud_rate_setting(void);

void set_sensor_array_interval(void);

void set_sensor_mode(int mode);

void set_autonomous_start_delay(void);

void set_sensor_serial_number(void);

void set_sensor_ph_output_style(void);

void set_system_immersion_rule(void);

void set_fast_profiling(void);

void set_use_sdcard(void);

void set_power(void);

void set_salinity_range(void);

void set_phs_per_loop(void);

void get_stored_values_from_eeprom_to_global(void);

void byte_array_to_float(uint8_t* destination_start_address, float value);

void set_sensor_temperature_calibration(void);

void set_sensor_name(void);

void set_sensor_sub_name(void);

void reply_ph_heartbeat_style_status(void);

void set_ph_heart_beat_style_status(void);

void get_stored_heart_beat_values_from_eeprom_to_global(void);

bool get_ph_heart_beat_style_status(void);

void set_anbsensors_or_alternate_head(void);

void set_interface(void);

void reply_anbsensors_or_alternate_head(void);

void reply_interface(void);

void reply_echo_data_board(void);

void reply_get_temperature(void);

float system_get_temperature(void);

void set_system_modbus_address(void);

void reply_need_more_time(void);

void reply_scan_n_no_of_electrodes(void);

void reply_immersed_dry_status(void);

void reply_firmware_version(void);

void reply_fast_profiling(void);

void reply_use_sdcard(void);

void reply_power(void);

void reply_salinity_range(void);

void reply_phs_per_loop(void);

void check_message(uint8_t command[], uint8_t Size);

#endif /* USART_COMMSBOARD_H_ */
