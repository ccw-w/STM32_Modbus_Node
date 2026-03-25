/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "font.h"
#include <stdio.h>
#include <string.h>
#include "dht11.h"
#include "modbus.h"
#include "CRC.h"
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

/* USER CODE BEGIN PV */
volatile uint8_t Modbus_Frame_Flag = 0;
volatile uint16_t Modbus_RX_Length = 0;

DHT11_Data_TypeDef DHT11_Data;

uint16_t g_adc_value = 0;
float g_adc_voltage = 0.0f;

uint32_t g_last_sample_tick = 0;
uint32_t g_last_oled_tick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Device_Data_Update(void);
void Device_Control_Update(void);
void Device_Alarm_Update(void);
void OLED_Display_Update(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Modbus_RX_Length = 8;
        Modbus_Frame_Flag = 1;
    }
}

void Device_Data_Update(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    g_adc_value = HAL_ADC_GetValue(&hadc1);
    g_adc_voltage = (g_adc_value / 4095.0f) * 3.3f;

    Modbus_Register[REG_ADC_RAW] = g_adc_value;
    Modbus_Register[REG_ADC_VOLTAGE] = (uint16_t)(g_adc_voltage * 100);

    if (DHT11_Read_TempAndHumidity(&DHT11_Data) == 0)
    {
        Modbus_Register[REG_TEMP_VALUE] = (uint16_t)(DHT11_Data.temperature * 10);
        Modbus_Register[REG_HUMI_VALUE] = (uint16_t)(DHT11_Data.humidity * 10);
        Modbus_Register[REG_SENSOR_STATE] = 0;
    }
    else
    {
        Modbus_Register[REG_TEMP_VALUE] = 0;
        Modbus_Register[REG_HUMI_VALUE] = 0;
        Modbus_Register[REG_SENSOR_STATE] = 1;
    }
}

void Device_Control_Update(void)
{
    if (Modbus_Register[REG_LED_CTRL])
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

    if (Modbus_Register[REG_BEEP_CTRL])
        HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
}

void Device_Alarm_Update(void)
{
    Modbus_Register[REG_ALARM_FLAG] = 0;

    if (Modbus_Register[REG_TEMP_VALUE] > Modbus_Register[REG_TEMP_ALARM_HIGH])
        Modbus_Register[REG_ALARM_FLAG] |= 0x0001;

    if (Modbus_Register[REG_HUMI_VALUE] > Modbus_Register[REG_HUMI_ALARM_HIGH])
        Modbus_Register[REG_ALARM_FLAG] |= 0x0002;
}

void OLED_Display_Update(void)
{
    char buf[24];

    OLED_NewFrame();

    sprintf(buf, "T:%d.%dC",
            Modbus_Register[REG_TEMP_VALUE] / 10,
            Modbus_Register[REG_TEMP_VALUE] % 10);
    OLED_PrintString(0, 0, buf, &font16x16, OLED_COLOR_NORMAL);

    sprintf(buf, "H:%d.%d%%",
            Modbus_Register[REG_HUMI_VALUE] / 10,
            Modbus_Register[REG_HUMI_VALUE] % 10);
    OLED_PrintString(0, 16, buf, &font16x16, OLED_COLOR_NORMAL);

    sprintf(buf, "ADC:%4d", Modbus_Register[REG_ADC_RAW]);
    OLED_PrintString(0, 32, buf, &font16x16, OLED_COLOR_NORMAL);

    sprintf(buf, "L:%d B:%d",
            Modbus_Register[REG_LED_CTRL],
            Modbus_Register[REG_BEEP_CTRL]);
    OLED_PrintString(0, 48, buf, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}
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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim3);                // DHT11微秒延时用
  HAL_ADCEx_Calibration_Start(&hadc1);       // ADC校准
  HAL_Delay(50);
  OLED_Init();
  Modbus_Register_Init();
  HAL_UART_Receive_DMA(&huart1, Modbus_RX_Buffer, 8);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (HAL_GetTick() - g_last_sample_tick >= 1000)
    {
        g_last_sample_tick = HAL_GetTick();

        // Device_Data_Update();
         Device_Alarm_Update();
        Device_Control_Update();
    }

    if (HAL_GetTick() - g_last_oled_tick >= 500)
    {
        g_last_oled_tick = HAL_GetTick();
        OLED_Display_Update();
    }

    if (Modbus_Frame_Flag)
    {
        Modbus_Frame_Flag = 0;

        if (Modbus_Server() == 1)
            Modbus_Register[REG_COMM_STATE] = 0;
        else
            Modbus_Register[REG_COMM_STATE] = 1;

        memset(Modbus_RX_Buffer, 0, sizeof(Modbus_RX_Buffer));
        Modbus_RX_Length = 0;

        HAL_UART_Receive_DMA(&huart1, Modbus_RX_Buffer, 8);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
