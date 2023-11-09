/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-29     RT-Thread    first version
 */

#include "GlobalConfig.h"
#include "Gpio.h"
#include "Pages.h"
#include "Oled.h"
#include "Uart.h"
#include "ButtonTask.h"
#include "Control.h"
#include "drv_flash.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

extern u64             pageInfor;

void delay_us1(uint32_t us)
{
     volatile uint32_t len;
     for (; us > 0; us --)
         for (len = 0; len < 2; len++);
}

time_t getTimeStamp(void)
{
    //注意 经常使用该函数作为定时器 如果修改了当前时间初始设置会导致问题
    return time(RT_NULL);
}

char *getRealTime(void)
{
    time_t      now;

    now = time(RT_NULL);

    return ctime(&now);
}

u8 getMonth(char *mon)
{
    if(0 == rt_memcmp("Jan", mon, 3))
    {
        return 1;
    }
    else if(0 == rt_memcmp("Feb", mon, 3))
    {
        return 2;
    }
    else if(0 == rt_memcmp("Mar", mon, 3))
    {
        return 3;
    }
    else if(0 == rt_memcmp("Apr", mon, 3))
    {
        return 4;
    }
    else if(0 == rt_memcmp("May", mon, 3))
    {
        return 5;
    }
    else if(0 == rt_memcmp("Jun", mon, 3))
    {
        return 6;
    }
    else if(0 == rt_memcmp("Jul", mon, 3))
    {
        return 7;
    }
    else if(0 == rt_memcmp("Aug", mon, 3))
    {
        return 8;
    }
    else if(0 == rt_memcmp("Sep", mon, 3))
    {
        return 9;
    }
    else if(0 == rt_memcmp("Oct", mon, 3))
    {
        return 10;
    }
    else if(0 == rt_memcmp("Nov", mon, 3))
    {
        return 11;
    }
    else if(0 == rt_memcmp("Dec", mon, 3))
    {
        return 12;
    }
    else {
        return 0;
    }
}

void getTimeForMat(type_sys_time *time_for)
{
    /* "Wed Jun 30 21:49:08 1993\n" */
    time_t time = getTimeStamp();
    struct tm *testTm = localtime(&time);
    time_for->year = testTm->tm_year + 1900;
    time_for->month = testTm->tm_mon + 1;
    time_for->day = testTm->tm_mday;
    time_for->hour = testTm->tm_hour;
    time_for->minute = testTm->tm_min;
    time_for->second = testTm->tm_sec;
}

type_sys_time GetSysTimeByTimestamp(time_t time)
{
    type_sys_time time_for;

    struct tm *testTm = localtime(&time);
    time_for.year = testTm->tm_year + 1900;
    time_for.month = testTm->tm_mon + 1;
    time_for.day = testTm->tm_mday;
    time_for.hour = testTm->tm_hour;
    time_for.minute = testTm->tm_min;
    time_for.second = testTm->tm_sec;

    return time_for;
}

void LedProgram(u8 period)
{
    static u16      cnt     = 0;
    static u8       flag    = 0;
    static u8       locationFlag = NO;
    static time_t   lastTime = 0;
    static u8       mode    = LED_NORMAL;

    if(locationFlag != GetLoacationFlag()) {
        locationFlag = GetLoacationFlag();
        if(YES == locationFlag) {
            lastTime = GetTimeRun();
        }
    }

    if(YES == locationFlag)
    {
        //1.1分钟快闪
        if(GetTimeRun() > lastTime + 60 ) {
            CloseModuleLocation();
            mode = LED_NORMAL;
        } else {
            mode = LED_FAST;
        }

        rt_kprintf("%d %d %d\n",GetTimeRun(), lastTime, 60 * (1000 / period));
    }

    if(LED_NORMAL == mode) {
        if(cnt < 1000/period) {
            cnt++;
        } else {
            if(0 == flag) {
                flag = 1;
            } else if(1 == flag) {
                flag = 0;
            }
            cnt = 0;
        }
    } else if(LED_FAST == mode) {
        if(cnt < 100/period) {
            cnt++;
        } else {
            if(0 == flag) {
                flag = 1;
            } else if(1 == flag) {
                flag = 0;
            }
            cnt = 0;
        }
    }

    rt_pin_write(POWER_LED, flag);
}

int main(void)
{
    static u8       Timer1sTouch        = NO;
    static u16      time1S              = 0;
    static u8       Timer10sTouch       = NO;
    static u16      time10S             = 0;

    GpioInit();
    ButtonTaskInit();
    OledInit();
    UartInit();

    InitTimeRun();
    //获取保存数据

    while (1)
    {
        time1S = TimerTask(&time1S, 1000/20, &Timer1sTouch);
        time10S = TimerTask(&time10S, 10000/20, &Timer10sTouch);

        //20ms
        {
            LedProgram(20);
        }

        //1s
        if(YES == Timer1sTouch) {
            //1.时间计数
            TimeRunning();
            //2.实际上的控制
            ControlProgram();
            //3.监控是否失联
            MonitorConnectedState();
        }

        //10s
        if(YES == Timer10sTouch) {
            //1.识别白天黑夜
            IdentifyDaYAndNight();
        }

        rt_thread_mdelay(20);
    }

    return RT_EOK;
}
