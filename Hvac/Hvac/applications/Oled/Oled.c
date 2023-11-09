/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-05     Administrator       the first version
 */
#include "GlobalConfig.h"
#include "typedef.h"
#include "Oled.h"
#include "ascii_fonts.h"
#include "ST7567.h"
#include "Pages.h"
#include "Gpio.h"
#include "Control.h"

type_page_t     pageSelect;
u64             pageInfor           = 0x00000000;   //只支持最多四级目录
u8              reflash_flag        = NO;

void clear_screen(void)
{
  ST7567_Fill(0);
//  ST7567_UpdateScreen();
}

void st7567Init(void)
{
    ST7567_Init();
    rt_thread_mdelay(100);
    clear_screen();
}

u8 GetReflashFlag(void)
{
    return reflash_flag;
}

void SetReflashFlag(u8 flag)
{
    reflash_flag = flag;
}

type_page_t* GetPageSelect(void)
{
    return &pageSelect;
}

//回到上一页
u8 BackPage(void)
{
    u8 page = 0;
    page = pageInfor & 0x000000FF;
    pageInfor >>= 8;

    pageSelect.lastCusor = pageSelect.cusor;

    return page;
}

void pageSelectSet(u8 show,u8 home, u8 max)
{
    pageSelect.cusor_show = show;
    pageSelect.cusor_home = home;
    pageSelect.cusor = pageSelect.cusor_home;
    pageSelect.cusor_max = max;
}

void pageSetting(u8 page)
{

    switch (page)
    {
        case HOME_PAGE:
            pageSelectSet(YES, 1, 1);
            break;
        case SETTING_PAGE:
            pageSelectSet(YES, 1, 18);
            break;
        case TIME_SELECT_PAGE:
            pageSelectSet(YES, 1, 4);
            break;
        default:
            break;
    }
}

void pageProgram(u8 page)
{
    static  u8              pagePre         = 0;
    static  u8              cusonPre        = 0;
    static type_sys_time    time_for;
    static type_sys_time    time_for_pre;

    //1.如果切换页面，清除缓存
    if(pagePre != page)
    {
        if((HOME_PAGE == pagePre) && (SETTING_PAGE == page)) {
            //1.从其他界面切换到设置界面就显示当前时间
            getTimeForMat(&time_for);
            time_for_pre = time_for;
        } else if((TIME_SELECT_PAGE == pagePre) && (SETTING_PAGE == page)) {
            //2.从设置界面切换出去，判断是否有更改时间设置
            //rt_kprintf("year = %d %d\n",time_for_pre.year, time_for.year);
            if(0 != rt_memcmp(&time_for_pre, &time_for, sizeof(type_sys_time))){
                set_date(time_for.year, time_for.month, time_for.day);
                set_time(time_for.hour, time_for.minute, time_for.second);
                //rt_kprintf("设置时间成功\r\n");
            }
        }

        pagePre = page;

        clear_screen();
    }

    //2.如果切换目录也清除缓存
    if(cusonPre != pageSelect.cusor)
    {
        cusonPre = pageSelect.cusor;
        clear_screen();
    }

    switch (page)
    {
        case HOME_PAGE:
            HomePage();

            if(YES == pageSelect.select)
            {
                if(1 == pageSelect.cusor)
                {
                    pageInfor <<= 8;
                    pageInfor |= SETTING_PAGE;
                    pageSelect.changeValue = NO;
                }

                pageSelect.select = NO;
            }
            break;
        case SETTING_PAGE:

            if(YES == pageSelect.select) {

                if(NO == pageSelect.changeValue) {
                    pageSelect.changeValue = YES;
                    pageSelect.up = NO;
                    pageSelect.down = NO;
                } else {
                    pageSelect.changeValue = NO;
                }

                pageSelect.select = NO;
            }

            SettingPage(&pageSelect);

            break;

        default:
            break;
    }

    pageSelect.select = NO;
    SetReflashFlag(NO);
}

void OledEntry(void* parameter)
{
                u8          nowPage             = 0;
    static      u8          nowPagePre          = 0xFF;
    static      u8          Timer1sTouch        = NO;
    static      u16         time1S              = 0;

    st7567Init();
    ST7567_BackLight_On();
    clear_screen();
    pageInfor <<= 8;
    pageInfor |= HOME_PAGE;
    wakeUpOledBackLight(GetBackLightTime());

    while(1)
    {
        time1S = TimerTask(&time1S, 10, &Timer1sTouch);

        nowPage = pageInfor & 0x000000FF;

        if(nowPagePre != nowPage)
        {
            //设置初始光标
            pageSetting(nowPage);

            pageProgram(nowPage);

            nowPagePre = nowPage;
        }
        else
        {
            if(YES == GetReflashFlag())
            {
                pageProgram(nowPage);
            }
        }


        if(YES == Timer1sTouch)
        {
            //监控无操作灭屏以及返回到主页
            monitorBackLight(GetBackLightTime());

            if((HOME_PAGE == nowPage) ||
               (SETTING_PAGE == nowPage) ||
               (TIME_SELECT_PAGE == nowPage))
            {
                SetReflashFlag(YES);
            }
        }

        rt_thread_mdelay(100);
    }
}

void OledInit(void)
{
    rt_err_t threadStart = RT_NULL;

    /* 创建串口 线程 */
    rt_thread_t thread = rt_thread_create("oled_task", OledEntry, RT_NULL, 2048, 20, 10);

    /* 如果线程创建成功则开始启动线程，否则提示线程创建失败 */
    if (RT_NULL != thread) {
        threadStart = rt_thread_startup(thread);
        if (RT_EOK != threadStart) {
            rt_kprintf("sensor task start failed\r\n");
        }
    } else {
        rt_kprintf("sensor task create failed\r\n");
    }
}



