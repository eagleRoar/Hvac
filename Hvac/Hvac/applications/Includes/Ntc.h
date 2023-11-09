/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-11     12154       the first version
 */
#ifndef APPLICATIONS_INCLUDES_NTC_H_
#define APPLICATIONS_INCLUDES_NTC_H_

#include "GlobalConfig.h"

typedef struct ntcList{
    int data;
    rt_slist_t node;
}ntc_list_t;

int GetNtcValue(void);

#endif /* APPLICATIONS_INCLUDES_NTC_H_ */
