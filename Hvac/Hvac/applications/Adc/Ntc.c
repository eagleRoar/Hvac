/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-11     12154       the first version
 */
#include "Ntc.h"
#include "typedef.h"
#include "Control.h"

#define ADC_DEV_NAME        "adc1"      /* ADC 设备名称 */
#define ADC_DEV_CHANNEL     4           /* ADC 通道 */
#define REFER_VOLTAGE       3300         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
#define CONVERT_BITS        (1 << 12)   /* 转换位数为12位 */

const int32_t NTC_Rtable[257]=  //NTC_Rtable[0]:-40,NTC_Rtable[200]:160 摄氏度
{
        203750, 192049, 181156, 171001, 161522, 152664, 144377, 136617, 129343, 122518,
        116110, 110088, 104425, 99096,  94078,  89350,  84892,  80688,  76720,  72973,
        69434,  66089,  62926,  59935,  57104,  54425,  51888,  49484,  47206,  45047,
        43000,  41057,  39214,  37465,  35804,  34226,  32727,  31303,  29949,  28661,
        27513,  26271,  25162,  24107,  23101,  22144,  21231,  20362,  19533,  18742,
        18016,  17269,  16583,  15928,  15302,  14704,  14134,  13588,  13067,  12568,
        12092,  11636,  11199,  10782,  10382,  10000,  9633,   9282,   8946,   8623,
        8314,   8018,   7734,   7461,   7199,   6948,   6707,   6476,   6254,   6040,
        5835,   5638,   5449,   5267,   5091,   4923,   4761,   4605,   4455,   4311,
        4168,   4038,   3909,   3785,   3665,   3550,   3439,   3332,   3229,   3129,
        3033,   2941,   2851,   2765,   2682,   2602,   2524,   2449,   2377,   2307,
        2240,   2175,   2112,   2051,   1992,   1935,   1880,   1827,   1776,   1726,
        1678,
};

s32 Res2Temp(u32 Res)
{
    u8 i=128,j;
    s16  temp;
    for(j=0;j<8;j++)
    {
        if((NTC_Rtable[i]>=Res)&&(NTC_Rtable[i+1]<Res))
                break;
        else
        {
            if(NTC_Rtable[i]<Res)
                i=i-(128>>j);
            else {
                i=i+(128>>j);
            }

        }
    }
    temp=i-40;
    temp=temp*10+(NTC_Rtable[i]-Res)*10/(NTC_Rtable[i]-NTC_Rtable[i+1]);
    if(Res>NTC_Rtable[0])
        return VALUE_NULL;

    if(temp>=800)   //温度大于80摄氏度，返回错误
        return VALUE_NULL;
    return temp;
}

int GetNtcValue(void)
{
    rt_adc_device_t adc_dev;
    rt_uint32_t value, vol;
    rt_err_t ret = RT_EOK;
    static ntc_list_t list;

    /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
        return RT_ERROR;
    }

    /* 使能设备 */
    ret = rt_adc_enable(adc_dev, ADC_DEV_CHANNEL);

    /* 读取采样值 */
    value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);

    /* 转换为对应电压值 */
    vol = value * REFER_VOLTAGE / CONVERT_BITS;
//    rt_kprintf("the voltage is :%d , the value = %d \n",
//            vol , value);

    s32 res = (10000 * vol) / (REFER_VOLTAGE - vol);

//    printf("res = %d\r\n", res);

    /* 关闭通道 */
    ret = rt_adc_disable(adc_dev, ADC_DEV_CHANNEL);

    if(RT_EOK == ret) {

        u8 len = rt_slist_len(&list.node);

        if(len < 10) {
            ntc_list_t *newNode = rt_malloc(sizeof(ntc_list_t));
            if(newNode)
            {
                newNode->data = res;
                rt_slist_insert(&list.node, &newNode->node);
            }
            return VALUE_NULL;
        } else {
            ntc_list_t *newNode = rt_malloc(sizeof(ntc_list_t));
            if(newNode)
            {
                newNode->data = res;
                //1.pop 出节点
                ntc_list_t *first;
                rt_slist_t *node;
                first = rt_slist_tail_entry(&list.node, ntc_list_t, node);
                rt_slist_remove(&list.node, &first->node);
                rt_free(first);
                rt_slist_insert(&list.node, &newNode->node);
            }
            int num = 0;
            ntc_list_t *temp;
            rt_slist_t *node;
            rt_slist_for_each_entry(temp, &list.node, node){
                num += temp->data;
            }
            num /= 10;

            return Res2Temp(num);
        }

    } else {
        return VALUE_NULL;
    }
}
