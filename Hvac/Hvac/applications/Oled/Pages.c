/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-31     Administrator       the first version
 */
#include "Oled.h"
#include "ST7567.h"
#include "Pages.h"
#include "Gpio.h"
#include "ButtonTask.h"
#include "Control.h"
#include "Ntc.h"

time_t backlightTime = 0;

//显示摄氏度符号 ℃
void ShowTempUnits(u8 x, u8 y)
{
    char                value[5];

    //显示圆圈
    ST7567_DrawCircle(x, y, 1, 1);
    //显示C
    x += 2;
    y -= 2;
    sprintf(value, "%s", "C");
    ST7567_GotoXY(x, y);
    ST7567_Puts(value, &Font_6x12, 1);
}

//显示温度
void ShowTempNum(u8 x, u8 y, int temp)
{
    char                value[6];
    char                value1[6];

    if(temp == VALUE_NULL) {
        sprintf(value, "%s", " --");
        ST7567_GotoXY(x, y);
        ST7567_Puts(value, &Font_12x24, 1);
    }
    else {

        float temp1 = (float)temp/10;
        sprintf(value, "%2.1f", temp1);
        sprintf(value1, "%5s", value);
        ST7567_GotoXY(x, y);
        ST7567_Puts(value1, &Font_12x24, 1);
    }
}

void showFanRun(u8 x, u8 y, u8 runFlag)
{
    static u8 cnt = 0;
    char temp[2];

    if(YES == runFlag)
    {
        switch(cnt++%4)
        {
            case 0:
                temp[0] = 0x20;
                break;
            case 1:
                temp[0] = 0x21;
                break;
            case 2:
                temp[0] = 0x22;
                break;
            case 3:
                temp[0] = 0x23;
                break;
            default:break;
        }
    }
    else
    {
        temp[0] = 0x20;
    }

    temp[1] = '\0';
    ST7567_GotoXY(x, y);
    ST7567_Puts(temp, &Font_chinese_8X16, 1);
}

void showTemp(u8 x, u8 y, u8 runFlag)
{
    char temp[2];

    switch (runFlag) {
        case NORUN:
            temp[0] = 0x20;
            break;
        case COOLING:
            temp[0] = 0x21;
            break;
        case HEATING:
            temp[0] = 0x22;
            break;
        default:
            temp[0] = 0x20;
            break;
    }

    temp[1] = '\0';
    ST7567_GotoXY(x, y);
    ST7567_Puts(temp, &Image_Temp, 1);
}

void HomePage(void)
{
    type_sys_time       time;
    u8                  x           = 0;
    u8                  y           = 0;
    char                temp[25];
    int                 currentTemp = 0;

    type_info_t         *info       = GetGlobalInfo();
    type_sensor_t       *sensorInfo = GetSensorInfo();

    //1.获取当前时间
    getTimeForMat(&time);
    //2.显示当前时间
    if(YES == info->connected) {
        sprintf(temp, " %02d:%02d:%02d     ID:%3d ", time.hour, time.minute, time.second, info->addr);
    } else {
        sprintf(temp, " %02d:%02d:%02d            ", time.hour, time.minute, time.second);
    }

    ST7567_GotoXY(x, y);
    ST7567_Puts(temp, &Font_6x12, 0);
    //3.显示温度
    x = 0;
    y = 12 + 2;//2为空隙

    currentTemp = GetTempValue();

    ShowTempNum(x, y, currentTemp);
    //显示单位
    x = 5 * 12;
    y = 12 + 2 + 14;
    ShowTempUnits(x, y);

    //显示提示为当前的温度
    x = 5 * 12;
    y = 12 + 2 ;
    sprintf(temp, "%s", "CV");//Current 当前值
    ST7567_GotoXY(x, y);
    ST7567_Puts(temp, &Font_6x12, 1);

    x = 0;
    y = 12 + 2 + 24 + 2;//2为空隙

    currentTemp = GetSetPoint();

    ShowTempNum(x, y, currentTemp);

    //显示单位
    x = 5 * 12;
    y = 12 + 2 + 24 + 2 + 14;
    ShowTempUnits(x, y);

    //显示当前温度的提示
    x = 5 * 12;
    y = 12 + 2 + 24 + 2;
    sprintf(temp, "%s", "SV");//Target 目标值
    ST7567_GotoXY(x, y);
    ST7567_Puts(temp, &Font_6x12, 1);

    //显示分割线
    x = 0 + 2 * 12 + 2 + 12 + 7 * 6;
    y = 12 + 6;
    ST7567_DrawLine(x, y, x, 57, 1);

    x = 0 + 2 * 12 + 2 + 12 + 7 * 6 + 8;
    y = 12 + 4 + 8;
    if(NORUN == info->CoolOrHeat) {
        showFanRun(x, y, NO);
    } else {
        showFanRun(x, y, YES);
    }
    x = 0 + 2 * 12 + 2 + 12 + 7 * 6 + 10 + 16;
    y = 12+4;
    showTemp(x, y, info->CoolOrHeat);

    x = 0 + 2 * 12 + 2 + 12 + 7 * 6 + 8;
    y = 12 + 4 + 32;

    sprintf(temp, "%s%d", "Mode", info->mode);

    ST7567_GotoXY(x, y);
    ST7567_Puts(temp, &Font_6x12, 0);

    //刷新显示
    ST7567_UpdateScreen();
}

