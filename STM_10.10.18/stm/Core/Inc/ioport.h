/*
 * ioport.h
 *
 *  Created on: Mar 9, 2023
 *      Author: phil31419
 */

#ifndef INC_IOPORT_H_
#define INC_IOPORT_H_

#include <stdint.h>
#include "main.h"
#include "stm32g0xx_hal.h"

enum ioport_value {
	IOPORT_PIN_LEVEL_LOW,  /*!< IOPORT pin value low */
	IOPORT_PIN_LEVEL_HIGH, /*!< IOPORT pin value high */
};

typedef struct GPIOPin
{
    GPIO_TypeDef *port;
    uint16_t pin;
} GPIOPin;

extern volatile GPIOPin sensor_select_A0;
extern volatile GPIOPin sensor_select_A1;
extern volatile GPIOPin sensor_select_A2;
extern volatile GPIOPin sensor_select_A3;
extern volatile GPIOPin sensor_select_A4;
extern volatile GPIOPin sensor_select_A5;
extern volatile GPIOPin sensor_select_A6;
extern volatile GPIOPin sensor_select_A7;

extern volatile GPIOPin sensor_select_B0;
extern volatile GPIOPin sensor_select_B1;
extern volatile GPIOPin sensor_select_B2;
extern volatile GPIOPin sensor_select_B3;
extern volatile GPIOPin sensor_select_B4;
extern volatile GPIOPin sensor_select_B5;
extern volatile GPIOPin sensor_select_B6;
extern volatile GPIOPin sensor_select_B7;

extern volatile GPIOPin water_detect_enable;

extern volatile GPIOPin awake_deep_sleep;

extern volatile GPIOPin gain_select_B2;
extern volatile GPIOPin gain_select_B3;
extern volatile GPIOPin gain_select_A5;
extern volatile GPIOPin gain_select_A6;

extern volatile GPIOPin spi_slave_data_ready_flag;
extern volatile GPIOPin spi_slave_data_already_read_flag;

extern volatile GPIOPin i2c_sda_slave;
extern volatile GPIOPin i2c_scl_slave;

extern volatile GPIOPin uart_rxd0_mod_bus;
extern volatile GPIOPin uart_txd0_mod_bus;

extern volatile GPIOPin spi_slave_chip_select;
extern volatile GPIOPin spi_slave_mosi;
extern volatile GPIOPin spi_slave_miso;
extern volatile GPIOPin spi_slave_sclk;

extern volatile GPIOPin power_5v_analog_enable;
extern volatile GPIOPin power_psu_2v5_enable;

extern volatile GPIOPin uart_rxd0_debug;
extern volatile GPIOPin uart_txd0_debug;

extern volatile GPIOPin dac_chip_select;

extern volatile GPIOPin spi_d_mosi;
extern volatile GPIOPin spi_d_miso;
extern volatile GPIOPin spi_d_sclk;

extern volatile GPIOPin power_ground_in1_ref_to_a;
extern volatile GPIOPin power_ground_in2_b_to_a;
extern volatile GPIOPin power_ground_in3_ctr_to_ctr_drv;
extern volatile GPIOPin power_ground_in4_ctr_to_f;

extern volatile GPIOPin im_adc;

extern volatile GPIOPin adc_data_ready;

extern volatile GPIOPin adc_chip_select;

extern volatile GPIOPin adc_sync_pd;

extern volatile GPIOPin adc_optional_system_clock;

extern volatile GPIOPin eeprom_chip_select;

extern volatile GPIOPin mcu_dac_out_1;
extern volatile GPIOPin mcu_dac_out_2;

void ioport_set_pin_level(GPIOPin Pin, GPIO_PinState PinState);
bool ioport_get_pin_level(GPIOPin Pin);

#endif /* INC_IOPORT_H_ */
