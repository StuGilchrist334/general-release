/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <string.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */


/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void uart1_send_byte(uint8_t byte);
void uart1_send_bytes(uint8_t bytes[], uint8_t bytes_to_send);
void spi1_enable(void);
void spi1_send_byte(uint8_t byte);
void spi2_send_byte(uint8_t byte);
void spi2_send_bytes(uint8_t num_bytes, uint8_t bytes[], unsigned long clock_edge);
uint8_t spi2_get_byte(void);
void spi2_get_bytes(uint8_t num_bytes, uint8_t* bytes, unsigned long clock_edge);
void spi2_wait_for_eeprom_write_to_complete();
void start_timer(uint8_t timer_number);
void stop_timer(uint8_t timer_number);
void set_timer_period(uint8_t timer_number, uint16_t timer_count);
void set_timer_count(uint8_t timer_number, uint16_t timer_count);
void cpu_irq_enable(void);
void cpu_irq_disable(void);
uint8_t flash_read_value(uint8_t array_position);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define POWER_GROUND_IN4_CTR_TO_F_Pin GPIO_PIN_11
#define POWER_GROUND_IN4_CTR_TO_F_GPIO_Port GPIOC
#define DAC_CHIP_SELECT_Pin GPIO_PIN_12
#define DAC_CHIP_SELECT_GPIO_Port GPIOC
#define IM_ADC_Pin GPIO_PIN_13
#define IM_ADC_GPIO_Port GPIOC
#define SENSOR_SELECT_A0_Pin GPIO_PIN_0
#define SENSOR_SELECT_A0_GPIO_Port GPIOC
#define SENSOR_SELECT_A1_Pin GPIO_PIN_1
#define SENSOR_SELECT_A1_GPIO_Port GPIOC
#define SENSOR_SELECT_A2_Pin GPIO_PIN_2
#define SENSOR_SELECT_A2_GPIO_Port GPIOC
#define SENSOR_SELECT_A3_Pin GPIO_PIN_3
#define SENSOR_SELECT_A3_GPIO_Port GPIOC
#define AWAKE_DEEP_SLEEP_Pin GPIO_PIN_0
#define AWAKE_DEEP_SLEEP_GPIO_Port GPIOA
#define SPI_SLAVE_SCLK_Pin GPIO_PIN_1
#define SPI_SLAVE_SCLK_GPIO_Port GPIOA
#define UART_TXD0_DEBUG_Pin GPIO_PIN_2
#define UART_TXD0_DEBUG_GPIO_Port GPIOA
#define UART_RXD0_DEBUG_Pin GPIO_PIN_3
#define UART_RXD0_DEBUG_GPIO_Port GPIOA
#define MCU_DAC_OUT_1_Pin GPIO_PIN_4
#define MCU_DAC_OUT_1_GPIO_Port GPIOA
#define MCU_DAC_OUT_2_Pin GPIO_PIN_5
#define MCU_DAC_OUT_2_GPIO_Port GPIOA
#define SPI_MISO_MASTER_SLAVE_Pin GPIO_PIN_6
#define SPI_MISO_MASTER_SLAVE_GPIO_Port GPIOA
#define SENSOR_SELECT_A4_Pin GPIO_PIN_4
#define SENSOR_SELECT_A4_GPIO_Port GPIOC
#define SENSOR_SELECT_A5_Pin GPIO_PIN_5
#define SENSOR_SELECT_A5_GPIO_Port GPIOC
#define POWER_PSU_2V5_ENABLE_Pin GPIO_PIN_2
#define POWER_PSU_2V5_ENABLE_GPIO_Port GPIOB
#define SENSOR_SELECT_B2_Pin GPIO_PIN_10
#define SENSOR_SELECT_B2_GPIO_Port GPIOB
#define SENSOR_SELECT_B3_Pin GPIO_PIN_11
#define SENSOR_SELECT_B3_GPIO_Port GPIOB
#define SENSOR_SELECT_B4_Pin GPIO_PIN_12
#define SENSOR_SELECT_B4_GPIO_Port GPIOB
#define SENSOR_SELECT_B5_Pin GPIO_PIN_13
#define SENSOR_SELECT_B5_GPIO_Port GPIOB
#define SENSOR_SELECT_B6_Pin GPIO_PIN_14
#define SENSOR_SELECT_B6_GPIO_Port GPIOB
#define SENSOR_SELECT_B7_Pin GPIO_PIN_15
#define SENSOR_SELECT_B7_GPIO_Port GPIOB
#define ADC_OPTIONAL_SYSTEM_CLOCK_Pin GPIO_PIN_8
#define ADC_OPTIONAL_SYSTEM_CLOCK_GPIO_Port GPIOA
#define UART_TXD0_MOD_BUS_Pin GPIO_PIN_9
#define UART_TXD0_MOD_BUS_GPIO_Port GPIOA
#define SENSOR_SELECT_A6_Pin GPIO_PIN_6
#define SENSOR_SELECT_A6_GPIO_Port GPIOC
#define SENSOR_SELECT_A7_Pin GPIO_PIN_7
#define SENSOR_SELECT_A7_GPIO_Port GPIOC
#define GAIN_SELECT_A5_Pin GPIO_PIN_8
#define GAIN_SELECT_A5_GPIO_Port GPIOD
#define GAIN_SELECT_A6_Pin GPIO_PIN_9
#define GAIN_SELECT_A6_GPIO_Port GPIOD
#define UART_RXD0_MOD_BUS_Pin GPIO_PIN_10
#define UART_RXD0_MOD_BUS_GPIO_Port GPIOA
#define ADC_SYNC_PD_Pin GPIO_PIN_11
#define ADC_SYNC_PD_GPIO_Port GPIOA
#define SPI_MASTER_SLAVE_CHIP_SELECT_Pin GPIO_PIN_15
#define SPI_MASTER_SLAVE_CHIP_SELECT_GPIO_Port GPIOA
#define POWER_GROUND_IN1_REF_TO_A_Pin GPIO_PIN_8
#define POWER_GROUND_IN1_REF_TO_A_GPIO_Port GPIOC
#define POWER_GROUND_IN3_CTR_TO_CTR_DRV_Pin GPIO_PIN_9
#define POWER_GROUND_IN3_CTR_TO_CTR_DRV_GPIO_Port GPIOC
#define POWER_5V_ANALOG_ENABLE_Pin GPIO_PIN_0
#define POWER_5V_ANALOG_ENABLE_GPIO_Port GPIOD
#define SPI_D_SCLK_Pin GPIO_PIN_1
#define SPI_D_SCLK_GPIO_Port GPIOD
#define WATER_DETECT_ENABLE_Pin GPIO_PIN_2
#define WATER_DETECT_ENABLE_GPIO_Port GPIOD
#define SPI_D_MISO_Pin GPIO_PIN_3
#define SPI_D_MISO_GPIO_Port GPIOD
#define SPI_D_MOSI_Pin GPIO_PIN_4
#define SPI_D_MOSI_GPIO_Port GPIOD
#define GAIN_SELECT_B2_Pin GPIO_PIN_5
#define GAIN_SELECT_B2_GPIO_Port GPIOD
#define GAIN_SELECT_B3_Pin GPIO_PIN_6
#define GAIN_SELECT_B3_GPIO_Port GPIOD
#define ADC_DATA_READY_Pin GPIO_PIN_3
#define ADC_DATA_READY_GPIO_Port GPIOB
#define ADC_CHIP_SELECT_Pin GPIO_PIN_4
#define ADC_CHIP_SELECT_GPIO_Port GPIOB
#define EEPROM_CHIP_SELECT_Pin GPIO_PIN_5
#define EEPROM_CHIP_SELECT_GPIO_Port GPIOB
#define I2C_SDA_SLAVE_Pin GPIO_PIN_6
#define I2C_SDA_SLAVE_GPIO_Port GPIOB
#define I2C_SCL_SLAVE_Pin GPIO_PIN_7
#define I2C_SCL_SLAVE_GPIO_Port GPIOB
#define SENSOR_SELECT_B0_Pin GPIO_PIN_8
#define SENSOR_SELECT_B0_GPIO_Port GPIOB
#define SENSOR_SELECT_B1_Pin GPIO_PIN_9
#define SENSOR_SELECT_B1_GPIO_Port GPIOB
#define POWER_GROUND_IN2_B_TO_A_Pin GPIO_PIN_10
#define POWER_GROUND_IN2_B_TO_A_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */
// Slave SPI data ready flag
//#define SPI_SLAVE_DATA_READY_FLAG			UART_RXD0_DEBUG

// Slave SPI read data flag
//#define SPI_SLAVE_DATA_ALREADY_READ_FLAG	UART_TXD0_DEBUG
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
