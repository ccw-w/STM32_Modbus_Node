/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "modbus.h"
#include "param.h"
#include "usart.h"
#include <string.h>
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern void Device_Data_Update(void);
extern void Device_Control_Update(void);
extern void Device_Alarm_Update(void);
extern void OLED_Display_Update(void);

extern volatile uint8_t Modbus_Frame_Flag;
extern volatile uint16_t Modbus_RX_Length;

extern uint32_t g_last_sample_tick;
extern uint32_t g_last_oled_tick;

extern uint16_t g_last_slave_addr;
extern uint16_t g_last_temp_alarm_high;
extern uint16_t g_last_humi_alarm_high;
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
    for (;;) {
      if (HAL_GetTick() - g_last_sample_tick >= 1000) {
        g_last_sample_tick = HAL_GetTick();

        Device_Data_Update();
        Device_Alarm_Update();
      }

      /* 每次循环都更新输出，LED闪烁 */
      Device_Control_Update();

      if (HAL_GetTick() - g_last_oled_tick >= 500) {
        g_last_oled_tick = HAL_GetTick();
        OLED_Display_Update();
      }

      if (Modbus_Register[REG_SLAVE_ADDR] != g_last_slave_addr ||
          Modbus_Register[REG_TEMP_ALARM_HIGH] != g_last_temp_alarm_high ||
          Modbus_Register[REG_HUMI_ALARM_HIGH] != g_last_humi_alarm_high) {
        g_last_slave_addr = Modbus_Register[REG_SLAVE_ADDR];
        g_last_temp_alarm_high = Modbus_Register[REG_TEMP_ALARM_HIGH];
        g_last_humi_alarm_high = Modbus_Register[REG_HUMI_ALARM_HIGH];

        Param_RequestSave();
      }

      if (g_param_need_save && (HAL_GetTick() - g_param_save_tick >= 2000)) {
        g_param_need_save = 0;
        Param_SaveFromRegister();
      }

      if (Modbus_Frame_Flag) {
        Modbus_Frame_Flag = 0;

        if (Modbus_Server() == 1)
          Modbus_Register[REG_COMM_STATE] = 0;
        else
          Modbus_Register[REG_COMM_STATE] = 1;

        memset(Modbus_RX_Buffer, 0, sizeof(Modbus_RX_Buffer));
        Modbus_RX_Length = 0;

        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, Modbus_RX_Buffer,
                                     sizeof(Modbus_RX_Buffer));
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
      }

      osDelay(10);
    }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

