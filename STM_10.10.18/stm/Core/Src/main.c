/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include <scan.h>
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart_commsboard.h"
#include "ioport.h"
#include "subsystem.h"
#include "init.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM14_Init(void);
static void MX_TIM16_Init(void);
static void MX_TIM17_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM6_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t rxData[COMMAND_MAXIMUM_SIZE];
uint8_t mainData[COMMAND_MAXIMUM_SIZE];

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  //note that timer priorities are in group 1 to allow HAL_Delay() to run normally with priority 0
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_TIM14_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
  MX_TIM7_Init();
  MX_TIM6_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  board_init();
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxData, sizeof(rxData));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (perform_fresh_measurement_routine_flag)
	  {
		  if(!already_scanning)
		  {
			  already_scanning = true;
			  scan(SCAN_TYPE_FRESH);
			  perform_fresh_measurement_routine_flag = false;
			  scan_internal_interval_flag = false;
			  scan_interval_flag = false;
			  scan_short_interval_flag = false;
			  scan_first_flag = false;
			  already_scanning = false;
		  }
	  }
	  if (perform_ocean_measurement_routine_flag)
	  {
		  if(!already_scanning)
		  {
			  already_scanning = true;
			  scan(SCAN_TYPE_OCEAN);
			  perform_ocean_measurement_routine_flag = false;
			  scan_internal_interval_flag = false;
			  scan_interval_flag = false;
			  scan_short_interval_flag = false;
			  scan_first_flag = false;
			  already_scanning = false;
		  }
	  }
	  if (perform_vf_measurement_routine_flag)
	  {
		  if(!already_scanning)
		  {
			  already_scanning = true;
			  scan(SCAN_TYPE_VF);
			  perform_vf_measurement_routine_flag = false;
			  scan_internal_interval_flag = false;
			  scan_interval_flag = false;
			  scan_short_interval_flag = false;
			  scan_first_flag = false;
			  already_scanning = false;
		  }
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00707CBB;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_SLAVE;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_INPUT;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 256 - 1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65535;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 256 - 1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 256 - 1;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 65535;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 256 - 1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 65535;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(void)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 256 - 1;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 65535;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_RS485Ex_Init(&huart1, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, POWER_GROUND_IN4_CTR_TO_F_Pin|DAC_CHIP_SELECT_Pin|IM_ADC_Pin|SENSOR_SELECT_A0_Pin
                          |SENSOR_SELECT_A1_Pin|SENSOR_SELECT_A2_Pin|SENSOR_SELECT_A3_Pin|SENSOR_SELECT_A4_Pin
                          |SENSOR_SELECT_A5_Pin|SENSOR_SELECT_A6_Pin|SENSOR_SELECT_A7_Pin|POWER_GROUND_IN1_REF_TO_A_Pin
                          |POWER_GROUND_IN3_CTR_TO_CTR_DRV_Pin|POWER_GROUND_IN2_B_TO_A_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, UART_RXD0_DEBUG_Pin|MCU_DAC_OUT_2_Pin|ADC_OPTIONAL_SYSTEM_CLOCK_Pin|ADC_SYNC_PD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, POWER_PSU_2V5_ENABLE_Pin|SENSOR_SELECT_B2_Pin|SENSOR_SELECT_B3_Pin|SENSOR_SELECT_B4_Pin
                          |SENSOR_SELECT_B5_Pin|SENSOR_SELECT_B6_Pin|SENSOR_SELECT_B7_Pin|ADC_CHIP_SELECT_Pin
                          |EEPROM_CHIP_SELECT_Pin|SENSOR_SELECT_B0_Pin|SENSOR_SELECT_B1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GAIN_SELECT_A5_Pin|GAIN_SELECT_A6_Pin|POWER_5V_ANALOG_ENABLE_Pin|WATER_DETECT_ENABLE_Pin
                          |GAIN_SELECT_B2_Pin|GAIN_SELECT_B3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : POWER_GROUND_IN4_CTR_TO_F_Pin DAC_CHIP_SELECT_Pin IM_ADC_Pin SENSOR_SELECT_A0_Pin
                           SENSOR_SELECT_A1_Pin SENSOR_SELECT_A2_Pin SENSOR_SELECT_A3_Pin SENSOR_SELECT_A4_Pin
                           SENSOR_SELECT_A5_Pin SENSOR_SELECT_A6_Pin SENSOR_SELECT_A7_Pin POWER_GROUND_IN1_REF_TO_A_Pin
                           POWER_GROUND_IN3_CTR_TO_CTR_DRV_Pin POWER_GROUND_IN2_B_TO_A_Pin */
  GPIO_InitStruct.Pin = POWER_GROUND_IN4_CTR_TO_F_Pin|DAC_CHIP_SELECT_Pin|IM_ADC_Pin|SENSOR_SELECT_A0_Pin
                          |SENSOR_SELECT_A1_Pin|SENSOR_SELECT_A2_Pin|SENSOR_SELECT_A3_Pin|SENSOR_SELECT_A4_Pin
                          |SENSOR_SELECT_A5_Pin|SENSOR_SELECT_A6_Pin|SENSOR_SELECT_A7_Pin|POWER_GROUND_IN1_REF_TO_A_Pin
                          |POWER_GROUND_IN3_CTR_TO_CTR_DRV_Pin|POWER_GROUND_IN2_B_TO_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : AWAKE_DEEP_SLEEP_Pin UART_TXD0_DEBUG_Pin */
  GPIO_InitStruct.Pin = AWAKE_DEEP_SLEEP_Pin|UART_TXD0_DEBUG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : UART_RXD0_DEBUG_Pin MCU_DAC_OUT_2_Pin ADC_OPTIONAL_SYSTEM_CLOCK_Pin ADC_SYNC_PD_Pin */
  GPIO_InitStruct.Pin = UART_RXD0_DEBUG_Pin|MCU_DAC_OUT_2_Pin|ADC_OPTIONAL_SYSTEM_CLOCK_Pin|ADC_SYNC_PD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : MCU_DAC_OUT_1_Pin */
  GPIO_InitStruct.Pin = MCU_DAC_OUT_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MCU_DAC_OUT_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : POWER_PSU_2V5_ENABLE_Pin SENSOR_SELECT_B2_Pin SENSOR_SELECT_B3_Pin SENSOR_SELECT_B4_Pin
                           SENSOR_SELECT_B5_Pin SENSOR_SELECT_B6_Pin SENSOR_SELECT_B7_Pin ADC_CHIP_SELECT_Pin
                           EEPROM_CHIP_SELECT_Pin SENSOR_SELECT_B0_Pin SENSOR_SELECT_B1_Pin */
  GPIO_InitStruct.Pin = POWER_PSU_2V5_ENABLE_Pin|SENSOR_SELECT_B2_Pin|SENSOR_SELECT_B3_Pin|SENSOR_SELECT_B4_Pin
                          |SENSOR_SELECT_B5_Pin|SENSOR_SELECT_B6_Pin|SENSOR_SELECT_B7_Pin|ADC_CHIP_SELECT_Pin
                          |EEPROM_CHIP_SELECT_Pin|SENSOR_SELECT_B0_Pin|SENSOR_SELECT_B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : GAIN_SELECT_A5_Pin GAIN_SELECT_A6_Pin POWER_5V_ANALOG_ENABLE_Pin WATER_DETECT_ENABLE_Pin
                           GAIN_SELECT_B2_Pin GAIN_SELECT_B3_Pin */
  GPIO_InitStruct.Pin = GAIN_SELECT_A5_Pin|GAIN_SELECT_A6_Pin|POWER_5V_ANALOG_ENABLE_Pin|WATER_DETECT_ENABLE_Pin
                          |GAIN_SELECT_B2_Pin|GAIN_SELECT_B3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : ADC_DATA_READY_Pin */
  GPIO_InitStruct.Pin = ADC_DATA_READY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC_DATA_READY_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  UNUSED(Size);

  memcpy(mainData, rxData, Size);

  check_message(mainData, sizeof(mainData));

  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxData, sizeof(rxData));
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UARTEx_RxEventCallback can be implemented in the user file.
   */
}

