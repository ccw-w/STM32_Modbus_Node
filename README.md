# STM32_Modbus_Node

<p align="center">
  <b>STM32F103-based multi-sensor Modbus RTU node with OLED display, alarm control, parameter persistence, and a FreeRTOS task architecture.</b><br/>
  <b>基于 STM32F103 的多传感器 Modbus RTU 通信节点，支持 OLED 显示、报警控制、参数掉电保存，以及 FreeRTOS 任务架构。</b>
</p>

<p align="center">
  <a href="#english">English</a> | <a href="#中文">中文</a>
</p>

---

# English

## Overview

`STM32_Modbus_Node` is an STM32F103-based data acquisition and Modbus communication project with an industrial-oriented design.

The system samples temperature, humidity, and ADC voltage data, displays key status information on an OLED, and exposes device data and control parameters through Modbus RTU over RS485. The current version has been upgraded from a bare-metal polling structure into a FreeRTOS-enhanced version with separated tasks for communication, sampling, display, and control.

---

## Current Features

Implemented functions:

- DHT11 temperature and humidity acquisition
- ADC raw value acquisition and voltage conversion
- OLED real-time display
- Modbus RTU slave communication over USART1 + RS485
- Support for function code `0x03` (Read Holding Registers)
- Support for function code `0x06` (Write Single Register)
- Remote LED and buzzer control
- Temperature / humidity over-limit alarm
- Alarm-triggered LED blinking and buzzer activation
- Parameter persistence in internal Flash
- UART DMA + ReceiveToIdle-based frame reception
- UART callback + thread flags used to wake the communication task
- FreeRTOS task-based architecture

---

## Hardware Platform

- MCU: `STM32F103C8T6`
- Sensor: `DHT11`
- Communication: `USART1 + RS485`
- Display: `OLED`
- Other peripherals:
  - LED
  - Buzzer
  - ADC input

---

## Software Architecture

The current FreeRTOS-enhanced version is divided into the following tasks:

### `dataTask`
Responsible for:

- Sampling DHT11 temperature/humidity
- Sampling ADC raw value
- Converting voltage value
- Updating alarm status

### `displayTask`
Responsible for:

- Refreshing OLED display periodically

### `ctrlTask`
Responsible for:

- LED / buzzer output control
- Parameter change detection
- Delayed Flash save for modified parameters

### `commTask`
Responsible for:

- Waiting for UART receive events
- Parsing Modbus RTU frames
- Returning Modbus responses
- Restarting DMA reception after frame handling

---

## Communication Design

The project implements a basic Modbus RTU slave.

Supported function codes:

- `0x03` Read Holding Registers
- `0x06` Write Single Register

Communication flow:

1. UART receives data through DMA + idle detection
2. UART callback records frame length
3. Communication task is awakened
4. Slave address and CRC16 are checked
5. Function code is parsed
6. Valid registers are processed
7. Response frame is sent back

Only specific registers are writable, which prevents accidental modification of read-only acquisition or status data.

---

## Alarm Logic

The project supports local alarm indication and remote alarm status reading.

Alarm behavior:

- Temperature over threshold sets alarm bit0
- Humidity over threshold sets alarm bit1
- Alarm flag is written into the register area
- On first alarm trigger, buzzer is automatically enabled
- During alarm, LED enters blinking mode
- After alarm is cleared, LED returns to normal control mode

---

## Parameter Persistence

The following parameters are saved into internal Flash:

- Slave address
- Temperature alarm threshold
- Humidity alarm threshold

To reduce Flash wear, parameters are not written immediately after every change.  
Instead, a save request is set first, and the actual Flash write is performed after a short delay.

On power-up, the system loads saved parameters from Flash. If saved data is invalid, default parameters are used.

---

## Register Map

### Read-only Registers

