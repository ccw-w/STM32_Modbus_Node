#include "delay_us.h"

extern TIM_HandleTypeDef htim3;

void delay_us(uint32_t us) {
  __HAL_TIM_SET_COUNTER(&htim3, 0); // 将定时器计数器清零
  while (__HAL_TIM_GET_COUNTER(&htim3) < us); // 等待计数器达到指定的微秒数
}
