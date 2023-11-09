/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-25     Administrator       the first version
 */
#include "Uart.h"
#include "Communicate.h"
#include "UartDataLayer.h"
#include "Control.h"

#define FLASH_TYPEPROGRAM_WORD                 0x02U  /*!<Program a word (32-bit) at a specified address.*/
#define UART_PERIOD                            20
rt_mutex_t eth_dynamic_mutex = RT_NULL;

int stm32_flash_read(rt_uint32_t addr, rt_uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return -RT_EINVAL;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(rt_uint8_t *) addr;
    }

    return size;
}

uint32_t myTestAddr = DATA_BLOCK_START;//0x0801FF9C;

HAL_StatusTypeDef flash_write(uint32_t address, uint64_t *data, int size)
{
    uint32_t PageError=0;
    FLASH_EraseInitTypeDef FlashEraseInit;//擦除结构体
    HAL_StatusTypeDef ret = HAL_OK;
    //整页擦除
    FlashEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;       //擦除类型，页擦除
    FlashEraseInit.Page = 63;                                               //从哪页开始擦除
    FlashEraseInit.NbPages = 1;                         //一次只擦除一页
    HAL_FLASH_Unlock();
    if(HAL_FLASHEx_Erase(&FlashEraseInit,&PageError) != HAL_OK)
    {
        return 2;//发生错误了
    }

    int len = 0;
    len = 0 == size % sizeof(uint64_t) ? size / sizeof(uint64_t) : size / sizeof(uint64_t) + 1;

    for(int i = 0; i < len; i++)
    {
        ret = HAL_FLASH_Program(TYPEPROGRAM_DOUBLEWORD, address + 8 * i, *(data + i));
    }
    HAL_FLASH_Lock();

    return ret;
}

//获取全局信息
void TakeGlobalInfo(void)
{
    u16             crc     = 0;
    type_info_t     *info   = GetGlobalInfo();

    //获取数据
    stm32_flash_read(DATA_BLOCK_START, (u8 *)info, sizeof(type_info_t));

    crc = usModbusRTU_CRC((u8 *)info, sizeof(type_info_t) - 2);

    //1.如果是从来没有保存过那么crc为错误
    if(crc != info->crc)
    {
        InitGlobalInformation();
    }
    else
    {
        rt_kprintf("------------TakeGlobalInfo， crc = %x, info->crc = %x\n", crc, info->crc);
    }
}

//保存全局信息
void SaveGlobalInfo(void)
{
    static u16      last_crc    = 0;
    u16             crc         = 0;
    type_info_t     *info   = GetGlobalInfo();

    crc = usModbusRTU_CRC((u8 *)info, sizeof(type_info_t) - 2);
    if(last_crc != crc)
    {
        last_crc = crc;
        //保存数据
        info->crc = crc;
        rt_kprintf("-----------------------SaveGlobalInfo\n");
        flash_write(DATA_BLOCK_START, (uint64_t *)info, sizeof(type_info_t));
    }
}

static void UartEntry(void* parameter)
{
    static      u8              Timer1sTouch        = NO;
    static      u16             time1S              = 0;
    static      u8              Timer10sTouch       = NO;
    static      u16             time10S             = 0;

    InitUart1();
    TakeGlobalInfo();
    InitSensorInfomation();
    InitActionInfomation();
    InitLastConnectTime();

    InitRecvMsgList();
    InitSendMsgList();
    GetGlobalInfo()->registerState = NO;
    while(1)
    {
        time1S = TimerTask(&time1S, 1000/UART_PERIOD, &Timer1sTouch);
        time10S = TimerTask(&time10S, 10000/UART_PERIOD, &Timer10sTouch);

        //1.100ms program
        if(YES == Uart1HasRecv())
        {
            //1.表示已经接收 清除标志
            Uart1CleanRecvFlag();
            //2.接收到数据分析
            Uart1Analysis(GetUart1Message()->data, GetUart1Message()->size);
        }
        else
        {
            SendMsgHandle();
        }

        //3.10s program
        if(YES == Timer10sTouch)
        {
            if(NO == GetGlobalInfo()->registerState)
            {
                u8 registerData[20];
                u8 len = GenerateRegisterData(registerData);
                if(RT_EOK == AddToSendMsgList(registerData, len))
                {
//                    rt_kprintf("send list size = %d\r\n",GetSendMsgListSize());
                }
            }

            SaveGlobalInfo();
        }

        rt_thread_mdelay(UART_PERIOD);
    }
}

void UartInit(void)
{
    rt_err_t threadStart = RT_NULL;

    eth_dynamic_mutex = rt_mutex_create("dmutex", RT_IPC_FLAG_FIFO);
    if(eth_dynamic_mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
    }

    /* 创建串口 线程 */
    rt_thread_t thread = rt_thread_create("uart_task", UartEntry, RT_NULL, 2048, 10, 8);//Justin 修改优先级

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
