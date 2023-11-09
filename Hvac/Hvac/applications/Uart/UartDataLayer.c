/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-05     12154       the first version
 */
#include "Gpio.h"
#include "UartDataLayer.h"
#include "Control.h"
#include "Communicate.h"

static      rt_device_t                 uart1_serial;
struct      rx_msg                      uart1_msg;              //接收串口数据以及相关消息

/**
 **接收数据的接口
 * @param dev
 * @param size
 * @return
 */
rt_err_t Uart1_input(rt_device_t dev, rt_size_t size)
{
    u16 crc16 = 0x0000;

    uart1_msg.dev = dev;
    uart1_msg.size = size;
    rt_device_read(uart1_msg.dev, 0, uart1_msg.data, uart1_msg.size);

    if(2 > size)
    {
        return RT_ERROR;
    }
    crc16 |= uart1_msg.data[uart1_msg.size-1];
    crc16 <<= 8;
    crc16 |= uart1_msg.data[uart1_msg.size-2];
    if(crc16 == usModbusRTU_CRC(uart1_msg.data, uart1_msg.size - 2))
    {
        if(0xFA == uart1_msg.data[0] ||
           0x00 == uart1_msg.data[0] ||
           GetGlobalInfo()->addr == uart1_msg.data[0])//屏蔽其他模块干扰 //Justin debug
        {
            uart1_msg.messageFlag = YES;
        }
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;
    }
}

/**
 **初始化和主机通讯的串口
 */
void InitUart1(void)
{
    uart1_serial = rt_device_find(DEVICE_UART1);
    rt_device_open(uart1_serial, RT_DEVICE_FLAG_DMA_RX);
    rt_device_set_rx_indicate(uart1_serial, Uart1_input);
}

/**
 **向主机发送数据
 * @param data
 * @param size
 */
void SendToMaster(u8 *data, u16 size)
{
    rt_pin_write(UART1_EN, IN_SEND);
    rt_device_write(uart1_serial, 0, data, size);
    rt_pin_write(UART1_EN, IN_RECV);
}

/**
 **获取串口1接收到数据的标志
 * @return
 */
u8 Uart1HasRecv(void)
{
    return uart1_msg.messageFlag;
}


/**
 **清除串口1接收到数据的标志
 * @return
 */
void Uart1CleanRecvFlag(void)
{
    uart1_msg.messageFlag = NO;
}

/**
 **返回串口1的数据
 * @return
 */
struct rx_msg *GetUart1Message(void)
{
    return &uart1_msg;
}

/**
 **解析串口任务
 * @param data
 * @param len
 */
void Uart1Analysis(u8 *data, u8 len)
{
    u16 crc16 = 0;

    //1.接收的数据必须大于2,否则无意义
    if(len <= 2){
        return;
    }

    //2.modbus校验
    crc16 |= data[len-1];
    crc16 <<= 8;
    crc16 |= data[len-2];
    if(crc16 != usModbusRTU_CRC(data, len - 2)) {
        return;
    }

    //3.执行具体功能
    if(REGISTER_CODE == data[0]) {
        //3.1 接收注册回复
        if(REGISTER_ANSER == data[1])
        {
            u32 id = 0x00;
            u32 readId = data[2] << 24 | data[3] << 16 | data[4] << 8 | data[5];
            ReadUniqueId(&id);
            if(id == readId)
            {
                GetGlobalInfo()->registerState = YES;
                GetGlobalInfo()->addr = data[7];
            }
        }
        //3.2 刷新连接时间
        SetLastConnectTime(GetTimeRun());

//        rt_kprintf("***************recv data:");
//        for(int i = 0; i < len; i++)
//        {
//            rt_kprintf("%x ",data[i]);
//        }
//        rt_kprintf("\n");//Justin
    } else {
        //3.2 如果未注册成功，不接受任何指令，此举是为了防止总线上的撞码
        if(NO == GetGlobalInfo()->registerState){
            return;
        }
        //3.3 如果addr不是本机地址并且不是广播地址，则过滤
        if((data[0] != GetGlobalInfo()->addr) && (data[0] != 0x00))//Justin debug 这个需要增加条件，如果是广播地址只允许时间和传感器数据
        {
            return;
        }

        //3.4 如果是本机地址，则执行相关功能
        ModbusCommandAnalysis(data);

        //3.5 刷新连接时间
        SetLastConnectTime(GetTimeRun());

//        rt_kprintf("***************recv data:");
//        for(int i = 0; i < len; i++)
//        {
//            rt_kprintf("%x ",data[i]);
//        }
//        rt_kprintf("\n");//Justin
    }
}