- `0x0000` Temperature value, unit: `0.1°C`
- `0x0001` Humidity value, unit: `0.1%RH`
- `0x0002` ADC raw value
- `0x0003` Voltage value, unit: `0.01V`
- `0x0012` Sensor state (`0`: normal, `1`: fault)
- `0x0013` Communication state (`0`: normal, `1`: error)
- `0x0030` Alarm flag

### Writable Registers

- `0x0010` LED control
- `0x0011` Buzzer control
- `0x0020` Temperature alarm threshold
- `0x0021` Humidity alarm threshold
- `0x0031` Slave address

---

## OLED Display Contents

The OLED page shows:

- Temperature and temperature threshold
- Humidity and humidity threshold
- Alarm flag
- Communication state
- LED state
- Buzzer state
- Voltage value

---

## Test Items

Tested / testable items include:

- Reading temperature, humidity, ADC, and voltage via Modbus Poll
- Writing LED and buzzer control registers
- Modifying temperature/humidity thresholds
- Modifying slave address
- Triggering local alarm by threshold crossing
- Checking parameter persistence after power cycle
- Verifying FreeRTOS-based task split works correctly

---

## Development Environment

- MCU: STM32F103 series
- IDE: Keil MDK5
- Configuration tool: STM32CubeMX
- Programmer: ST-Link
- Language: C
- Library: STM32 HAL
- RTOS: FreeRTOS

---

## Repository Structure

```text
02_Project/
├─ CubeMX/
│  ├─ STM32_Modbus/                 # Original bare-metal version
│  └─ STM32_Modbus_FreeRTOS/        # FreeRTOS-enhanced version
```

---

## Future Improvements

Possible future improvements:

- Replace DHT11 with a more industrial sensor
    
- Add CRC or dual-copy protection for saved parameters
    
- Add mutex protection for shared display / I2C resources
    
- Add more Modbus function codes
    
- Improve measurement accuracy for external voltage sensing
    

---

## License

This repository is licensed under the MIT License.

---

# 中文

## 项目简介

`STM32_Modbus_Node` 是一个基于 STM32F103 的工业风数据采集与通信项目。

系统能够采集温度、湿度和 ADC 电压数据，通过 OLED 实时显示关键状态信息，并通过 RS485 上的 Modbus RTU 对外提供数据读取和控制参数访问。当前版本已经从裸机轮询结构升级为 FreeRTOS 增强版，完成了通信、采集、显示、控制等任务拆分。

---

## 当前功能

已实现功能包括：

- DHT11 温湿度采集
    
- ADC 原始值采集与电压换算
    
- OLED 实时显示
    
- 基于 USART1 + RS485 的 Modbus RTU 从站通信
    
- 支持功能码 `0x03`（读保持寄存器）
    
- 支持功能码 `0x06`（写单个保持寄存器）
    
- 支持远程 LED / 蜂鸣器控制
    
- 支持温湿度超限报警
    
- 报警时 LED 自动闪烁、蜂鸣器自动打开
    
- 支持关键参数掉电保存到片内 Flash
    
- 基于 UART DMA + 空闲中断 的按帧接收

- 基于 UART 回调 + 线程标志唤醒通信任务
    
- 基于 FreeRTOS 的任务化结构
    

---

## 硬件平台

- 主控：`STM32F103C8T6`
    
- 传感器：`DHT11`
    
- 通信接口：`USART1 + RS485`
    
- 显示模块：`OLED`
    
- 其他外设：
    
    - LED
        
    - 蜂鸣器
        
    - ADC 输入
        

---

## 软件架构

当前 FreeRTOS 增强版划分为以下几个任务：

### `dataTask`

负责：

- 采集 DHT11 温湿度
    
- 采集 ADC 原始值
    
- 换算电压值
    
- 更新报警状态
    

### `displayTask`

负责：

- 周期刷新 OLED 显示内容
    

### `ctrlTask`

负责：

- LED / 蜂鸣器输出控制
    
- 参数变化检测
    
- 修改参数后的延时写 Flash 保存
    

### `commTask`

负责：

