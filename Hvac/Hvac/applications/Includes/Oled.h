/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-05     Administrator       the first version
 */
#ifndef APPLICATIONS_INCLUDES_OLED_H_
#define APPLICATIONS_INCLUDES_OLED_H_

#include "typedef.h"
#include <sys/time.h>

#pragma pack(1)
typedef struct pageSelect
{
    u8  cusor_show;
    u8  cusor_home;
    u8  cusor_max;
    u8  cusor;
    u8  showMore;   //显示更多
    u8  select;
    u8  lastCusor;
    u8  changeValue;
    u8  up;
    u8  down;
}type_page_t;
#pragma pack()


enum
{
//主页面
HOME_PAGE = 0x00,
//设置界面
SETTING_PAGE,
//时间设置组件界面
TIME_SELECT_PAGE,
};

void OledInit(void);
u8 GetReflashFlag(void);
void SetReflashFlag(u8 flag);
type_page_t* GetPageSelect(void);
u8 BackPage(void);
void st7567Init(void);
void ST7567_BackLight_On(void);
void clear_screen(void);
void wakeUpOledBackLight(time_t *time);
void pageSetting(u8 page);
void pageProgram(u8 page);
#endif /* APPLICATIONS_INCLUDES_OLED_H_ */
