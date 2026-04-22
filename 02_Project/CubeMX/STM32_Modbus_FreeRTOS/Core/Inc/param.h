#ifndef __PARAM_H__
#define __PARAM_H__

#include "main.h"
#include <stdint.h>

#define PARAM_FLASH_ADDR 0x0800FC00UL // 参数断电存储地址，位于最后一个扇区
#define PARAM_SAVE_FLAG 0xA55A        // 合法数据签名

typedef struct {
  uint16_t slave_addr;
  uint16_t temp_alarm_high;
  uint16_t humi_alarm_high;
  uint16_t save_flag;
} DeviceParam_t;

extern DeviceParam_t g_param;
extern uint8_t g_param_need_save;
extern uint32_t g_param_save_tick;

void Param_LoadDefault(void);
void Param_LoadToRegister(void);
uint8_t Param_SaveFromRegister(void);
void Param_RequestSave(void);

#endif