/**
 ** 返回显示的名称
 ** Module      |Master
    Mode        |Conventional
                |OB/-O
                |OB/-B
    Uint        | ℃/℉
    Day point   |(目标温度)
    Night point | 夜晚目标温度
    DeadBand    | 1℃
    Fan         |Auto/Always ON
    T data      |Auto/Remote/ Local NTC
    Date        |20xx/mm/dd
    Time        |Hh:mm:SS
    Day time    | 白天持续时间范围
    Version
 **
 * @param num
 * @return
 */
void SettingPageGetChar(u8 num, int data, char *temp)
{

    switch (num) {
        case 1:
            if(data == MODULE_MASTER) {
                strcpy(temp, "Master");
            } else if(data == MODULE_SLAVE) {
                strcpy(temp, "Slave");
            }
            break;
        case 2:
            if(data == MODE_1_CONVENTIONAL) {
                strcpy(temp, "Mode 1");
            } else if(data == MODE_2_0) {
                strcpy(temp, "Mode 2");
            } else if(data == MODE_3_B) {
                strcpy(temp, "Mode 3");
            }
            break;
        case 3:
            if(data == UNIT_CENTIGRADE) {//摄氏度
                strcpy(temp, "C");
            } else if(data == UNIT_FAHRENHEIT) {
                strcpy(temp, "F");
            }
            break;
        case 4:
        case 5:
        case 6:
            if(VALUE_NULL == data) {
                strcpy(temp, "- -");
            } else {
                sprintf(temp, "%d.0", data/10);
            }
            break;
        case 7:
            if(data == FAN_AUTO) {
                strcpy(temp, "Auto");
            } else if(data == FAN_OPEN_ALWAYS) {
                strcpy(temp, "Open");
            }
            break;
        case 8:
            if(data == TEMP_FROM_REMOTE) {//摄氏度
                strcpy(temp, "Remote");
            } else if(data == TEMP_FROM_LOCAL) {
                strcpy(temp, "Local");
            }
            break;
        case 9:
            sprintf(temp, "%4d/", data);
            break;
        case 10:
            sprintf(temp, "%02d/", data);
            break;
        case 11:
            sprintf(temp, "%02d", data);
            break;
        case 12:
            sprintf(temp, "%02d:", data);
            break;
        case 13:
            sprintf(temp, "%02d:", data);
            break;
        case 14:
            sprintf(temp, "%02d", data);
            break;
        case 15:
            sprintf(temp, "%02d:", data);
            break;
        case 16:
            sprintf(temp, "%02d", data);
            break;
        case 17:
            sprintf(temp, "%02d:", data);
            break;
        case 18:
            sprintf(temp, "%02d", data);
            break;
        default:
            strcpy(temp, " ");
            break;
    }
}

