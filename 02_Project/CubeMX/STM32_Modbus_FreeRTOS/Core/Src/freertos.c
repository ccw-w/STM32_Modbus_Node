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
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

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
/* Definitions for commTask */
osThreadId_t commTaskHandle;
const osThreadAttr_t commTask_attributes = {
    .name = "commTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal1,
};
/* Definitions for dataTask */
osThreadId_t dataTaskHandle;
const osThreadAttr_t dataTask_attributes = {
    .name = "dataTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal1,
};
/* Definitions for displayTask */
osThreadId_t displayTaskHandle;
const osThreadAttr_t displayTask_attributes = {
    .name = "displayTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for ctrlTask */
osThreadId_t ctrlTaskHandle;
const osThreadAttr_t ctrlTask_attributes = {
    .name = "ctrlTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityBelowNormal1,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern void Device_Data_Update(void);
extern void Device_Control_Update(void);
extern void Device_Alarm_Update(void);
extern void OLED_Display_Update(void);

extern volatile uint8_t Modbus_Frame_Flag;
extern volatile uint16_t Modbus_RX_Length;

extern uint16_t g_last_slave_addr;
extern uint16_t g_last_temp_alarm_high;
extern uint16_t g_last_humi_alarm_high;
/* USER CODE END FunctionPrototypes */

void StartCommTask(void *argument);
void StartDataTask(void *argument);
void StartDisplayTask(void *argument);
void StartCtrlTask(void *argument);

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
  /* creation of commTask */
  commTaskHandle = osThreadNew(StartCommTask, NULL, &commTask_attributes);

  /* creation of dataTask */
  dataTaskHandle = osThreadNew(StartDataTask, NULL, &dataTask_attributes);

  /* creation of displayTask */
  displayTaskHandle =
      osThreadNew(StartDisplayTask, NULL, &displayTask_attributes);

  /* creation of ctrlTask */
  ctrlTaskHandle = osThreadNew(StartCtrlTask, NULL, &ctrlTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartCommTask */
/**
 * @brief 通信任务：等待串口接收事件，处理 Modbus 帧并重新开启 DMA 接收
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCommTask */
void StartCommTask(void *argument) {
  /* USER CODE BEGIN StartCommTask */
  /* Infinite loop */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, Modbus_RX_Buffer,
                               sizeof(Modbus_RX_Buffer));
  __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
  for (;;) {
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

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
  }
  /* USER CODE END StartCommTask */
}

/* USER CODE BEGIN Header_StartDataTask */
/**
 * @brief 采集任务：周期采集传感器和 ADC 数据，并更新报警状态
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDataTask */
void StartDataTask(void *argument) {
  /* USER CODE BEGIN StartDataTask */
  /* Infinite loop */
  for (;;) {
    Device_Data_Update();
    Device_Alarm_Update();
    osDelay(1000);
  }
  /* USER CODE END StartDataTask */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
 * @brief 显示任务：周期刷新 OLED 显示内容
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument) {
  /* USER CODE BEGIN StartDisplayTask */
  /* Infinite loop */
  for (;;) {
    OLED_Display_Update();
    osDelay(500);
  }
  /* USER CODE END StartDisplayTask */
}

/* USER CODE BEGIN Header_StartCtrlTask */
/**
 * @brief 控制任务：处理 LED/蜂鸣器控制，并负责参数变化检测与延时保存
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCtrlTask */
void StartCtrlTask(void *argument) {
  /* USER CODE BEGIN StartCtrlTask */
  /* Infinite loop */
  for (;;) {
    Device_Control_Update();

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

    osDelay(100);
  }
  /* USER CODE END StartCtrlTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
