#include <stdbool.h>
#include "main.h"
#include "subsystem.h"
#include "ioport.h"
#include "tasktable.h"
#include "globals.h"

void enable_analog_power_5v(void)
{
	ioport_set_pin_level(power_5v_analog_enable, IOPORT_PIN_LEVEL_HIGH);
}

void disable_analog_power_5v(void)
{
	ioport_set_pin_level(power_5v_analog_enable, IOPORT_PIN_LEVEL_LOW);
}

void enable_analog_power_2v5(void)
{
	ioport_set_pin_level(power_psu_2v5_enable, IOPORT_PIN_LEVEL_HIGH);
}

void disable_analog_power_2v5(void)
{
	ioport_set_pin_level(power_psu_2v5_enable, IOPORT_PIN_LEVEL_LOW);
}

void enable_analog_power_all(void)
{
	enable_analog_power_5v();
	enable_analog_power_2v5();
}

void disable_analog_power_all(void)
{
	disable_analog_power_5v();
	disable_analog_power_2v5();
}

void enable_water_detection(void)
{
	disable_sensor_electrodes();
	ioport_set_pin_level(water_detect_enable, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(im_adc, IOPORT_PIN_LEVEL_HIGH);
	enable_dac_output_to_counter_electrode();
}

void disable_water_detection(void)
{
	disable_sensor_electrodes();
	//ioport_set_pin_level(im_adc, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(water_detect_enable, IOPORT_PIN_LEVEL_LOW);
}

void enable_dac_output_to_counter_electrode(void)
{
	ioport_set_pin_level(power_ground_in3_ctr_to_ctr_drv, IOPORT_PIN_LEVEL_HIGH);
}

void enable_afe_2nd_gain_resistor(void)
{
	// turn off the switch to include resistor in series with the first one
	ioport_set_pin_level(gain_select_B2, IOPORT_PIN_LEVEL_LOW);
}

void disable_afe_2nd_gain_resistor(void)
{
	// turn on the switch to exclude resistor
	ioport_set_pin_level(gain_select_B2, IOPORT_PIN_LEVEL_HIGH);
}

void enable_afe_3rd_gain_resistor(void)
{
	// turn off the switch to include resistor in series with the first one
	ioport_set_pin_level(gain_select_B3, IOPORT_PIN_LEVEL_LOW);
}

void disable_afe_3rd_gain_resistor(void)
{
	// turn on the switch to exclude resistor
	ioport_set_pin_level(gain_select_B3, IOPORT_PIN_LEVEL_HIGH);
}

void enable_afe_4th_gain_resistor(void)
{
	// turn off the switch to include resistor in series with the first one
	ioport_set_pin_level(gain_select_A6, IOPORT_PIN_LEVEL_LOW);
}

void disable_afe_4th_gain_resistor(void)
{
	// turn on the switch to exclude resistor
	ioport_set_pin_level(gain_select_A6, IOPORT_PIN_LEVEL_HIGH);
}

void enable_afe_5th_gain_resistor(void)
{
	// turn off the switch to include resistor in series with the first one
	ioport_set_pin_level(gain_select_A5, IOPORT_PIN_LEVEL_LOW);
}

void disable_afe_5th_gain_resistor(void)
{
	// turn on the switch to exclude resistor
	ioport_set_pin_level(gain_select_A5, IOPORT_PIN_LEVEL_HIGH);
}

void enable_afe_all_gain_resistors(void)
{
	enable_afe_2nd_gain_resistor();
	enable_afe_3rd_gain_resistor();
	enable_afe_4th_gain_resistor();
	enable_afe_5th_gain_resistor();
}

void disable_afe_all_gain_resistors(void)
{
	disable_afe_2nd_gain_resistor();
	disable_afe_3rd_gain_resistor();
	disable_afe_4th_gain_resistor();
	disable_afe_5th_gain_resistor();
}

void disable_sensor_electrodes(void)
{
	// disable temperature channel
	ioport_set_pin_level(sensor_select_A0, IOPORT_PIN_LEVEL_LOW);

	// disable all electrodes
	ioport_set_pin_level(sensor_select_B7, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B4, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A1, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A2, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A3, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A4, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A7, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B2, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B0, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B1, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B3, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A6, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B5, IOPORT_PIN_LEVEL_LOW);

	ioport_set_pin_level(sensor_select_B6, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A5, IOPORT_PIN_LEVEL_LOW);
}

void enable_ph_electrodes(void)
{
// enable all ph electrodes, disable temp and iref channels
	ioport_set_pin_level(sensor_select_A0, IOPORT_PIN_LEVEL_LOW);

	ioport_set_pin_level(sensor_select_B7, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_B4, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_A1, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_A2, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_A3, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_A4, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_A7, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_B2, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_B0, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_B1, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_B3, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_A6, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(sensor_select_B5, IOPORT_PIN_LEVEL_HIGH);

	ioport_set_pin_level(sensor_select_B6, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A5, IOPORT_PIN_LEVEL_LOW);
}

void enable_single_electrode(uint8_t channel, bool disable_others)
{
	// disable any other electrode that was enabled previously
	if(disable_others)
		disable_sensor_electrodes();

	// enable the selected electrode
	switch(channel)
	{
		//order changed SG/NL 101123
		case 1:
			ioport_set_pin_level(sensor_select_B7,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 2:
			ioport_set_pin_level(sensor_select_B4,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 3:
			ioport_set_pin_level(sensor_select_A4,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 4:
			ioport_set_pin_level(sensor_select_A1,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 5:
			ioport_set_pin_level(sensor_select_B2,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 6:
			ioport_set_pin_level(sensor_select_A2,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 7:
			ioport_set_pin_level(sensor_select_B0,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 8:
			ioport_set_pin_level(sensor_select_B1,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 9:
			ioport_set_pin_level(sensor_select_A3,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 10:
			ioport_set_pin_level(sensor_select_B3,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 11:
			ioport_set_pin_level(sensor_select_A7,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 12:
			ioport_set_pin_level(sensor_select_A6,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 13:
			ioport_set_pin_level(sensor_select_B5,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 14:
			ioport_set_pin_level(sensor_select_B6,  IOPORT_PIN_LEVEL_HIGH);
			break;
		case 15:
			ioport_set_pin_level(sensor_select_A5,  IOPORT_PIN_LEVEL_HIGH);
			break;
	}
}

void enable_multielectrode_scan(bool last_scan)
{
	// disable any other electrode that was enabled previously
	disable_sensor_electrodes();

	int max_in_one_go, preconditioned_so_far = 0;
	int selected_channel;

	if(ocean_scan)
		max_in_one_go = MAX_PRECON_ELECTRODES;
	else
		max_in_one_go = number_of_ph_electrodes;

	for(int i = 0; i < number_of_ph_electrodes; i++)
	{
		if(preconditioned_so_far >= max_in_one_go)
			return;

		selected_channel = 0;

		if(precondition_select[i])
		{
			selected_channel = i + 1;
			if (selected_channel > number_of_ph_electrodes)
				selected_channel -= number_of_ph_electrodes;

			if(last_scan)
				precondition_select[i] = false; //done for this electrode

			preconditioned_so_far++;
		}

		switch(selected_channel)
		{
			case 1:
				ioport_set_pin_level(sensor_select_B7,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 2:
				ioport_set_pin_level(sensor_select_B4,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 3:
				ioport_set_pin_level(sensor_select_A4,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 4:
				ioport_set_pin_level(sensor_select_A1,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 5:
				ioport_set_pin_level(sensor_select_B2,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 6:
				ioport_set_pin_level(sensor_select_A2,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 7:
				ioport_set_pin_level(sensor_select_B0,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 8:
				ioport_set_pin_level(sensor_select_B1,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 9:
				ioport_set_pin_level(sensor_select_A3,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 10:
				ioport_set_pin_level(sensor_select_B3,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 11:
				ioport_set_pin_level(sensor_select_A7,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 12:
				ioport_set_pin_level(sensor_select_A6,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 13:
				ioport_set_pin_level(sensor_select_B5,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 14:
				ioport_set_pin_level(sensor_select_B6,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 15:
				ioport_set_pin_level(sensor_select_A5,  IOPORT_PIN_LEVEL_HIGH);
				break;
		}
	}
}

void enable_electrodes(uint8_t electrode_number_start, uint8_t total_electrodes)
{
	int electrode_number;

	// disable any other electrode that was enabled previously
	disable_sensor_electrodes();

	for(int i = 0; i < total_electrodes; i++)
	{
		electrode_number = electrode_number_start + i;

		if(ph_scan)
			if (electrode_number > number_of_ph_electrodes)
				electrode_number -= number_of_ph_electrodes;

		switch(electrode_number)
		{
			case 1:
				ioport_set_pin_level(sensor_select_B7,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 2:
				ioport_set_pin_level(sensor_select_B4,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 3:
				ioport_set_pin_level(sensor_select_A4,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 4:
				ioport_set_pin_level(sensor_select_A1,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 5:
				ioport_set_pin_level(sensor_select_B2,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 6:
				ioport_set_pin_level(sensor_select_A2,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 7:
				ioport_set_pin_level(sensor_select_B0,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 8:
				ioport_set_pin_level(sensor_select_B1,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 9:
				ioport_set_pin_level(sensor_select_A3,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 10:
				ioport_set_pin_level(sensor_select_B3,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 11:
				ioport_set_pin_level(sensor_select_A7,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 12:
				ioport_set_pin_level(sensor_select_A6,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 13:
				ioport_set_pin_level(sensor_select_B5,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 14:
				ioport_set_pin_level(sensor_select_B6,  IOPORT_PIN_LEVEL_HIGH);
				break;
			case 15:
				ioport_set_pin_level(sensor_select_A5,  IOPORT_PIN_LEVEL_HIGH);
				break;
		}
	}
}

void enable_temperature_switch(void)
{
	// disable any other electrode that was enabled previously
	disable_sensor_electrodes();

	// enable the switch for reading temperature
	ioport_set_pin_level(sensor_select_A0,  IOPORT_PIN_LEVEL_HIGH);
}