/**
 **解析modbus协议数据
 * @param data
 */
void ModbusCommandAnalysis(u8 *data)
{
    u8      size        = 0;
    u8      len         = 0;
    u8      *reply      = RT_NULL;

    if(RT_NULL == data){
        return;
    }

    u8 addr = data[0];
    u8 cmd = data[1];
    u16 reg = data[2] << 8 | data[3];
    u16 test = 0x0000;
    switch(cmd)
    {
        case READ_MUTI:
            size = data[4] << 8 | data[5];
            len = size * 2 + 5;

            //rt_kprintf("size = %d, len = %d, reg = %x\r\n",size,len,reg);
            reply = rt_malloc(len);
            if(reply)
            {
                reply[0] = addr;
                reply[1] = cmd;
                reply[2] = size;
                for(int i = 0; i < size; i++)
                {
                    ModbusCommand(READ_CMD, reg + i, &test);
                    //rt_kprintf("i = %d, test = %d\r\n",i,test);
                    reply[3 + i * 2] = test >> 8;
                    reply[4 + i * 2] = test;
                }
                reply[3 + 2 * size] = usModbusRTU_CRC(reply, 3 + 2 * size);
                reply[4 + 2 * size] = usModbusRTU_CRC(reply, 3 + 2 * size) >> 8;
                //加入到发送列表
                AddToSendMsgList(reply, len);
                rt_free(reply);
            }
            break;
        case WRITE_SINGLE:
            size = 1;
            test = data[4] << 8 | data[5];
            ModbusCommand(WRITE_CMD, reg, &test);
            //对于地址0 的广播事件不回复
            if(0 != data[0]) {
                AddToSendMsgList(data, 8);
            }
            break;
        case WRITE_MUTI:
            size = data[4] << 8 | data[5];

            //1.处理写数据
            for(int i = 0; i < size; i++)
            {
                test = data[7 + i * 2] << 8 | data[8 + i * 2];
                ModbusCommand(WRITE_CMD, reg + i, &test);
            }

            if(0 != addr)//如果是广播 不返回
            {
                reply = rt_malloc(8);
                if(reply){

                    //2.回复数据
                    reply[0] = addr;
                    reply[1] = cmd;
                    reply[2] = reg >> 8;
                    reply[3] = reg;
                    reply[4] = 0x00;
                    reply[5] = size;
                    reply[6] = usModbusRTU_CRC(reply, 6);
                    reply[7] = usModbusRTU_CRC(reply, 6) >> 8;
                    //加入到发送列表

                    AddToSendMsgList(reply, 8);
                    rt_free(reply);
                }
            }

            break;
        default:break;
    }
}

/**
 ** 读写寄存器
 * @param rw
 * @param reg
 * @return
 */