void uart1_send_byte(uint8_t byte)
{
	HAL_UART_Transmit(&huart1, &byte, sizeof(byte), 100);
}

void uart1_send_bytes(uint8_t bytes[], uint8_t bytes_to_send)
{
	HAL_UART_Transmit(&huart1, bytes, bytes_to_send, 100);
}

void spi1_enable(void)
{
	__HAL_SPI_ENABLE(&hspi1);
}

void spi1_send_byte(uint8_t byte)
{
	while( !(SPI1->SR & (SPI_SR_TXE)) );  // wait until transmit buffer empty
	*(__IO uint8_t *)&SPI1->DR = byte; // send data
}

//simplified function to send one byte in spi mode 0
void spi2_send_byte(uint8_t byte)
{
	__HAL_SPI_DISABLE(&hspi2);
	hspi2.Instance->CR1 &= ~(SPI_CR1_CPHA);       // Clear CPHA bit
	hspi2.Instance->CR1 |= SPI_PHASE_1EDGE;       // Set CPHA bit for 1st edge
	__HAL_SPI_ENABLE(&hspi2);

	while( !(SPI2->SR & (SPI_SR_TXE)) );  // wait until transmit buffer empty
	*(__IO uint8_t *)&SPI2->DR = byte; // send data
	while (((SPI2->SR)&(SPI_SR_BSY))) {};
}

