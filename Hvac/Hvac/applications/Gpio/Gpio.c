/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-02     Administrator       the first version
 */
#include "GlobalConfig.h"
#include "typedef.h"
#include "Gpio.h"

static u8 locationFlag = NO;

/**
 ** 获取设备定位标志
 * @return
 */
u8 GetLoacationFlag(void)
{
    return locationFlag;
}

/**
 ** 设备定位
 */
void ModuleLocation(void)
{
    locationFlag = YES;
}

/**
 ** 关闭设备定位
 */
void CloseModuleLocation(void)
{
    locationFlag = NO;
}

void GpioInit(void)
{
    //LCD
    rt_pin_mode(LCD_CS, PIN_MODE_OUTPUT);
    rt_pin_mode(LCD_SCK, PIN_MODE_OUTPUT);
    rt_pin_mode(LCD_MOSI, PIN_MODE_OUTPUT);
    rt_pin_mode(LCD_RST, PIN_MODE_OUTPUT);
    rt_pin_mode(LCD_A0, PIN_MODE_OUTPUT);
    rt_pin_mode(LCD_BK, PIN_MODE_OUTPUT);

    //BUTTON
    rt_pin_mode(KEY_ENTER, PIN_MODE_INPUT);
    rt_pin_mode(KEY_UP, PIN_MODE_INPUT);
    rt_pin_mode(KEY_DOWN, PIN_MODE_INPUT);

    //RELAY
    rt_pin_mode(RELAY_W, PIN_MODE_OUTPUT);
    rt_pin_mode(RELAY_Y, PIN_MODE_OUTPUT);
    rt_pin_mode(RELAY_G, PIN_MODE_OUTPUT);
    rt_pin_mode(RELAY_W2, PIN_MODE_OUTPUT);
    rt_pin_mode(RELAY_Y2, PIN_MODE_OUTPUT);

    //Uart
    rt_pin_mode(UART1_EN, PIN_MODE_OUTPUT);
    rt_pin_write(UART1_EN, IN_RECV);

    //LED
    rt_pin_mode(POWER_LED, PIN_MODE_OUTPUT);
}

/**
 * @brief  : 计时器功能
 * @param  : time      计时器
 * @param  : touchTime 实际上定时的时间,单位ms
 * @param  ：flag ON 定时器到了; OFF 还未达到定时器设定时间
 * @return : 当前的计时器数
 */
u16 TimerTask(u16 *time, u16 touchTime, u8 *flag)
{
    u16 temp = 0;

    temp = *time;

    if(*time < touchTime)
    {
        temp++;
        *time = temp;
        *flag = NO;
    }
    else
    {
        *flag = YES;
        *time = 0;
    }

    return *time;
}

u16 usModbusRTU_CRC(const u8* pucData, u32 ulLen)
{
    u8 ucIndex = 0U;
    u16 usCRC = 0xFFFFU;

    while (ulLen > 0U) {
        usCRC ^= *pucData++;
        while (ucIndex < 8U) {
            if (usCRC & 0x0001U) {
                usCRC >>= 1U;
                usCRC ^= 0xA001U;
            } else {
                usCRC >>= 1U;
            }
            ucIndex++;
        }
        ucIndex = 0U;
        ulLen--;
    }
    return usCRC;
}

void ReadUniqueId(u32 *id)
{
    u8      data[12]    = {0};
    u32     id1         = 0;
    u32     id2         = 0;
    u32     id3         = 0;

    id1 = *(__IO u32*)(ID_ADDR1);
    id2 = *(__IO u32*)(ID_ADDR2);
    id3 = *(__IO u32*)(ID_ADDR3);

    rt_memcpy(&data[0], (u8 *)&id1, 4);
    rt_memcpy(&data[4], (u8 *)&id2, 4);
    rt_memcpy(&data[8], (u8 *)&id3, 4);

    *id = crc32_cal(data, 12);
}