void ModbusCommand(u8 rw,
                  u16 reg,
                  u16 *data)
{
    type_info_t     *info           = GetGlobalInfo();
    type_sensor_t   *sensorInfo     = GetSensorInfo();
    static time_t   timeCorrect     = 0;

    switch (reg) {
        //定位
        case LOCATION:
            if(READ_CMD == rw){
                *data = 0x0000;
                ModuleLocation();
            }
            break;
        //1.端口数量
        case REG_PORT_SIZE:
            if(READ_CMD == rw){
                *data = 0x05;   //端口数据为5
            }
            break;
        //2.当前控制
        case REG_CTRL:
            if(READ_CMD == rw){
                *data = info->ctrl;
            }
            else if(WRITE_CMD == rw) {
                info->ctrl = *data;
                u8 en = info->ctrl >> 15;
                u8 type = info->ctrl >> 8 & 0x0F;
                s16 point = info->ctrl & 0x00FF;
                point *= 10;
                SetActionInfo(en, type, point);
            }
            break;
        //3.端口状态
        case REG_PORT_STATE:
            if(READ_CMD == rw){
                *data = 0x0000;
                if(YES == rt_pin_read(RELAY_Y2))
                {
                    *data |= 0x0010;
                }
                if(YES == rt_pin_read(RELAY_W2))
                {
                    *data |= 0x0008;
                }
                if(YES == rt_pin_read(RELAY_G))
                {
                    *data |= 0x0004;
                }
                if(YES == rt_pin_read(RELAY_Y))
                {
                    *data |= 0x0002;
                }
                if(YES == rt_pin_read(RELAY_W))
                {
                    *data |= 0x0001;
                }
            }
            break;
        //4.单位
        case REG_UNIT:
            if(READ_CMD == rw){
                *data = info->unit;
            }else if(WRITE_CMD == rw){
                info->unit = *data;
            }
            break;
        //5.接线模式
        case REG_MODE:
            if(READ_CMD == rw){
                *data = info->mode;
            }else if(WRITE_CMD == rw){
                info->mode = *data;
            }
            break;
        //6.DeadBand
        case REG_DEADBAND:
            if(READ_CMD == rw){
                *data = info->deadBand;
            }else if(WRITE_CMD == rw){
                info->deadBand = *data;
            }
            break;
        //7.控制模式
        case REG_CTRL_MODE:
            if(READ_CMD == rw){
                *data = 0x00;   //默认就是恒温模式
            }
            break;
        //8.白天Set Point
        case REG_DAY_SETPOINT:
            if(READ_CMD == rw){
                *data = info->daySetPoint;
            }else if(WRITE_CMD == rw){
                info->daySetPoint = *data;
            }
            break;
        //9.晚上Set Point
        case REG_NIGHT_SETPOINT:
            if(READ_CMD == rw){
                *data = info->nightSetPoint;
            }else if(WRITE_CMD == rw){
                info->nightSetPoint = *data;
            }
            break;
        //10.白天制冷目标
        case REG_DAY_COOL:
            //目前不支持
            *data = 0xFFFF;
            break;
        //11.白天制热目标
        case REG_DAY_HEAT:
            //目前不支持
            *data = 0xFFFF;
            break;
        //12.晚上制冷目标
        case REG_NIGHT_COOL:
            //目前不支持
            *data = 0xFFFF;
            break;
        //13.晚上制热目标
        case REG_NIGHT_HEAT:
            //目前不支持
            *data = 0xFFFF;
            break;
        //14.温度数据来源
        case REG_TEMP_FROM:
            if(READ_CMD == rw){
                *data = info->tempFrom;
            }else if(WRITE_CMD == rw){
                info->tempFrom = *data;
            }
            break;
        //15.白天开始的时间 小时+分钟
        case REG_DAY_START:
            if(READ_CMD == rw){
                *data = info->dayStartTime;
            }else if(WRITE_CMD == rw){
                info->dayStartTime = *data;
            }
            break;
        //16.白天结束的时间 小时+分钟
        case REG_DAY_END:
            if(READ_CMD == rw){
                *data = info->dayEndTime;
            }else if(WRITE_CMD == rw){
                info->dayEndTime = *data;
            }
            break;
        //17.当前Co2数值
        case REG_NOW_CO2:
            if(WRITE_CMD == rw){
                sensorInfo->co2 = *data;
            }
            break;
        //18.当前湿度数值
        case REG_NOW_HUMI:
            if(WRITE_CMD == rw){
                sensorInfo->humi = *data;
            }
            break;
        //19.当前温度数值
        case REG_NOW_TEMP:
            if(WRITE_CMD == rw){
                sensorInfo->temp = *data;
            }
            //rt_kprintf("sensorInfo->temp = %d\n", sensorInfo->temp);
            break;
        //20.当前光敏数值
        case REG_NOW_LIGHT:
            if(WRITE_CMD == rw){
                sensorInfo->light = *data;
            }
            break;
        //21.当前广播时间
        case REG_NOW_TIME:
            if(WRITE_CMD == rw){
//                sensorInfo->time |=
                timeCorrect = 0;
                timeCorrect |= *data << 16;
            }
            break;
        //22.当前广播时间1
        case REG_NOW_TIME1:
            if(WRITE_CMD == rw){
                timeCorrect |= *data;
                sensorInfo->time = timeCorrect;

                //1.校正时间
                type_sys_time timeFor = GetSysTimeByTimestamp(sensorInfo->time);
                set_date(timeFor.year, timeFor.month, timeFor.day);
                set_time(timeFor.hour, timeFor.minute, timeFor.second);
            }
            break;
        default:
            *data = 0xFFFF; //无该数据
            break;
    }
}

/**
 **生成注册数据
 * @return
 */
u8 GenerateRegisterData(u8 *data)
{
    u32 id;

    if(RT_NULL == data) {
        return 0;
    }

    ReadUniqueId(&id);
    data[0] = REGISTER_CODE;
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x06;
    data[7] = GetGlobalInfo()->addr;
    data[8] = GetGlobalInfo()->type;
    data[9] = id >> 24;
    data[10] = id >> 16;
    data[11] = id >> 8;
    data[12] = id;
    u16 crc16Result = usModbusRTU_CRC(data, 13);
    data[13] = crc16Result;                         //CRC16低位
    data[14] = (crc16Result>>8);                    //CRC16高位

    return 15;
}
