/*
 * ioport.c
 *
 *  Created on: Mar 9, 2023
 *      Author: phil31419
 */

#include "ioport.h"

volatile GPIOPin sensor_select_A0 = {SENSOR_SELECT_A0_GPIO_Port, SENSOR_SELECT_A0_Pin};
volatile GPIOPin sensor_select_A1 = {SENSOR_SELECT_A1_GPIO_Port, SENSOR_SELECT_A1_Pin};
volatile GPIOPin sensor_select_A2 = {SENSOR_SELECT_A2_GPIO_Port, SENSOR_SELECT_A2_Pin};
volatile GPIOPin sensor_select_A3 = {SENSOR_SELECT_A3_GPIO_Port, SENSOR_SELECT_A3_Pin};
volatile GPIOPin sensor_select_A4 = {SENSOR_SELECT_A4_GPIO_Port, SENSOR_SELECT_A4_Pin};
volatile GPIOPin sensor_select_A5 = {SENSOR_SELECT_A5_GPIO_Port, SENSOR_SELECT_A5_Pin};
volatile GPIOPin sensor_select_A6 = {SENSOR_SELECT_A6_GPIO_Port, SENSOR_SELECT_A6_Pin};
volatile GPIOPin sensor_select_A7 = {SENSOR_SELECT_A7_GPIO_Port, SENSOR_SELECT_A7_Pin};

volatile GPIOPin sensor_select_B0 = {SENSOR_SELECT_B0_GPIO_Port, SENSOR_SELECT_B0_Pin};
volatile GPIOPin sensor_select_B1 = {SENSOR_SELECT_B1_GPIO_Port, SENSOR_SELECT_B1_Pin};
volatile GPIOPin sensor_select_B2 = {SENSOR_SELECT_B2_GPIO_Port, SENSOR_SELECT_B2_Pin};
volatile GPIOPin sensor_select_B3 = {SENSOR_SELECT_B3_GPIO_Port, SENSOR_SELECT_B3_Pin};
volatile GPIOPin sensor_select_B4 = {SENSOR_SELECT_B4_GPIO_Port, SENSOR_SELECT_B4_Pin};
volatile GPIOPin sensor_select_B5 = {SENSOR_SELECT_B5_GPIO_Port, SENSOR_SELECT_B5_Pin};
volatile GPIOPin sensor_select_B6 = {SENSOR_SELECT_B6_GPIO_Port, SENSOR_SELECT_B6_Pin};
volatile GPIOPin sensor_select_B7 = {SENSOR_SELECT_B7_GPIO_Port, SENSOR_SELECT_B7_Pin};

volatile GPIOPin gain_select_B2 = {GAIN_SELECT_B2_GPIO_Port, GAIN_SELECT_B2_Pin};
volatile GPIOPin gain_select_B3 = {GAIN_SELECT_B3_GPIO_Port, GAIN_SELECT_B3_Pin};
volatile GPIOPin gain_select_A6 = {GAIN_SELECT_A6_GPIO_Port, GAIN_SELECT_A6_Pin};
volatile GPIOPin gain_select_A5 = {GAIN_SELECT_A5_GPIO_Port, GAIN_SELECT_A5_Pin};

volatile GPIOPin spi_slave_data_ready_flag = {UART_RXD0_DEBUG_GPIO_Port, UART_RXD0_DEBUG_Pin};
volatile GPIOPin spi_slave_data_already_read_flag = {UART_TXD0_DEBUG_GPIO_Port, UART_TXD0_DEBUG_Pin};

volatile GPIOPin power_5v_analog_enable = {POWER_5V_ANALOG_ENABLE_GPIO_Port, POWER_5V_ANALOG_ENABLE_Pin};
volatile GPIOPin power_psu_2v5_enable = {POWER_PSU_2V5_ENABLE_GPIO_Port, POWER_PSU_2V5_ENABLE_Pin};

volatile GPIOPin power_ground_in1_ref_to_a = {POWER_GROUND_IN1_REF_TO_A_GPIO_Port, POWER_GROUND_IN1_REF_TO_A_Pin};
volatile GPIOPin power_ground_in2_b_to_a = {POWER_GROUND_IN2_B_TO_A_GPIO_Port, POWER_GROUND_IN2_B_TO_A_Pin};
volatile GPIOPin power_ground_in3_ctr_to_ctr_drv = {POWER_GROUND_IN3_CTR_TO_CTR_DRV_GPIO_Port, POWER_GROUND_IN3_CTR_TO_CTR_DRV_Pin};
volatile GPIOPin power_ground_in4_ctr_to_f = {POWER_GROUND_IN4_CTR_TO_F_GPIO_Port, POWER_GROUND_IN4_CTR_TO_F_Pin};

volatile GPIOPin im_adc = {IM_ADC_GPIO_Port, IM_ADC_Pin};
volatile GPIOPin water_detect_enable = {WATER_DETECT_ENABLE_GPIO_Port, WATER_DETECT_ENABLE_Pin};

volatile GPIOPin mcu_dac_out_1 = {MCU_DAC_OUT_1_GPIO_Port, MCU_DAC_OUT_1_Pin};
volatile GPIOPin mcu_dac_out_2 = {MCU_DAC_OUT_2_GPIO_Port, MCU_DAC_OUT_2_Pin};

volatile GPIOPin adc_data_ready = {ADC_DATA_READY_GPIO_Port, ADC_DATA_READY_Pin};
volatile GPIOPin adc_sync_pd = {ADC_SYNC_PD_GPIO_Port, ADC_SYNC_PD_Pin};

volatile GPIOPin dac_chip_select = {DAC_CHIP_SELECT_GPIO_Port, DAC_CHIP_SELECT_Pin};
volatile GPIOPin adc_chip_select = {ADC_CHIP_SELECT_GPIO_Port, ADC_CHIP_SELECT_Pin};
volatile GPIOPin eeprom_chip_select = {EEPROM_CHIP_SELECT_GPIO_Port, EEPROM_CHIP_SELECT_Pin};

void ioport_set_pin_level(GPIOPin Pin, GPIO_PinState PinState)
{
	HAL_GPIO_WritePin(Pin.port, Pin.pin, PinState);
}

bool ioport_get_pin_level(GPIOPin Pin)
{
	return HAL_GPIO_ReadPin(Pin.port, Pin.pin);
}
