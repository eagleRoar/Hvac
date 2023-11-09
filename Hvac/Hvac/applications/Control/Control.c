/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-25     12154       the first version
 */
#include "Gpio.h"
#include "typedef.h"
#include "GlobalConfig.h"
#include "Control.h"
#include "Ntc.h"

type_info_t     gInfo;              //模块的基本信息
type_sensor_t   sensorInfo;         //接收到的数据
type_action_t   actionInfo;         //接收到的控制信息
time_t          timeRun;            //为了计数
time_t          lastConnectTime;    //保存接收到数据的时间

void InitGlobalInformation(void)
{
    rt_memset((u8 *)&gInfo, 0, sizeof(gInfo));

    gInfo.registerState = NO;
    gInfo.type = HVAC_TYPE;
    gInfo.addr = DEFAULT_ADDR;
    gInfo.daySetPoint = DEFAULT_SET_POINT;         //250表示 25.0摄氏度
    gInfo.nightSetPoint = DEFAULT_SET_POINT;
    gInfo.deadBand = DEFAULT_DEADBAND;
    gInfo.mode = MODE_1_CONVENTIONAL;
    gInfo.unit = UNIT_CENTIGRADE;           //摄氏度
    gInfo.tempFrom = TEMP_FROM_REMOTE;      //默认数据来自外部协议
    gInfo.dayStartTime = DEFAULT_DAY_START;
    gInfo.dayEndTime = DEFAULT_DAY_END;
    gInfo.CoolOrHeat = NORUN;
    gInfo.connected = NO;
    gInfo.moduleMode = MODULE_SLAVE;
    gInfo.fanRun = FAN_AUTO;
}

type_info_t *GetGlobalInfo(void)
{
    return &gInfo;
}

/*
 * 获取注册状态
 */
u8 GetRegisterState(void)
{
    return GetGlobalInfo()->registerState;
}

/*
 * 设置注册状态
 */
void SetRegisterState(u8 state)
{
    GetGlobalInfo()->registerState = state;
}

/*
 * 获取白天开始时间
 */
u16 GetDayStart(void)
{
    return GetGlobalInfo()->dayStartTime;
}

/*
 * 设置白天开始时间
 */
void SetDayStart(u16 time)
{
    GetGlobalInfo()->dayStartTime = time;
}

/*
 * 获取白天结束时间
 */
u16 GetDayEnd(void)
{
    return GetGlobalInfo()->dayEndTime;
}

/*
 * 设置白天结束时间
 */
void SetDayEnd(u16 time)
{
    GetGlobalInfo()->dayEndTime = time;
}

/*
 * 获取白天黑夜
 */
u8 IdentifyDaYAndNight(void)
{
    int start                   = 0;
    int end                     = 0;
    int now                     = 0;
    u8  hour                    = 0;
    u8  min                     = 0;
    type_sys_time time_for;
    type_info_t *info           = GetGlobalInfo();

    //1.获取当前时间
    getTimeForMat(&time_for);
    now = time_for.hour * 3600 + time_for.minute * 60 + time_for.second;
    //2.获取设置的时间
    hour = info->dayStartTime >> 8;
    min = info->dayStartTime;
    start = hour * 3600 + min * 60;

    hour = info->dayEndTime >> 8;
    min = info->dayEndTime;
    end = hour * 3600 + min * 60;

    if(end > start) {
        if(now > start && now <= end) {
            return IN_DAY;
        } else {
            return IN_NIGHT;
        }
    } else {
        if(now > start || now <= end) {
            return IN_NIGHT;
        } else {
            return IN_DAY;
        }
    }
}

/*
 *获取模块的mode
 */
u8 GetControlMode(void)
{
    return GetGlobalInfo()->mode;
}

/*
 * 设置模块的mode
 */
void SetControlMode(u8 mode)
{
    GetGlobalInfo()->mode = mode;
}

/*
 * 获取单位
 */
u8 GetUnit(void)
{
    return GetGlobalInfo()->unit;
}

/*
 * 设置单位
 */
void SetUnit(u8 unit)
{
    GetGlobalInfo()->unit = unit;
}

/*
 * 获取温度值来源
 */