void AddData(u8 min, u8 max, u8 *data)
{
    if(*data < max) {
        (*data) ++;
    } else {
        *data = min;
    }
}

void SubData(u8 min, u8 max, u8 *data)
{
    if(*data > min) {
        (*data) --;
    } else {
        *data = max;
    }
}

void AddData_u16(u16 min, u16 max, u16 *data)
{
    if(*data < max) {
        (*data) ++;
    } else {
        *data = min;
    }
}

void SubData_u16(u16 min, u16 max, u16 *data)
{
    if(*data > min) {
        (*data) --;
    } else {
        *data = max;
    }
}

/**
 * @param page
 */
void SettingPage(type_page_t *page)
{
    char                temp[20];
    char                temp1[20];
    char                temp2[20];
    char                temp3[20];
    u8                  x           = 0;
    u8                  y           = 0;
    type_info_t         *info       = GetGlobalInfo();
    int                 data        = 0;
    static int          lastData[3] = {0, 0, 0};
    static u8           max         = 5;
    static u8           min         = 1;
    char name[12][12] = {"Module","Mode","Uint","Day Point","Night Point",
                         "DeadBand","Fan","Temp From","Date","Time",
                         "Day Start","Day End"};
    type_sys_time       time_for;
    u8                  showMenu    = 0;
    static u8 cnt = 0;

    cnt++;
    //1.获取当前时间
    getTimeForMat(&time_for);

    //2.显示当前需要显示的目录
    y = 0;

    if(page->cusor < 9) {
        showMenu = page->cusor;
    } else if(9 == page->cusor || 10 == page->cusor || 11 == page->cusor) {
        showMenu = 9;
    } else if(12 == page->cusor || 13 == page->cusor || 14 == page->cusor) {
        showMenu = 10;
    } else if(15 == page->cusor || 16 == page->cusor) {
        showMenu = 11;
    } else if(17 == page->cusor || 18 == page->cusor) {
        showMenu = 12;
    }

    if(showMenu < min) {
        min = showMenu;
        max = min + 4;
    } else if(showMenu > max) {
        max = showMenu;
        min = max - 4;
    }

    for(int i = min - 1; i < max; i++)
    {
        x = 2;
        sprintf(temp, "%-10s", name[i]);
        ST7567_GotoXY(x, y);
        ST7567_Puts(temp, &Font_6x12, 1);

        //显示横向位置
        x = 2 + 6 * 10;
        if(0 == i) {
            if(1 == page->cusor) {
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData(MODULE_SLAVE, MODULE_MASTER, &info->moduleMode);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData(MODULE_SLAVE, MODULE_MASTER, &info->moduleMode);
                         page->down = NO;
                    }
                }
            }

            data = info->moduleMode;
        } else if(1 == i) {
            if(2 == page->cusor) {
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData(MODE_1_CONVENTIONAL, MODE_3_B, &info->mode);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData(MODE_1_CONVENTIONAL, MODE_3_B, &info->mode);
                         page->down = NO;
                     }
                }
            }

            data = info->mode;
        } else if(2 == i) {
            if(3 == page->cusor) {

                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData(UNIT_CENTIGRADE, UNIT_FAHRENHEIT, &info->unit);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData(UNIT_CENTIGRADE, UNIT_FAHRENHEIT, &info->unit);
                         page->down = NO;
                     }
                }
            }

            data = info->unit;
        } else if(3 == i) {
            if(4 == page->cusor) {

                if(YES == page->changeValue) {
                    if(YES == page->up) {

                        if(info->daySetPoint < 490) {
                            info->daySetPoint += 10;
                        } else {
                            info->daySetPoint = 0;
                        }

                        page->up = NO;
                    }

                    if(YES == page->down) {

                        if(info->daySetPoint > 10) {
                            info->daySetPoint -= 10;
                        } else {
                            info->daySetPoint = 500;
                        }

                        page->down = NO;
                     }
                }
            }

            data = info->daySetPoint;
        } else if(4 == i) {
            if(5 == page->cusor) {

                if(YES == page->changeValue) {
                    if(YES == page->up) {

                        if(info->nightSetPoint < 490) {
                            info->nightSetPoint += 10;
                        } else {
                            info->nightSetPoint = 0;
                        }

                        page->up = NO;
                    }

                    if(YES == page->down) {

                        if(info->nightSetPoint > 10) {
                            info->nightSetPoint -= 10;
                        } else {
                            info->nightSetPoint = 500;
                        }

                        page->down = NO;
                     }
                }
            }

            data = info->nightSetPoint;
        } else if(5 == i) {
            if(6 == page->cusor) {

                if(YES == page->changeValue) {
                    if(YES == page->up) {

                        if(info->deadBand < 50) {
                            info->deadBand += 10;
                        } else {
                            info->deadBand = 10;
                        }

                        page->up = NO;
                    }
                    if(YES == page->down) {

                        if(info->deadBand > 10) {
                            info->deadBand -= 10;
                        } else {
                            info->deadBand = 50;
                        }

                         page->down = NO;
                     }
                }
            }

            data = info->deadBand;
        } else if(6 == i) {
            if(7 == page->cusor) {

                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData(FAN_AUTO, FAN_OPEN_ALWAYS, &info->fanRun);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData(FAN_AUTO, FAN_OPEN_ALWAYS, &info->fanRun);
                         page->down = NO;
                     }
                }
            }
            data = info->fanRun;
        } else if(7 == i) {
            if(8 == page->cusor) {
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData(TEMP_FROM_REMOTE, TEMP_FROM_LOCAL, &info->tempFrom);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData(TEMP_FROM_REMOTE, TEMP_FROM_LOCAL, &info->tempFrom);
                         page->down = NO;
                     }
                }
            }

            data = info->tempFrom;
        } else if(8 == i) {

            if(9 == page->cusor) { //年份
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData_u16(2000, 2100, &time_for.year);
                        set_date(time_for.year, time_for.month, time_for.day);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData_u16(2000, 2100, &time_for.year);
                        set_date(time_for.year, time_for.month, time_for.day);
                        page->down = NO;
                     }
                }
            }

            if(10 == page->cusor) { //月份
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData_u16(1, 12, &time_for.month);
                        set_date(time_for.year, time_for.month, time_for.day);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData_u16(1, 12, &time_for.month);
                        set_date(time_for.year, time_for.month, time_for.day);
                        page->down = NO;
                     }
                }
            }

            if(11 == page->cusor) { //日
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData_u16(1, 31, &time_for.day);
                        set_date(time_for.year, time_for.month, time_for.day);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData_u16(1, 31, &time_for.day);
                        set_date(time_for.year, time_for.month, time_for.day);
                        page->down = NO;
                     }
                }
            }


            lastData[2] = time_for.year;
            lastData[1] = time_for.month;
            lastData[0] = time_for.day;
            SettingPageGetChar(9, lastData[2], temp3);
            SettingPageGetChar(10, lastData[1], temp2);
            SettingPageGetChar(11, lastData[0], temp1);

            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp3, &Font_6x12, 9 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp3, &Font_6x12, 9 == page->cusor ? 0 : 1);
            }
            x += 5 * 6;
            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp2, &Font_6x12, 10 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp2, &Font_6x12, 10 == page->cusor ? 0 : 1);
            }
            x += 3 * 6;
            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp1, &Font_6x12, 11 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp1, &Font_6x12, 11 == page->cusor ? 0 : 1);
            }

        } else if(9 == i) {

            if(12 == page->cusor) { //小时
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData_u16(0, 23, &time_for.hour);
                        set_time(time_for.hour, time_for.minute, time_for.second);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData_u16(0, 23, &time_for.hour);
                        set_time(time_for.hour, time_for.minute, time_for.second);
                        page->down = NO;
                     }
                }
            }

            if(13 == page->cusor) { //分钟
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData_u16(0, 59, &time_for.minute);
                        set_time(time_for.hour, time_for.minute, time_for.second);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData_u16(0, 59, &time_for.minute);
                        set_time(time_for.hour, time_for.minute, time_for.second);
                        page->down = NO;
                     }
                }
            }

            if(14 == page->cusor) { //秒
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData_u16(0, 59, &time_for.second);
                        set_time(time_for.hour, time_for.minute, time_for.second);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData_u16(0, 59, &time_for.second);
                        set_time(time_for.hour, time_for.minute, time_for.second);
                        page->down = NO;
                     }
                }
            }

            lastData[2] = time_for.hour;
            lastData[1] = time_for.minute;
            lastData[0] = time_for.second;
            SettingPageGetChar(12, lastData[2], temp3);
            SettingPageGetChar(13, lastData[1], temp2);
            SettingPageGetChar(14, lastData[0], temp1);
            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp3, &Font_6x12, 12 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp3, &Font_6x12, 12 == page->cusor ? 0 : 1);
            }
            x += 3 * 6;
            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp2, &Font_6x12, 13 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp2, &Font_6x12, 13 == page->cusor ? 0 : 1);
            }
            x += 3 * 6;
            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp1, &Font_6x12, 14 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp1, &Font_6x12, 14 == page->cusor ? 0 : 1);
            }
        } else if(10 == i) {

            if(15 == page->cusor) { //小时
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData(0, 23, (u8 *)&info->dayStartTime + 1);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData(0, 23, (u8 *)&info->dayStartTime + 1);
                        page->down = NO;
                     }
                }
            }

            if(16 == page->cusor) { //分钟
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData(0, 59, (u8 *)&info->dayStartTime);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData(0, 59, (u8 *)&info->dayStartTime);
                        page->down = NO;
                     }
                }
            }

            lastData[1] = info->dayStartTime >> 8;
            lastData[0] = info->dayStartTime & 0x00FF;
            SettingPageGetChar(15, lastData[1], temp2);
            SettingPageGetChar(16, lastData[0], temp1);
            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp2, &Font_6x12, 15 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp2, &Font_6x12, 15 == page->cusor ? 0 : 1);
            }
            x += 3 * 6;
            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp1, &Font_6x12, 16 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp1, &Font_6x12, 16 == page->cusor ? 0 : 1);
            }
        } else if(11 == i) {

            if(17 == page->cusor) { //小时
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData(0, 23, (u8 *)&info->dayEndTime + 1);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData(0, 23, (u8 *)&info->dayEndTime + 1);
                        page->down = NO;
                     }
                }
            }

            if(18 == page->cusor) { //分钟
                if(YES == page->changeValue) {
                    if(YES == page->up) {
                        AddData(0, 59, (u8 *)&info->dayEndTime);
                        page->up = NO;
                    }
                    if(YES == page->down) {
                        SubData(0, 59, (u8 *)&info->dayEndTime);
                        page->down = NO;
                     }
                }
            }

            lastData[1] = info->dayEndTime >> 8;
            lastData[0] = info->dayEndTime & 0x00FF;
            SettingPageGetChar(17, lastData[1], temp2);
            SettingPageGetChar(18, lastData[0], temp1);
            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp2, &Font_6x12, 17 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp2, &Font_6x12, 17 == page->cusor ? 0 : 1);
            }
            x += 3 * 6;
            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp1, &Font_6x12, 18 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp1, &Font_6x12, 18 == page->cusor ? 0 : 1);
            }
        }

        if(i < 8) {

            SettingPageGetChar(i + 1, data, temp1);
            sprintf(temp, "%11s", temp1);
            ST7567_GotoXY(x, y);
            if(YES == page->changeValue) {
                ST7567_Puts(temp, &Font_6x12, i+1 == page->cusor ? (0 == cnt%2 ? 0 : 1) : 1);
            } else {
                ST7567_Puts(temp, &Font_6x12, i+1 == page->cusor ? 0 : 1);
            }
        }

        y+= 12;

    }

    //3.竖行分割线
    ST7567_DrawLine(62, 0, 62, 62, 1);

    //4.横向分割线
    y = 0;
    for(int i = 0; i < 4; i++) {
        y += 12;
        ST7567_DrawLine(0, y, 128, y, 1);
    }
    //5.边框线
    ST7567_DrawRectangle(0, 0, 128, 60, 1);

    //6.刷新显示
    ST7567_UpdateScreen();
}

