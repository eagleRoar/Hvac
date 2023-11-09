/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-05     12154       the first version
 */
#ifndef APPLICATIONS_INCLUDES_UARTDATALAYER_H_
#define APPLICATIONS_INCLUDES_UARTDATALAYER_H_

#include "typedef.h"
#include "GlobalConfig.h"

#define             DEVICE_UART1            "uart1"
#define             DEVICE_UART3            "uart3"
#define             UART_MSG_SIZE           1024

#define             REGISTER_CODE           0xFA
#define             REGISTER_ANSER          0x80

//ModBus 读写命令
#define             READ_MUTI               0x03        //Read multiple registers
#define             WRITE_SINGLE            0x06        //Write single register
#define             WRITE_MUTI              0x10        //Write multiple registers

//寄存器定义
#define             REG_NOW_TIME            0x0009      //当前时间
#define             REG_NOW_TIME1           0x000A      //当前时间

#define             LOCATION                0x0000      //定位
#define             REG_PORT_SIZE           0x0400      //端口数量
#define             REG_CTRL                0x0401      //控制
#define             REG_PORT_STATE          0x0402      //端口状态
#define             REG_UNIT                0x0403      //单位
#define             REG_MODE                0x0404      //接线方式
#define             REG_DEADBAND            0x0405      //deadband
#define             REG_CTRL_MODE           0x0406      //控制方式
#define             REG_DAY_SETPOINT        0x0407      //白天 set point
#define             REG_NIGHT_SETPOINT      0x0408      //晚上 set point
#define             REG_DAY_COOL            0x0409      //白天制冷温度
#define             REG_DAY_HEAT            0x040A      //白天制热温度
#define             REG_NIGHT_COOL          0x040B      //晚上制冷温度
#define             REG_NIGHT_HEAT          0x040C      //晚上制热温度
#define             REG_TEMP_FROM           0x040D      //温度来源
#define             REG_DAY_START           0x040E      //白天开始的时间 小时+分钟
#define             REG_DAY_END             0x040F      //白天结束的时间 小时+分钟


#define             REG_NOW_CO2             0xF000      //当前Co2值
#define             REG_NOW_HUMI            0xF001      //当前湿度值
#define             REG_NOW_TEMP            0xF002      //当前温度值
#define             REG_NOW_LIGHT           0xF003      //当前光敏值

//读写动作
#define             READ_CMD                0
#define             WRITE_CMD               1

#pragma pack(1)
/* 串口接收消息结构*/
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
    u8 messageFlag;
    u8 data[UART_MSG_SIZE];                 //保存串口数据
};

#pragma pack()

void InitUart1(void);
void SendToMaster(u8 *data, u16 size);
u8 Uart1HasRecv(void);
void Uart1CleanRecvFlag(void);
struct rx_msg *GetUart1Message(void);
u8 GenerateRegisterData(u8 *data);
void Uart1Analysis(u8 *data, u8 len);
void ModbusCommand(u8 rw, u16 reg,u16 *data);
void ModbusCommandAnalysis(u8 *data);
#endif /* APPLICATIONS_INCLUDES_UARTDATALAYER_H_ */