u8 GetTempFrom(void)
{
    return GetGlobalInfo()->tempFrom;
}

/*
 * 设置温度值来源
 */
void SetTempFrom(u8 from)
{
    GetGlobalInfo()->tempFrom = from;
}

/*
 * 获取DeadBand
 */
u8 GetTempDeadBand(void)
{
    return GetGlobalInfo()->deadBand;
}

/*
 * 设置DeadBand
 */
void SetTempDeadBand(u8 deadband)
{
    GetGlobalInfo()->deadBand = deadband;
}

/*
 * 获取setPoint 值
 */
s16 GetDaySetPoint(void)
{
    return GetGlobalInfo()->daySetPoint;
}

/*
 * 设置setPoint 值
 */
void SetDaySetPoint(s16 setPoint)
{
    GetGlobalInfo()->daySetPoint = setPoint;
}

/*
 * 获取setPoint 值
 */
s16 GetNightSetPoint(void)
{
    return GetGlobalInfo()->nightSetPoint;
}

/*
 * 设置setPoint 值
 */
void SetNightSetPoint(s16 setPoint)
{
    GetGlobalInfo()->nightSetPoint = setPoint;
}

/*
 * 获取setPoint 值
 */
s16 GetSetPoint(void)
{
    type_action_t *action   = GetActionInfo();
    type_info_t *info       = GetGlobalInfo();

    //1.如果是按照受控的
    if(MODULE_SLAVE == info->moduleMode) {
        if(ACTION_TYPE_BY_AUTO == action->type) {
            if(IN_DAY == IdentifyDaYAndNight())
            {
                return GetDaySetPoint();
            }
            else
            {
                return GetNightSetPoint();
            }
        } else if(ACTION_TYPE_USE_MASTER == action->type) {
            return action->setPoint;
        }
    } else {
        //如果是主机模式
        if(IN_DAY == IdentifyDaYAndNight())
        {
            return GetDaySetPoint();
        }
        else
        {
            return GetNightSetPoint();
        }
    }

    return VALUE_NULL;
}

/**
 ** 获取温度数据
 * @return
 */
int GetTempValue(void)
{
    type_sensor_t *sensor = GetSensorInfo();

    //1.分辨传感器数据来源
    if(TEMP_FROM_REMOTE == GetTempFrom())
    {
        //1.传感器数据来自主机传输
        return sensor->temp;
    } else if(TEMP_FROM_LOCAL == GetTempFrom()) {
        //2.需要获取NTC数据

        if(GetNtcValue() <= -20) {
            return VALUE_NULL;
        } else {
            return GetNtcValue();
        }
    }

    return VALUE_NULL;
}

/**
 * 实际控制
 */
void ControlProgram(void)
{
    type_action_t   *action         = GetActionInfo();
    int             temperature     = GetTempValue(); //温度保留小数点1位
    s16             point           = GetSetPoint();
    time_t          nowTime         = GetTimeRun();
    type_info_t     *info           = GetGlobalInfo();
    u8              controlMode     = GetControlMode();
    static time_t   lastTime        = 0;
    static int      lastTemp        = 0;
    static u8       stage           = 1;
    static u8       CoolOrHeat      = NORUN;


    //1.如果温度值为异常(1.还没有收到主机发送过来 2.还没有检测到ntc值)

    if((VALUE_NULL == temperature) || (VALUE_NULL == point)){
        info->CoolOrHeat = NORUN;
        CtrlRelay(RT_NULL, RT_NULL, NO);
        return;
    }

//    rt_kprintf("en = %d, mode = %d, setPoint = %d\r\n",
//            action->en, controlMode, point);

    //2.正常工作
    if(NO == action->en && MODULE_SLAVE == controlMode) {
        info->CoolOrHeat = NORUN;
        CtrlRelay(RT_NULL, RT_NULL, NO);
    } else {

        //1.对比当前温度 以及设置的恒温点
        u8 deadband = info->deadBand;
        int tempMin = (point > deadband) ? point - deadband : 0;
        int tempMax = point + deadband;

        if(temperature > tempMax) {
            info->CoolOrHeat = COOLING;
        } else if(temperature < tempMin) {
            info->CoolOrHeat = HEATING;
        } else {
            info->CoolOrHeat = NORUN;
            CtrlRelay(RT_NULL, RT_NULL, NO);
        }

        if(CoolOrHeat != info->CoolOrHeat){

            lastTime = GetTimeRun();
            lastTemp = temperature;
            stage = 1;
            CoolOrHeat = info->CoolOrHeat;
        }

        //2.默认1挡位, 如果超过5分钟温度没变化超过1度的话就切换2挡位
        if((nowTime > (lastTime + 300)) &&
           (abs(temperature - lastTemp) <= 10)) {//10表示1度
            stage = 2;
        }

//        rt_kprintf("time goes %d, value = %d %d %d, stage = %d\n",
//                nowTime - lastTime, temperature , lastTemp, temperature - lastTemp, stage);

        //3.如果当前状态失联不使能加热或者制冷
        if(NO == info->connected && MODULE_SLAVE == controlMode) {
            info->CoolOrHeat = NORUN;
            CtrlRelay(RT_NULL, RT_NULL, NO);
        } else {
            CtrlRelay(info->CoolOrHeat, stage, YES);
        }
    }
}

