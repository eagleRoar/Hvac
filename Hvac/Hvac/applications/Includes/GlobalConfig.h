/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-02     Administrator       the first version
 */
#ifndef APPLICATIONS_GLOBALCONFIG_H_
#define APPLICATIONS_GLOBALCONFIG_H_

#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <sys/types.h>
//#include <dfs_posix.h>
//#include <dfs_file.h>
#include <unistd.h>
#include <sys/time.h>
#include "drv_flash.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rtdbg.h>

/*
 * 模块类型
 */
#define         HVAC_TYPE           0x61

/*
 * 模块默认地址
 */
#define         DEFAULT_ADDR        0x01

/*
 * 默认的set point 温度
 */
#define         DEFAULT_SET_POINT   250 //250表示  25.0℃

/*
 * 默认的白天开始时间
 */
#define         DEFAULT_DAY_START   0x0800//高8位表示小时 低8位表示分钟

/*
 * 默认的白天结束时间
 */
#define         DEFAULT_DAY_END     0x1200//18:00

/*
 * 默认的deadband
 */
#define         DEFAULT_DEADBAND    50

///*
// * 默认的单位
// */
//#define         DEFAULT_DEADBAND    10

//定义flash 数据区间的开始地址
#define             DATA_BLOCK_START        0x801FF00

#pragma pack(1)
enum{
    IN_DAY      = 0x00,
    IN_NIGHT,
};

enum{
    MODE_1_CONVENTIONAL     = 0x01,     //Conventional
    MODE_2_0,                           //Heat pump OB/-O
    MODE_3_B,                           //Heat pump OB/-B
};

enum{
    UNIT_CENTIGRADE         = 0x01,     //摄氏度
    UNIT_FAHRENHEIT,                    //华氏度
};

enum{
    TEMP_FROM_REMOTE      = 0x01,     //传感器数据来源外部协议传入
    TEMP_FROM_LOCAL,                  //传感器数据来源本地ntc采集
};

#pragma pack()

#endif /* APPLICATIONS_GLOBALCONFIG_H_ */
