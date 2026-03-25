#ifndef MODBUS_H
#define MODBUS_H

#include "main.h"

/* 0x0000 ~ 0x000F：采集数据区（只读） */
#define REG_TEMP_VALUE        0x0000   // 温度*10
#define REG_HUMI_VALUE        0x0001   // 湿度*10
#define REG_ADC_RAW           0x0002   // ADC原始值
#define REG_ADC_VOLTAGE       0x0003   // 电压*100

/* 0x0010 ~ 0x001F：控制状态区（部分可写） */
//可写
#define REG_LED_CTRL          0x0010   // 0关 1开
#define REG_BEEP_CTRL         0x0011   // 0关 1开
//只读
#define REG_SENSOR_STATE      0x0012   // 0正常 1故障
#define REG_COMM_STATE        0x0013   // 0正常 1异常

/* 0x0020 ~ 0x002F：参数区（可写） */
#define REG_TEMP_ALARM_HIGH   0x0020   // 温度报警上限*10
#define REG_HUMI_ALARM_HIGH   0x0021   // 湿度报警上限*10

/* 0x0030 ~ 0x003F：报警/系统区 */
//只读
#define REG_ALARM_FLAG        0x0030   // 报警标志
//可写
#define REG_SLAVE_ADDR        0x0031   // 从站地址

extern uint16_t Modbus_Register[256];
extern uint8_t Modbus_RX_Buffer[256];
extern volatile uint16_t Modbus_RX_Length;

void Modbus_03(void);
void Modbus_06(void);
uint8_t Modbus_Server(void);
void Modbus_Register_Init(void);
uint8_t Modbus_Is_Writeable_Register(uint16_t reg_addr);

#endif// MODBUS_H