/**
 *
 * @param page
 */
void TimeSelectPage(type_page_t *page,
                    u16 min,
                    u16 max,
                    u8 step,
                    void *data,
                    u8 size,
                    u8 *flag)
{
    char                    temp[20];
    u8                      column      = 0;
    static u8               cnt         = 0;
    static int              num         = 0;

    if(YES == *flag)
    {
        rt_memcpy(&num, data, size);//不能直接使用=

        if(num < min){
            num = min;
        } else if(num > max) {
            num = max;
        }
        *flag = NO;
    }

    //画三角形
    cnt++;
    if(1 == page->cusor)
    {
        if(cnt % 2)
        {
            ST7567_DrawFilledTriangle(56, 15, 64, 15, 60, 11, 1);
        }
        else
        {
            ST7567_DrawFilledTriangle(56, 15, 64, 15, 60, 11, 0);
        }
    }
    else
    {
        ST7567_DrawFilledTriangle(56, 15, 64, 15, 60, 11, 1);
    }
    column = 16;
    sprintf(temp, "%4d", num);
    ST7567_GotoXY(8 * 6, column);
    ST7567_Puts(temp, &Font_8x16, 1);
    //画三角形
    if(2 == page->cusor)
    {
        if(cnt % 2)
        {
            ST7567_DrawFilledTriangle(56, 33, 64, 33, 60, 37, 1);
        }
        else
        {
            ST7567_DrawFilledTriangle(56, 33, 64, 33, 60, 37, 0);
        }
    }
    else
    {
        ST7567_DrawFilledTriangle(56, 33, 64, 33, 60, 37, 1);
    }

    column = 48;
    sprintf(temp, "%s", "YES");
    ST7567_GotoXY(8 * 5, column);
    ST7567_Puts(temp, &Font_8x16, 3 == page->cusor ? 0 : 1);

    sprintf(temp, "%s", "NO");
    ST7567_GotoXY(8 * 9, column);
    ST7567_Puts(temp, &Font_8x16, 4 == page->cusor ? 0 : 1);

    //计算
    if(YES == page->select)
    {
        //加数
        if(1 == page->cusor)
        {
            if(num + step < max)
            {
                num += step;
            }
            else
            {
                num = min;
            }
        }
        else if(2 == page->cusor)
        {
            if(num > min + step)
            {
                num -= step;
            }
            else
            {
                num = max;
            }
        }
        else if(3 == page->cusor)
        {
            //设置时间
            rt_memcpy(data, &num, size);
            BackPage();
        }
        else if(4 == page->cusor)
        {
            BackPage();
        }

        page->select = NO;
    }

    //刷新显示
    ST7567_UpdateScreen();
}


