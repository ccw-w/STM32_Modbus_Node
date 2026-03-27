#include "param.h"
#include "modbus.h"
#include "stm32f1xx_hal_flash.h"
#include <string.h>

DeviceParam_t g_param;
uint8_t g_param_need_save = 0;
uint32_t g_param_save_tick = 0;

static uint8_t Param_IsValid(const DeviceParam_t *p) {
  if (p->save_flag != PARAM_SAVE_FLAG)
    return 0;

  if (p->slave_addr == 0 || p->slave_addr > 247)
    return 0;

  return 1;
}

void Param_LoadDefault(void) {
  g_param.slave_addr = 1;
  g_param.temp_alarm_high = 300; // 30.0℃
  g_param.humi_alarm_high = 700; // 70.0%
  g_param.save_flag = PARAM_SAVE_FLAG;
}

void Param_LoadToRegister(void) {
  const DeviceParam_t *p = (const DeviceParam_t *)PARAM_FLASH_ADDR;

  if (Param_IsValid(p)) {
    memcpy(&g_param, p, sizeof(DeviceParam_t));
  } else {
    Param_LoadDefault();
  }

  Modbus_Register[REG_SLAVE_ADDR] = g_param.slave_addr;
  Modbus_Register[REG_TEMP_ALARM_HIGH] = g_param.temp_alarm_high;
  Modbus_Register[REG_HUMI_ALARM_HIGH] = g_param.humi_alarm_high;
}

uint8_t Param_SaveFromRegister(void) {
  uint16_t i;
  uint16_t *pData = (uint16_t *)&g_param;
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t PageError = 0;

  g_param.slave_addr = Modbus_Register[REG_SLAVE_ADDR];
  g_param.temp_alarm_high = Modbus_Register[REG_TEMP_ALARM_HIGH];
  g_param.humi_alarm_high = Modbus_Register[REG_HUMI_ALARM_HIGH];
  g_param.save_flag = PARAM_SAVE_FLAG;

  HAL_FLASH_Unlock();

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = PARAM_FLASH_ADDR;
  EraseInitStruct.NbPages = 1;

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
    HAL_FLASH_Lock();
    return 0;
  }

  for (i = 0; i < (sizeof(DeviceParam_t) / 2); i++) {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, PARAM_FLASH_ADDR + i * 2,
                          pData[i]) != HAL_OK) {
      HAL_FLASH_Lock();
      return 0;
    }
  }

  HAL_FLASH_Lock();
  return 1;
}

void Param_RequestSave(void) {
  g_param_need_save = 1;
  g_param_save_tick = HAL_GetTick();
}
