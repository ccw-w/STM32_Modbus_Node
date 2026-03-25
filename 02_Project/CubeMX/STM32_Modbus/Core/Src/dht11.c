#include "dht11.h"
#include "main.h"
#include "gpio.h"
#include "delay_us.h"

#define DHT11_TIMEOUT_US   120

static uint8_t DHT11_WaitWhileLevel(GPIO_PinState level, uint32_t timeout_us)
{
    uint32_t i = 0;

    while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == level)
    {
        delay_us(1);
        i++;
        if (i >= timeout_us)
        {
            return ERROR;
        }
    }

    return SUCCESS;
}

static uint8_t DHT11_ReadByteSafe(uint8_t *data)
{
    uint8_t i;
    uint8_t value = 0;

    for (i = 0; i < 8; i++)
    {
        if (DHT11_WaitWhileLevel(GPIO_PIN_RESET, DHT11_TIMEOUT_US) != SUCCESS)
            return ERROR;

        delay_us(35);

        if (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET)
        {
            value |= (uint8_t)(0x01 << (7 - i));

            if (DHT11_WaitWhileLevel(GPIO_PIN_SET, DHT11_TIMEOUT_US) != SUCCESS)
                return ERROR;
        }
        else
        {
            value &= (uint8_t)~(0x01 << (7 - i));
        }
    }

    *data = value;
    return SUCCESS;
}

uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data)
{
    uint8_t sum;
    uint16_t humi_temp;
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_DATA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET);
    delay_us(30);

    GPIO_InitStruct.Pin = DHT11_DATA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct);

    delay_us(30);

    if (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) != GPIO_PIN_RESET)
        return ERROR;

    if (DHT11_WaitWhileLevel(GPIO_PIN_RESET, DHT11_TIMEOUT_US) != SUCCESS)
        return ERROR;

    if (DHT11_WaitWhileLevel(GPIO_PIN_SET, DHT11_TIMEOUT_US) != SUCCESS)
        return ERROR;

    if (DHT11_ReadByteSafe(&DHT11_Data->humi_high8bit) != SUCCESS)
        return ERROR;
    if (DHT11_ReadByteSafe(&DHT11_Data->humi_low8bit) != SUCCESS)
        return ERROR;
    if (DHT11_ReadByteSafe(&DHT11_Data->temp_high8bit) != SUCCESS)
        return ERROR;
    if (DHT11_ReadByteSafe(&DHT11_Data->temp_low8bit) != SUCCESS)
        return ERROR;
    if (DHT11_ReadByteSafe(&DHT11_Data->check_sum) != SUCCESS)
        return ERROR;

    sum = DHT11_Data->humi_high8bit +
          DHT11_Data->humi_low8bit +
          DHT11_Data->temp_high8bit +
          DHT11_Data->temp_low8bit;

    if (sum != DHT11_Data->check_sum)
        return ERROR;

    humi_temp = DHT11_Data->humi_high8bit * 100 + DHT11_Data->humi_low8bit;
    DHT11_Data->humidity = (float)humi_temp / 100.0f;

    humi_temp = DHT11_Data->temp_high8bit * 100 + DHT11_Data->temp_low8bit;
    DHT11_Data->temperature = (float)humi_temp / 100.0f;

    return SUCCESS;
}

uint8_t DHT11_ReadByte(void)
{
    uint8_t data = 0;
    DHT11_ReadByteSafe(&data);
    return data;
}
