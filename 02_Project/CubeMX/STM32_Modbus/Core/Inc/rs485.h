#ifndef _RS485_H_
#define _RS485_H_

#define RS485_EN_W HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_SET)// 使能RS485发送
#define RS485_EN_R HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_RESET)// 使能RS485接收
void RS485_Init(void);

#endif // _RS485_H_