/**
 **初始化接受到的信息
 */
void InitSensorInfomation(void)
{
    sensorInfo.co2 = VALUE_NULL;
    sensorInfo.humi = VALUE_NULL;
    sensorInfo.temp = VALUE_NULL;
    sensorInfo.light = VALUE_NULL;
    sensorInfo.time = 0;
}

/**
 ** 获取接收信息的结构体
 */
type_sensor_t *GetSensorInfo(void)
{
    return &sensorInfo;
}

/**
 ** 初始化主机发送的命令动作
 */
void InitActionInfomation(void)
{
    actionInfo.en = NO;
    actionInfo.setPoint = VALUE_NULL;
    actionInfo.type = ACTION_TYPE_BY_AUTO;
}

/**
 ** 获取指定动作
 * @return
 */
type_action_t *GetActionInfo(void)
{
    return &actionInfo;
}

void SetActionInfo(u8 en, u8 type, s16 point)
{
    actionInfo.en = en;
    actionInfo.type = type;
    actionInfo.setPoint = point;
}

/**
 ** 初始化时间计数
 */
void InitTimeRun(void)
{
    timeRun = 0;
}

/**
 ** 获取时间计数
 * @return
 */
time_t GetTimeRun(void)
{
    return timeRun;
}

/**
 ** 时间计数
 */
void TimeRunning(void)
{
    timeRun ++;
}

/**
 ** 发送实际控制
 * @param coolOrHeat
 * @param stage
 * @param en
 */
