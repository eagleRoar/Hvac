/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-31     Administrator       the first version
 */
#ifndef APPLICATIONS_INCLUDES_PAGES_H_
#define APPLICATIONS_INCLUDES_PAGES_H_
#include "GlobalConfig.h"
#include "Oled.h"

void HomePage(void);
void DataPrintPage(type_page_t *page, u16 *data);
void TimeSelectPage(type_page_t *page,
                    u16 min,
                    u16 max,
                    u8 step,
                    void *data,
                    u8 size,
                    u8 *flag);
time_t* GetBackLightTime(void);
void monitorBackLight(time_t *time);
void EnterBtnCallBack(u8 type);
void UpBtnCallBack(u8 type);
void DowmBtnCallBack(u8 type);
void wakeUpOledBackLight(time_t *time);
void SettingPage(type_page_t *page);

#endif /* APPLICATIONS_INCLUDES_PAGES_H_ */
