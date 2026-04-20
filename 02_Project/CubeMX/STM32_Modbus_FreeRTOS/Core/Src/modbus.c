#include "modbus.h"
#include "CRC.h"
#include "main.h"
#include "string.h"
#include "usart.h"

uint16_t Modbus_Register[256] = {0};
uint8_t Modbus_RX_Buffer[256] = {0};

void Modbus_Register_Init(void) {
  memset(Modbus_Register, 0, sizeof(Modbus_Register));

  Modbus_Register[REG_LED_CTRL] = 0;
  Modbus_Register[REG_BEEP_CTRL] = 0;
  Modbus_Register[REG_SENSOR_STATE] = 0;
  Modbus_Register[REG_COMM_STATE] = 0;

  Modbus_Register[REG_TEMP_ALARM_HIGH] = 300;
  Modbus_Register[REG_HUMI_ALARM_HIGH] = 800;
  Modbus_Register[REG_ALARM_FLAG] = 0;
  Modbus_Register[REG_SLAVE_ADDR] = 0x0001;
}

uint8_t Modbus_Is_Writeable_Register(uint16_t reg_addr) {
  switch (reg_addr) {
  case REG_LED_CTRL:
  case REG_BEEP_CTRL:
  case REG_TEMP_ALARM_HIGH:
  case REG_HUMI_ALARM_HIGH:
  case REG_SLAVE_ADDR:
    return 1;

  default:
    return 0;
  }
}

uint8_t Modbus_Server(void) {
  uint8_t modbus_addr;
  uint16_t crc_recv;
  uint16_t crc_calc;

  if (Modbus_RX_Length < 4)
    return 0;

  modbus_addr = (uint8_t)(Modbus_Register[REG_SLAVE_ADDR] & 0x00FF);

  if (Modbus_RX_Buffer[0] != modbus_addr)
    return 0;

  crc_recv = Modbus_RX_Buffer[Modbus_RX_Length - 2] |
             (Modbus_RX_Buffer[Modbus_RX_Length - 1] << 8);

  crc_calc = CalculateCRC(Modbus_RX_Buffer, Modbus_RX_Length - 2);

  if (crc_recv != crc_calc)
    return 0;

  switch (Modbus_RX_Buffer[1]) {
  case 0x03:
    if (Modbus_RX_Length != 8)
      return 0;
    Modbus_03();
    return 1;

  case 0x06:
    if (Modbus_RX_Length != 8)
      return 0;
    Modbus_06();
    return 1;

  default:
    return 0;
  }
}

void Modbus_03(void) {
  uint8_t response[256];
  uint16_t reg_addr, reg_num;
  uint16_t crc;
  uint16_t index = 3;

  reg_addr = (Modbus_RX_Buffer[2] << 8) | Modbus_RX_Buffer[3];
  reg_num = (Modbus_RX_Buffer[4] << 8) | Modbus_RX_Buffer[5];

  if (reg_num == 0)
    return;

  if ((reg_addr + reg_num) > 256)
    return;

  if ((reg_num * 2 + 5) > sizeof(response))
    return;

  response[0] = Modbus_RX_Buffer[0];
  response[1] = Modbus_RX_Buffer[1];
  response[2] = (uint8_t)(reg_num * 2);

  for (uint16_t i = 0; i < reg_num; i++) {
    response[index++] = (uint8_t)(Modbus_Register[reg_addr + i] >> 8);
    response[index++] = (uint8_t)(Modbus_Register[reg_addr + i] & 0xFF);
  }

  crc = CalculateCRC(response, index);
  response[index++] = crc & 0xFF;
  response[index++] = crc >> 8;

  HAL_UART_Transmit(&huart1, response, index, HAL_MAX_DELAY);
}

void Modbus_06(void) {
  uint16_t reg_addr, reg_value;
  uint16_t crc;
  uint8_t response[8];

  reg_addr = (Modbus_RX_Buffer[2] << 8) | Modbus_RX_Buffer[3];
  reg_value = (Modbus_RX_Buffer[4] << 8) | Modbus_RX_Buffer[5];

  if (reg_addr >= 256)
    return;

  if (!Modbus_Is_Writeable_Register(reg_addr))
    return;

  Modbus_Register[reg_addr] = reg_value;

  memcpy(response, Modbus_RX_Buffer, 6);

  crc = CalculateCRC(response, 6);
  response[6] = crc & 0xFF;
  response[7] = crc >> 8;

  HAL_UART_Transmit(&huart1, response, 8, HAL_MAX_DELAY);
}
