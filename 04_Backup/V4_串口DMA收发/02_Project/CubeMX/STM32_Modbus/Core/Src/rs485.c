#include "rs485.h"
#include "main.h"
#include "usart.h"

void RS485_Send_Bytes(uint8_t *data, uint16_t size)
{
    RS485_EN_W; // 使能发送
    HAL_UART_Transmit(&huart1, data, size, HAL_MAX_DELAY);
}
void RS485_Init(void)
{
	
}
