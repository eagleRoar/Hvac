/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-25     12154       the first version
 */
#ifndef APPLICATIONS_INCLUDES_CONTROL_H_
#define APPLICATIONS_INCLUDES_CONTROL_H_

#define     VALUE_NULL      -9999

/*
 * 失联时间
 */
#define     TIMEOUT         30  //20s

//1字节对齐
#pragma pack(1)
enum{
    MODULE_SLAVE = 0,           //从机
    MODULE_MASTER               //主机
};

enum{
    ACTION_TYPE_USE_MASTER = 0,
    ACTION_TYPE_USE_DAY_SETPOINT,
    ACTION_TYPE_USE_NIGHT_SETPOINT,
    ACTION_TYPE_USE_NULL,
    ACTION_TYPE_USE_NULL1,
    ACTION_TYPE_BY_AUTO,//按照当前的逻辑
};

enum{
    NORUN = 0x00,
    COOLING,
    HEATING,
};

enum{
    FAN_AUTO = 0x00,
    FAN_OPEN_ALWAYS,
};

typedef struct actionInfo{
    u8 en;
    u8 type;    //0: 使用传输的setPoint 1：使用白天的温度设置 2：使用夜晚的温度设置
                //3:使用白天的温度设置 4:使用夜晚的温度设置 5：全自动，根据0x0404~0x040f设置的参数、接收到的温度参数自动运行
    s16 setPoint;//如果type 为0 则选择这个setPoint
}type_action_t;

typedef struct stateInformation{
    u8      registerState;  //是否已经注册成功
    u8      type;           //模块的类型
    u8      addr;           //模块的地址
    s16     daySetPoint;    //恒温点
    s16     nightSetPoint;  //恒温点
    u16     temp;           //当前温度
    u8      deadBand;       //区间
    u8      mode;           //模块的模式 1.Conventional 2.Heat pump OB/-O 3.Heat pump OB/-B
    u8      unit;           //单位 ℃/℉
    u8      tempFrom;       //温度来源 1.Remote/ 2.Local NTC
    u16     dayStartTime;   //白天开始的时间
    u16     dayEndTime;     //白天结束时间
    u16     ctrl;           //当前控制
    u8      CoolOrHeat;     //当前制冷还是加热状态
    u8      connected;      //是否连接
    u8      moduleMode;     //选择主机或者从机
    u8      fanRun;         //选择风扇的开启模式 常开或者自动
    u16     crc;            //crc校验值
}type_info_t;

typedef struct sensorInfomation{
    int co2;
    int humi;
    int temp;
    int light;
    time_t time;
}type_sensor_t;

#pragma pack()

void InitGlobalInformation(void);
type_info_t *GetGlobalInfo(void);
u8 GetRegisterState(void);
void SetRegisterState(u8 state);
u16 GetDayStart(void);
void SetDayStart(u16 time);
u16 GetDayEnd(void);
void SetDayEnd(u16 time);
u8 IdentifyDaYAndNight(void);
u8 GetControlMode(void);
void SetControlMode(u8 mode);
u8 GetUnit(void);
void SetUnit(u8 unit);
u8 GetTempFrom(void);
void SetTempFrom(u8 from);
u8 GetTempDeadBand(void);
void SetTempDeadBand(u8 deadband);
s16 GetDaySetPoint(void);
void SetDaySetPoint(s16 setPoint);
s16 GetNightSetPoint(void);
void SetNightSetPoint(s16 setPoint);
s16 GetSetPoint(void);
void InitSensorInfomation(void);
type_sensor_t *GetSensorInfo(void);
void InitActionInfomation(void);
type_action_t *GetActionInfo(void);
void SetActionInfo(u8 en, u8 type, s16 point);
void CtrlRelay(u8 coolOrHeat, u8 stage, u8 en);
void InitTimeRun(void);
time_t GetTimeRun(void);
void TimeRunning(void);
void ControlProgram(void);
void InitLastConnectTime(void);
time_t GetLastConnectTime(void);
void SetLastConnectTime(time_t time);
void MonitorConnectedState(void);
#endif /* APPLICATIONS_INCLUDES_CONTROL_H_ */