void spi2_send_bytes(uint8_t num_bytes, uint8_t bytes[], unsigned long clock_edge)
{
	__HAL_SPI_DISABLE(&hspi2);
	hspi2.Instance->CR1 &= ~(SPI_CR1_CPHA);       // Clear CPHA bit
	hspi2.Instance->CR1 |= clock_edge;       // Set CPHA bit for specified edge
	__HAL_SPI_ENABLE(&hspi2);

	for(int i = 0; i < num_bytes; i++)
	{
		while( !(SPI2->SR & (SPI_SR_TXE)) );  // wait until transmit buffer empty
		while (((SPI2->SR)&(SPI_SR_BSY))) {};
		*(__IO uint8_t *)&SPI2->DR = bytes[i]; // send data
	}
	while(!(SPI2->SR & (SPI_SR_TXE)));	//wait until transmit buffer empty
	while ((SPI2->SR)&(SPI_SR_BSY));	//wait until final byte sent
}

//simplified function to return a one byte read in spi mode 0
uint8_t spi2_get_byte()
{
	uint8_t byte;

	__HAL_SPI_DISABLE(&hspi2);
	hspi2.Instance->CR1 &= ~(SPI_CR1_CPHA);
	hspi2.Instance->CR1 |= SPI_PHASE_1EDGE;
	__HAL_SPI_ENABLE(&hspi2);

	HAL_SPIEx_FlushRxFifo(&hspi2);

	while( !(SPI2->SR & SPI_SR_TXE) );  // wait until transmit buffer empty
	*(__IO uint8_t *)&SPI2->DR = 0x00; // send dummy data

	while(!(SPI2->SR & SPI_SR_RXNE));

	byte = SPI2->DR;

	return byte;
}


void spi2_get_bytes(uint8_t num_bytes, uint8_t* bytes, unsigned long clock_edge)
{
	HAL_SPIEx_FlushRxFifo(&hspi2);

	__HAL_SPI_DISABLE(&hspi2);
	hspi2.Instance->CR1 &= ~(SPI_CR1_CPHA);       // Clear CPHA bit
	hspi2.Instance->CR1 |= clock_edge;       // Set CPHA bit for 2nd edge
	__HAL_SPI_ENABLE(&hspi2);

	for(int i = 0; i < num_bytes; i++)
	{
		while( !(SPI2->SR & SPI_SR_TXE) );  // wait until transmit buffer empty
		*(__IO uint8_t *)&SPI2->DR = 0; // send dummy data
		while( !(SPI2->SR & SPI_SR_RXNE));
		bytes[i] = SPI2->DR;
	}

	return;
}

void spi2_wait_for_eeprom_write_to_complete()
{
	int count = 0;
	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_LOW);
	spi2_send_byte(0x05); //read status register
	spi2_get_byte(); //to clear the byte resulting from from the command. check if this is necessary

	uint8_t reg = spi2_get_byte(); //read the status register
	while((reg & 1) == 1) //while write in progress, keep reading
	{
		count++;
		reg = spi2_get_byte();
	}

	ioport_set_pin_level(eeprom_chip_select, IOPORT_PIN_LEVEL_HIGH);

	return;
}

