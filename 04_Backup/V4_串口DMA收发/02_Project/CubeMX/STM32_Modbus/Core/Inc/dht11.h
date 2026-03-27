#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"

typedef struct
{
	uint8_t  humi_high8bit;		//原始数据：湿度高8位
	uint8_t  humi_low8bit;	 	//原始数据：湿度低8位
	uint8_t  temp_high8bit;	 	//原始数据：温度高8位
	uint8_t  temp_low8bit;	 	//原始数据：温度高8位
	uint8_t  check_sum;	 	    //校验和
  float    humidity;        //实际湿度
  float    temperature;     //实际温度  
} DHT11_Data_TypeDef;

uint8_t DHT11_ReadByte(void);
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data);

#endif /* __DHT11_H */