/**
 * 输出数据文件
 * @param page
 */
void DataPrintPage(type_page_t *page, u16 *data)
{
    char                    temp[20];
    u8                      x           = 0;
    u8                      column      = 16;
//    record_state_t          *record     = GetRecordState();

    //1.显示开始时间
    x = 3;//初始从第三位开始写,显示年
    sprintf(temp, "%04d", data[0]);
    ST7567_GotoXY(8 * x, column);
    ST7567_Puts(temp, &Font_8x16, 1 == page->cusor ? 0 : 1);
    x += strlen(temp) + 1;

    //显示月
    sprintf(temp, "%02d", data[1]);
    ST7567_GotoXY(8 * x, column);
    ST7567_Puts(temp, &Font_8x16, 2 == page->cusor ? 0 : 1);
    x += strlen(temp) + 1;

    //显示日
    sprintf(temp, "%02d", data[2]);
    ST7567_GotoXY(8 * x, column);
    ST7567_Puts(temp, &Font_8x16, 3 == page->cusor ? 0 : 1);
    column += 16;

    //1.显示结束时间
    x = 3;//初始从第三位开始写,显示年
    sprintf(temp, "%04d", data[3]);
    ST7567_GotoXY(8 * x, column);
    ST7567_Puts(temp, &Font_8x16, 4 == page->cusor ? 0 : 1);
    x += strlen(temp) + 1;

    //显示月
    sprintf(temp, "%02d", data[4]);
    ST7567_GotoXY(8 * x, column);
    ST7567_Puts(temp, &Font_8x16, 5 == page->cusor ? 0 : 1);
    x += strlen(temp) + 1;

    //显示日
    sprintf(temp, "%02d", data[5]);
    ST7567_GotoXY(8 * x, column);
    ST7567_Puts(temp, &Font_8x16, 6 == page->cusor ? 0 : 1);
    column += 16;

    sprintf(temp, "%s", "YES");
    ST7567_GotoXY(8 * 5, column);
    ST7567_Puts(temp, &Font_8x16, 7 == page->cusor ? 0 : 1);

    sprintf(temp, "%s", "NO");
    ST7567_GotoXY(8 * 9, column);
    ST7567_Puts(temp, &Font_8x16, 8 == page->cusor ? 0 : 1);

    //刷新显示
    ST7567_UpdateScreen();
}

