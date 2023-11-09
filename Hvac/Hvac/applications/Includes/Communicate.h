/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-01     12154       the first version
 */
#ifndef APPLICATIONS_INCLUDES_COMMUNICATE_H_
#define APPLICATIONS_INCLUDES_COMMUNICATE_H_

#include "GlobalConfig.h"
#include "typedef.h"

#define     MSG_SIZE        20      //发送串口数据长度
#define     MSG_LIST_MAX    20      //列表中最大能存储的数目

//在以下结构体添加pragma的字节对齐限定会导致出错
/* 链表数据 */
typedef struct dataList{
    u8 data[MSG_SIZE];
    u8 len;
    rt_list_t node;
}/*__attribute__((aligned(1)))*/ rt_massage_list ;

void InitSendMsgList(void);
rt_err_t AddToSendMsgList(u8 *data, u8 len);
rt_err_t RemoveFromSendMsgList(u8 *data, u8 len);
u16 GetSendMsgListSize(void);
void InitRecvMsgList(void);
void SendMsgHandle(void);

#endif /* APPLICATIONS_INCLUDES_COMMUNICATE_H_ */