- 等待串口接收事件
    
- 解析 Modbus RTU 帧
    
- 返回应答数据
    
- 帧处理完成后重新开启 DMA 接收
    

---

## 通信设计

项目实现了一个基础的 Modbus RTU 从站。

支持功能码：

- `0x03` 读保持寄存器
    
- `0x06` 写单个保持寄存器
    

通信流程：

1. 串口通过 DMA + 空闲中断接收数据
    
2. UART 回调记录本次帧长度
    
3. 唤醒通信任务
    
4. 校验从站地址和 CRC16
    
5. 解析功能码
    
6. 处理合法寄存器
    
7. 返回 Modbus 应答帧
    

并且只开放指定寄存器可写，避免误写只读采集区和状态区。

---

## 报警逻辑

项目支持本地声光报警和远程报警状态读取。

报警行为如下：

- 温度超阈值时置报警 bit0
    
- 湿度超阈值时置报警 bit1
    
- 报警标志写入寄存器区
    
- 报警首次触发时自动打开蜂鸣器
    
- 报警期间 LED 进入闪烁模式
    
- 报警解除后 LED 恢复正常控制
    

---

## 参数掉电保存

以下参数会保存到 STM32 片内 Flash：

- 从站地址
    
- 温度报警阈值
    
- 湿度报警阈值
    

为了减少 Flash 擦写次数，参数变化后不会立刻写入，而是先置保存请求标志，延时一段时间后再执行真正的 Flash 写入。

系统上电后会优先从 Flash 恢复这些参数；如果保存数据无效，则自动加载默认参数。

---

## 寄存器说明

### 只读寄存器

- `0x0000` 温度值，单位：`0.1°C`
    
- `0x0001` 湿度值，单位：`0.1%RH`
    
- `0x0002` ADC 原始值
    
- `0x0003` 电压值，单位：`0.01V`
    
- `0x0012` 传感器状态（`0` 正常，`1` 故障）
    
- `0x0013` 通信状态（`0` 正常，`1` 异常）
    
- `0x0030` 报警标志
    

### 可写寄存器

- `0x0010` LED 控制
    
- `0x0011` 蜂鸣器控制
    
- `0x0020` 温度报警阈值
    
- `0x0021` 湿度报警阈值
    
- `0x0031` 从站地址
    

---

## OLED 显示内容

OLED 页面显示内容包括：

- 温度与温度阈值
    
- 湿度与湿度阈值
    
- 报警标志
    
- 通信状态
    
- LED 状态
    
- 蜂鸣器状态
    
- 电压值
    

---

## 测试项

已完成 / 可继续验证的测试项包括：

- 通过 Modbus Poll 读取温湿度、ADC、电压
    
- 写 LED / 蜂鸣器控制寄存器
    
- 修改温湿度报警阈值
    
- 修改从站地址
    
- 超阈值触发本地报警
    
- 掉电重上电后检查参数是否恢复
    
- 验证 FreeRTOS 任务拆分后系统运行是否正常
    

---

## 开发环境

- MCU：STM32F103 系列
    
- IDE：Keil MDK5
    
- 配置工具：STM32CubeMX
    
- 下载工具：ST-Link
    
- 开发语言：C
    
- 库：STM32 HAL
    
- RTOS：FreeRTOS
    

---

## 仓库结构

```text
02_Project/
├─ CubeMX/
│  ├─ STM32_Modbus/                 # 原始裸机版本
│  └─ STM32_Modbus_FreeRTOS/        # FreeRTOS 增强版
```

---

## 后续可改进方向

后续可以继续增强的方向包括：

- 将 DHT11 更换为更偏工业场景的传感器
    
- 给参数保存增加 CRC 或双备份保护
    
- 为显示/I2C 等共享资源增加互斥保护
    
- 扩展更多 Modbus 功能码
    
- 提升外部电压测量的精度与标定方式
    

---

## 开源协议

本仓库使用 MIT License。