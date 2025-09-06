/*
 * init.c
 *
 *  Created on: Mar 9, 2023
 *      Author: phil31419
 */

#include "ioport.h"

void board_init(void)
{
	// power to analog front end
	ioport_set_pin_level(power_5v_analog_enable, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(power_psu_2v5_enable, IOPORT_PIN_LEVEL_LOW);

	// set the dac drive to the sensor low initially until needed
	ioport_set_pin_level(power_ground_in3_ctr_to_ctr_drv, IOPORT_PIN_LEVEL_LOW);
	// open switch for ref to 3.3V
	ioport_set_pin_level(power_ground_in1_ref_to_a, IOPORT_PIN_LEVEL_LOW);
	// open counter to ground connection
	ioport_set_pin_level(power_ground_in4_ctr_to_f, IOPORT_PIN_LEVEL_LOW);

	ioport_set_pin_level(power_ground_in2_b_to_a, IOPORT_PIN_LEVEL_LOW);


	// set all sensor enable pins to low
	ioport_set_pin_level(sensor_select_A0, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A1, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A2, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A3, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A4, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A5, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A6, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_A7, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B0, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B1, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B2, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B3, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B4, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B5, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B6, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(sensor_select_B7, IOPORT_PIN_LEVEL_LOW);

	//deselect all spi devices
	ioport_set_pin_level(dac_chip_select, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(adc_chip_select, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_HIGH);

	//disable all gain resistors
	ioport_set_pin_level(gain_select_B2, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(gain_select_B3, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(gain_select_A5, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(gain_select_A6, IOPORT_PIN_LEVEL_HIGH);

	//immersion switches
	ioport_set_pin_level(im_adc, IOPORT_PIN_LEVEL_HIGH);

	ioport_set_pin_level(water_detect_enable, IOPORT_PIN_LEVEL_LOW);

	ioport_set_pin_level(spi_slave_data_ready_flag, IOPORT_PIN_LEVEL_HIGH);
}
