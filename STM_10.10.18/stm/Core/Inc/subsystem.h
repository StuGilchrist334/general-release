#ifndef INC_SUBSYSTEM_H_
#define INC_SUBSYSTEM_H_

#include <stdint.h>
#include <stdbool.h>


#define AFE_SPI_BAUDRATE 4000000
#define SLAVE_SPI_BAUDRATE 2000000

void enable_analog_power_5v(void);

void disable_analog_power_5v(void);

void enable_analog_power_2v5(void);

void disable_analog_power_2v5(void);

void enable_analog_power_all(void);

void disable_analog_power_all(void);

void enable_water_detection(void);

void disable_water_detection(void);

void enable_dac_output_to_counter_electrode(void);

void enable_afe_2nd_gain_resistor(void);

void disable_afe_2nd_gain_resistor(void);

void enable_afe_3rd_gain_resistor(void);

void disable_afe_3rd_gain_resistor(void);

void enable_afe_4th_gain_resistor(void);

void disable_afe_4th_gain_resistor(void);

void enable_afe_5th_gain_resistor(void);

void disable_afe_5th_gain_resistor(void);

void enable_afe_all_gain_resistors(void);

void disable_afe_all_gain_resistors(void);

void disable_sensor_electrodes(void);

void enable_ph_electrodes(void);

void enable_single_electrode(uint8_t channel, bool disable_others);

void enable_temperature_switch(void);

void enable_multielectrode_scan(bool last_scan);

void enable_electrodes(uint8_t electrode_number_start, uint8_t total_electrodes);

#endif /* INC_SUBSYSTEM_H_ */