void start_timer(uint8_t timer_number)
{
	if(timer_number == 6) //SCAN_DAC_POSITIVE_CONDITIONING_TIMER
	{
		__HAL_TIM_CLEAR_IT(&htim6, TIM_IT_UPDATE); //to stop interrupt firing immediately
		HAL_TIM_Base_Start_IT(&htim6);
	}
	if(timer_number == 7) //SCAN_DAC_POSITIVE_TIMER
	{
		__HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE); //to stop interrupt firing immediately
		HAL_TIM_Base_Start_IT(&htim7);
	}
	if(timer_number == 14) //SCAN_DAC_NEGATIVE_TIMER
	{
		__HAL_TIM_CLEAR_IT(&htim14, TIM_IT_UPDATE); //to stop interrupt firing immediately
		HAL_TIM_Base_Start_IT(&htim14);
	}
	if(timer_number == 16) //SCAN_ADC_MEASUREMENT_POSITIVE_TIMER
	{
		__HAL_TIM_CLEAR_IT(&htim16, TIM_IT_UPDATE); //to stop interrupt firing immediately
		HAL_TIM_Base_Start_IT(&htim16);
	}
	if(timer_number == 17) //SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER
	{
		__HAL_TIM_CLEAR_IT(&htim17, TIM_IT_UPDATE); //to stop interrupt firing immediately
		HAL_TIM_Base_Start_IT(&htim17);
	}
}

void stop_timer(uint8_t timer_number)
{
	if(timer_number == 6) //SCAN_DAC_POSITIVE_CONDITIONING_TIMER
		HAL_TIM_Base_Stop_IT(&htim6);
	if(timer_number == 7) //SCAN_DAC_POSITIVE_TIMER
		HAL_TIM_Base_Stop_IT(&htim7);
	if(timer_number == 14) //SCAN_DAC_NEGATIVE_TIMER
		HAL_TIM_Base_Stop_IT(&htim14);
	if(timer_number == 16) //SCAN_ADC_MEASUREMENT_POSITIVE_TIMER
		HAL_TIM_Base_Stop_IT(&htim16);
	if(timer_number == 17) //SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER
		HAL_TIM_Base_Stop_IT(&htim17);
}

void set_timer_period(uint8_t timer_number, uint16_t timer_overflow_count)
{
	if(timer_number == 6) //SCAN_DAC_POSITIVE_CONDITIONING_TIMER
		htim6.Instance->ARR = timer_overflow_count;
	if(timer_number == 7) //SCAN_DAC_POSITIVE_TIMER
		htim7.Instance->ARR = timer_overflow_count;
	if(timer_number == 14) //SCAN_DAC_NEGATIVE_TIMER
		htim14.Instance->ARR = timer_overflow_count;
	if(timer_number == 16) //SCAN_ADC_MEASUREMENT_POSITIVE_TIMER
		htim16.Instance->ARR = timer_overflow_count;
	if(timer_number == 17) //SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER
		htim17.Instance->ARR = timer_overflow_count;
}

void set_timer_count(uint8_t timer_number, uint16_t timer_count)
{
	if(timer_number == 6) //SCAN_DAC_POSITIVE_CONDITIONING_TIMER
		htim6.Instance->CNT = timer_count;
	if(timer_number == 7) //SCAN_DAC_POSITIVE_TIMER
		htim7.Instance->CNT = timer_count;
	if(timer_number == 14) //SCAN_DAC_NEGATIVE_TIMER
		htim14.Instance->CNT = timer_count;
	if(timer_number == 16) //SCAN_ADC_MEASUREMENT_POS ITIVE_TIMER
		htim16.Instance->CNT = timer_count;
	if(timer_number == 17) //SCAN_ADC_MEASUREMENT_NEGATIVE_TIMER
		htim17.Instance->CNT = timer_count;
}

void cpu_irq_enable()
{
	__enable_irq();
}

void cpu_irq_disable()
{
	__disable_irq();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	if(htim == &htim6)
	{
		conditioning_timer_loop();
	}

	if(htim == &htim7)
	{
		dac_positive_scan_cycle_processor();
	}

	if(htim == &htim14)
	{
		dac_negative_scan_cycle_processor();
	}

	if(htim == &htim16)
	{
		adc_positive_scan_cycle_processor();
	}

	if(htim == &htim17)
	{
		adc_negative_scan_cycle_processor();
	}

}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
