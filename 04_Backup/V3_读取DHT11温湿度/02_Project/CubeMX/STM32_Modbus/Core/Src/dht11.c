 #include "dht11.h"
 #include "main.h"
 #include "gpio.h"
 #include "delay_us.h"

uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data)
{  
  uint8_t temp;
  uint16_t humi_temp;
     // 1. 配置为输出模式，发送开始信号
     GPIO_InitTypeDef GPIO_InitStruct = {0};
     GPIO_InitStruct.Pin = DHT11_DATA_Pin;
     GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
     GPIO_InitStruct.Pull = GPIO_NOPULL;
     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
     HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct);

     HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_RESET);
     HAL_Delay(20);   // 主机拉低至少18ms
     HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET);
     delay_us(30);    // 主机拉高20~40us

     // 2. 切换为输入模式，等待DHT11响应
     GPIO_InitStruct.Pin = DHT11_DATA_Pin;
     GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
     GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
     HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct);
     delay_us(30);    // 延迟20~40us
     /*判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行*/  
     if (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_RESET)
     {
         while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_RESET);
         while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET);
         /*开始接收数据*/   
        DHT11_Data->humi_high8bit= DHT11_ReadByte();
        DHT11_Data->humi_low8bit = DHT11_ReadByte();
        DHT11_Data->temp_high8bit= DHT11_ReadByte();
        DHT11_Data->temp_low8bit = DHT11_ReadByte();
        DHT11_Data->check_sum    = DHT11_ReadByte(); 
        
        /* 对数据进行处理 */
        humi_temp=DHT11_Data->humi_high8bit*100+DHT11_Data->humi_low8bit;
        DHT11_Data->humidity =(float)humi_temp/100;
        
        humi_temp=DHT11_Data->temp_high8bit*100+DHT11_Data->temp_low8bit;
        DHT11_Data->temperature=(float)humi_temp/100;    
    /*检查读取的数据是否正确*/
    temp = DHT11_Data->humi_high8bit + DHT11_Data->humi_low8bit + 
           DHT11_Data->temp_high8bit+ DHT11_Data->temp_low8bit;
    if(DHT11_Data->check_sum==temp)
    { 
      return SUCCESS;
    }
    else 
      return ERROR;
	}	
	else
		return ERROR;
     }


 uint8_t DHT11_ReadByte(void)
 {
     uint8_t i, data=0;
	
	for(i=0;i<8;i++){
        /*每bit以50us低电平标置开始，轮询直到从机发出 的50us 低电平 结束*/ 
        while(HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_RESET);
        /*DHT11 以26~28us的高电平表示“0”，以70us高电平表示“1”，
		 *通过检测 x us后的电平即可区别这两个状 ，x 即下面的延时 
		 */
		delay_us(35); //延时x us 这个延时需要大于数据0持续的时间即可
        if(HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET)//仍然为1则为数据1
        {
            data |=(uint8_t)(0x01<<(7-i));  //把第7-i位置1，MSB先行
            while(HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET);
		}
		else	 // x us后为低电平表示数据“0”
		{			   
			data &= (uint8_t)~(0x01 << (7 - i));   // 清零当前位，其他位保持不变
		}
	}
	return data;
}
