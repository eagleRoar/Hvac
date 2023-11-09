/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-02     Administrator       the first version
 */
#ifndef APPLICATIONS_INCLUDES_GPIO_H_
#define APPLICATIONS_INCLUDES_GPIO_H_

#include "typedef.h"
#include <sys/time.h>

#define     SET_PIN_HIGH        1
#define     SET_PIN_LOW         0

//OLED
#define     LCD_CS          GET_PIN(C, 9)
#define     LCD_SCK         GET_PIN(D, 1)
#define     LCD_MOSI        GET_PIN(D, 4)
#define     LCD_RST         GET_PIN(D, 0)
#define     LCD_A0          GET_PIN(D, 2)
#define     LCD_BK          GET_PIN(D, 5)

#define     CTL_LCD         GET_PIN(C, 6)

//BUTTON
#define     KEY_ENTER       GET_PIN(D, 9)
#define     KEY_UP          GET_PIN(C, 7)
#define     KEY_DOWN        GET_PIN(C, 6)

//继电器
#define     RELAY_W         GET_PIN(B, 13)
#define     RELAY_Y         GET_PIN(B, 14)
#define     RELAY_G         GET_PIN(B, 15)
#define     RELAY_W2        GET_PIN(A, 8)
#define     RELAY_Y2        GET_PIN(A, 9)

//Uart1 en
#define     UART1_EN        GET_PIN(B, 0)

//NTC
#define     NTC             GET_PIN(A, 4)

//LED
#define     POWER_LED       GET_PIN(B, 12)

enum{
    IN_RECV = 0,
    IN_SEND,
};

enum{
    LED_NORMAL = 0,
    LED_FAST
};

typedef struct system_time
{
    u16 year;
    u16 month;
    u16 day;
    u16 hour;
    u16 minute;
    u16 second;
}type_sys_time;

void GpioInit(void);
u16 TimerTask(u16 *time, u16 touchTime, u8 *flag);
u16 usModbusRTU_CRC(const u8* pucData, u32 ulLen);
void getTimeForMat(type_sys_time *time_for);
type_sys_time GetSysTimeByTimestamp(time_t time);
time_t getTimeStamp(void);
u8 GetLoacationFlag(void);
void ModuleLocation(void);
void CloseModuleLocation(void);
#endif /* APPLICATIONS_INCLUDES_GPIO_H_ */