void CtrlRelay(u8 coolOrHeat, u8 stage, u8 en)
{
    type_info_t *info = GetGlobalInfo();

    if(YES == en) {
        if(COOLING == coolOrHeat) {
            //1.低档制冷
            if(1 == stage) {
                if(MODE_1_CONVENTIONAL == info->mode) {
                    rt_pin_write(RELAY_Y, YES);
                    rt_pin_write(RELAY_G, YES);

                    rt_pin_write(RELAY_W, NO);
                    rt_pin_write(RELAY_W2, NO);
                    rt_pin_write(RELAY_Y2, NO);
                } else if(MODE_2_0 == info->mode) {
                    rt_pin_write(RELAY_W, YES);
                    rt_pin_write(RELAY_Y, YES);
                    rt_pin_write(RELAY_G, YES);

                    rt_pin_write(RELAY_W2, NO);
                    rt_pin_write(RELAY_Y2, NO);
                } else if(MODE_3_B == info->mode) {
                    rt_pin_write(RELAY_Y, YES);
                    rt_pin_write(RELAY_G, YES);

                    rt_pin_write(RELAY_W, NO);
                    rt_pin_write(RELAY_W2, NO);
                    rt_pin_write(RELAY_Y2, NO);
                }
            } else if(2 == stage) {
                //2.高档制冷
                if(MODE_1_CONVENTIONAL == info->mode) {
                    rt_pin_write(RELAY_Y, YES);
                    rt_pin_write(RELAY_G, YES);
                    rt_pin_write(RELAY_Y2, YES);

                    rt_pin_write(RELAY_W, NO);
                    rt_pin_write(RELAY_W2, NO);
                } else if(MODE_2_0 == info->mode) {
                    rt_pin_write(RELAY_W, YES);
                    rt_pin_write(RELAY_Y, YES);
                    rt_pin_write(RELAY_G, YES);
                    rt_pin_write(RELAY_Y2, YES);

                    rt_pin_write(RELAY_W2, NO);
                } else if(MODE_3_B == info->mode) {
                    rt_pin_write(RELAY_Y, YES);
                    rt_pin_write(RELAY_G, YES);
                    rt_pin_write(RELAY_Y2, YES);

                    rt_pin_write(RELAY_W, NO);
                    rt_pin_write(RELAY_W2, NO);
                }
            }
        } else if(HEATING == coolOrHeat) {
            //3.低档加热
            if(1 == stage) {
                if(MODE_1_CONVENTIONAL == info->mode) {
                    rt_pin_write(RELAY_W, YES);
                    rt_pin_write(RELAY_G, YES);

                    rt_pin_write(RELAY_Y, NO);
                    rt_pin_write(RELAY_Y2, NO);
                    rt_pin_write(RELAY_W2, NO);
                } else if(MODE_2_0 == info->mode) {
                    rt_pin_write(RELAY_Y, YES);
                    rt_pin_write(RELAY_G, YES);

                    rt_pin_write(RELAY_W, NO);
                    rt_pin_write(RELAY_Y2, NO);
                    rt_pin_write(RELAY_W2, NO);
                } else if(MODE_3_B == info->mode) {
                    rt_pin_write(RELAY_W, YES);
                    rt_pin_write(RELAY_Y, YES);
                    rt_pin_write(RELAY_G, YES);

                    rt_pin_write(RELAY_Y2, NO);
                    rt_pin_write(RELAY_W2, NO);
                }
            } else if(2 == stage) {
                //4.高档加热
                if(MODE_1_CONVENTIONAL == info->mode) {
                    rt_pin_write(RELAY_W, YES);
                    rt_pin_write(RELAY_G, YES);
                    rt_pin_write(RELAY_W2, YES);

                    rt_pin_write(RELAY_Y, NO);
                    rt_pin_write(RELAY_Y2, NO);
                } else if(MODE_2_0 == info->mode) {
                    rt_pin_write(RELAY_Y, YES);
                    rt_pin_write(RELAY_G, YES);
                    rt_pin_write(RELAY_W2, YES);

                    rt_pin_write(RELAY_W, NO);
                    rt_pin_write(RELAY_Y2, NO);
                } else if(MODE_3_B == info->mode) {
                    rt_pin_write(RELAY_W, YES);
                    rt_pin_write(RELAY_Y, YES);
                    rt_pin_write(RELAY_G, YES);
                    rt_pin_write(RELAY_W2, YES);

                    rt_pin_write(RELAY_Y2, NO);
                }
            }
        }
    } else {
        rt_pin_write(RELAY_W, NO);
        rt_pin_write(RELAY_Y, NO);
        rt_pin_write(RELAY_G, NO);
        rt_pin_write(RELAY_W2, NO);
        rt_pin_write(RELAY_Y2, NO);
    }
}

/**
 ** 初始化最后和主机通讯的时间
 */
void InitLastConnectTime(void)
{
    lastConnectTime = 0;
}

/**
 ** 获取最后通讯时间
 */
time_t GetLastConnectTime(void)
{
    return lastConnectTime;
}

/**
 ** 设置最后通讯时间
 * @param time
 */
void SetLastConnectTime(time_t time)
{
    lastConnectTime = time;
}

/**
 ** 监控连接状态
 */
void MonitorConnectedState(void)
{
    type_info_t *info = GetGlobalInfo();

    //1.判断是否失联
    if(0 == GetLastConnectTime()){
        info->connected = NO;
        info->registerState = NO;
    } else {

        if(GetTimeRun() > (GetLastConnectTime() + TIMEOUT)) {
            info->connected = NO;
            info->registerState = NO;
        } else {
            info->connected = YES;
        }
    }
}
