/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-01     12154       the first version
 *
 * 1.如果正在接收时，不发送；
 * 2.发送和接收数据各自放在缓存列表中；
 * 3.接收时监控是否在线；
 * 4.超过一定时间没收到外部数据认为失联，失联之后需要周期性发送注册信息，直到收到注册回复才能执行命令；
 */

#include "Communicate.h"
#include "UartDataLayer.h"

rt_massage_list sendMsgList;      //需要发送出去的数据列表
rt_massage_list recvMsgList;      //接收到的数据列表
extern rt_mutex_t eth_dynamic_mutex;
/**
 ** 初始化发送数据列表的链表结构
 * @param data
 * @param len
 * @return
 */
void InitSendMsgList(void)
{
    rt_list_init(&sendMsgList.node);
}

/**
 **获取发送列表的长度
 * @return
 */
u16 GetSendMsgListSize(void)
{
    return rt_list_len(&sendMsgList.node);
}


/**
 **加入到需要发送的列表中
 * @param data
 * @param len
 * @return
 */
rt_err_t AddToSendMsgList(u8 *data, u8 len)
{
    u16                 listRealSize    = 0;
    rt_massage_list     *newMsg         = RT_NULL;

    //1.发送数据长度检验
    if((len > MSG_SIZE) || (GetSendMsgListSize() >= MSG_LIST_MAX)) {
        return RT_ERROR;
    }

    //2.申请数据空间
    newMsg = rt_calloc(1, sizeof(rt_massage_list));

    //3.遍历列表，如果没有相同内容则加入列表
    if(RT_NULL != newMsg) {
        //3.1 遍历列表
        listRealSize = rt_list_len(&sendMsgList.node);

        if(0 == listRealSize) {
            newMsg->len = len;
            rt_memcpy(newMsg->data, data, len);
            rt_list_insert_after(&sendMsgList.node, &newMsg->node);
        }
        else {
            rt_massage_list *temp;
            rt_list_t node;
            rt_list_for_each_entry(temp, &sendMsgList.node, node)
            {
                if(0 == rt_memcmp(data, temp->data, len))
                {
                    //3.2 列表中有相同的数据,不添加,并释放申请空间
                    rt_free(newMsg);
                    return RT_ERROR;
                }
            }
            //3.3添加到列表中
            newMsg->len = len;
            rt_memcpy(newMsg->data, data, len);
            rt_list_insert_after(&sendMsgList.node, &newMsg->node);
        }

        return RT_EOK;
    }
    else {
        return RT_ERROR;
    }
}

/**
 **移除节点
 * @param data 数据内容
 * @param len  数据长度
 * @return
 */
rt_err_t RemoveFromSendMsgList(u8 *data, u8 len)
{
    //1.如果数据为空或者长度异常大则返回错误
    if((RT_NULL == data) ||
       (len > MSG_SIZE || 0 == len)) {
        return RT_ERROR;
    }

    //2.遍历列表，如果有数据相等则删除并释放内容
    rt_massage_list *pos;
    rt_massage_list *temp;
    rt_list_t node;
    rt_list_for_each_entry_safe(pos, temp, &sendMsgList.node, node)
    {
        if(0 == rt_memcmp(data, pos->data, len)) {
            //3.删除节点并释放空间
            rt_list_remove(&pos->node);
            rt_free(pos);
            return RT_EOK;
        }
    }

    return RT_ERROR;
}

/**
 **处理发送列表，并将处理过的数据移出列表
 */
void SendMsgHandle(void)
{
    //1.判断列表是否为空
    if(0 == GetSendMsgListSize()) {
        return;
    }

    //2.取出首组数据并处理
    rt_list_t *list = &sendMsgList.node;
    rt_list_t node;
    rt_massage_list *first = rt_list_first_entry(list, rt_massage_list, node);
    if(RT_NULL != first) {
        if(first->len) {
        //3.处理数据
//        rt_mutex_take(eth_dynamic_mutex, RT_WAITING_FOREVER);
        SendToMaster(first->data, first->len);
//        rt_mutex_release(eth_dynamic_mutex);
        //4.数据已处理，移出列表
//        rt_kprintf("send data : ");
//        for(int i = 0; i < first->len; i++){
//            rt_kprintf(" %x",first->data[i]);
//        }
//        rt_kprintf("\n");//Justin

            RemoveFromSendMsgList(first->data, MSG_SIZE);
        }
    }
}

/**
 ** 初始化发送数据列表的链表结构
 * @param data
 * @param len
 * @return
 */
void InitRecvMsgList(void)
{
    rt_list_init(&recvMsgList.node);
}

/**
 **获取接收列表的长度
 * @return
 */
u16 GetRecvMsgListSize(void)
{
    return rt_list_len(&recvMsgList.node);
}

/**
 **加入到需要发送的列表中
 * @param data
 * @param len
 * @return
 */
rt_err_t AddToRecvMsgList(u8 *data, u8 len)
{
    u16                 listRealSize    = 0;
    rt_massage_list     *newMsg         = RT_NULL;

    //1.发送数据长度检验
    if((len > MSG_SIZE) || (GetRecvMsgListSize() >= MSG_LIST_MAX)) {
        return RT_ERROR;
    }

    //2.申请数据空间
    newMsg = rt_calloc(1, sizeof(rt_massage_list));

    //3.遍历列表，如果没有相同内容则加入列表
    if(RT_NULL != newMsg) {
        //3.1 遍历列表
        listRealSize = rt_list_len(&recvMsgList.node);

        if(0 == listRealSize) {
            newMsg->len = len;
            rt_memcpy(newMsg->data, data, len);
            rt_list_insert_after(&recvMsgList.node, &newMsg->node);
        }
        else {
            rt_massage_list *temp;
            rt_list_t node;
            rt_list_for_each_entry(temp, &recvMsgList.node, node)
            {
                if(0 == rt_memcmp(data, temp->data, len))
                {
                    //3.2 列表中有相同的数据,不添加,并释放申请空间
                    rt_free(newMsg);
                    return RT_ERROR;
                }
            }
            //3.3添加到列表中
            newMsg->len = len;
            rt_memcpy(newMsg->data, data, len);
            rt_list_insert_after(&recvMsgList.node, &newMsg->node);
        }

        return RT_EOK;
    }
    else {
        return RT_ERROR;
    }
}

/**
 **移除节点
 * @param data 数据内容
 * @param len  数据长度
 * @return
 */
rt_err_t RemoveFromRecvMsgList(u8 *data, u8 len)
{
    //1.如果数据为空或者长度异常大则返回错误
    if((RT_NULL == data) ||
       (len > MSG_SIZE || 0 == len)) {
        return RT_ERROR;
    }

    //2.遍历列表，如果有数据相等则删除并释放内容
    rt_massage_list *pos;
    rt_massage_list *temp;
    rt_list_t node;
    rt_list_for_each_entry_safe(pos, temp, &recvMsgList.node, node)
    {
        if(0 == rt_memcmp(data, pos->data, len)) {
            //3.删除节点并释放空间
            rt_list_remove(&pos->node);
            rt_free(pos);
            return RT_EOK;
        }
    }

    return RT_ERROR;
}