time_t* GetBackLightTime(void)
{
    return &backlightTime;
}

//唤醒屏幕背景光
void wakeUpOledBackLight(time_t *time)
{
    *time = GetTimeRun();

    rt_pin_write(LCD_BK, YES);
}

//监控无操作1min 后熄屏幕
void monitorBackLight(time_t *time)
{
    if(*time + 60  < GetTimeRun())     //getTimeStamp单位S
    {
        rt_pin_write(LCD_BK, NO);
        BackPage();
    }
}

void EnterBtnCallBack(u8 type)
{

    if(SHORT_PRESS == type)
    {
        //唤醒屏幕
        wakeUpOledBackLight(GetBackLightTime());
        GetPageSelect()->select = YES;
        //提示界面刷新
        SetReflashFlag(YES);
    }
    else if(LONG_PRESS == type)
    {
        BackPage();
        //提示界面刷新
        SetReflashFlag(YES);
    }
}

void UpBtnCallBack(u8 type)
{
    if(SHORT_PRESS == type)
    {
        //只有不选择数值的时候才增加光标
        if(NO == GetPageSelect()->changeValue)
        {
            if(GetPageSelect()->cusor > GetPageSelect()->cusor_home)
            {
                GetPageSelect()->cusor--;
            }
            else
            {
                GetPageSelect()->cusor = GetPageSelect()->cusor_max;
            }
        }

        //唤醒屏幕
        wakeUpOledBackLight(GetBackLightTime());
        //提示界面刷新
        SetReflashFlag(YES);
        GetPageSelect()->up = YES;
    }
}

void DowmBtnCallBack(u8 type)
{
    if(SHORT_PRESS == type)
    {
        //唤醒屏幕
        //只有不选择数值的时候才增加光标
        if(NO == GetPageSelect()->changeValue)
        {
            if(GetPageSelect()->cusor_max > 0)
            {
                if(GetPageSelect()->cusor < GetPageSelect()->cusor_max)
                {
                    GetPageSelect()->cusor++;
                }
                else
                {
                    GetPageSelect()->cusor = GetPageSelect()->cusor_home;
                }
            }
        }
        //提示界面刷新
        wakeUpOledBackLight(GetBackLightTime());
        SetReflashFlag(YES);
        GetPageSelect()->down = YES;
    }
}

