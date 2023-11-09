/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-25     Administrator       the first version
 */
#ifndef APPLICATIONS_INCLUDES_UART_H_
#define APPLICATIONS_INCLUDES_UART_H_

#include "Gpio.h"
#include "GlobalConfig.h"



void UartInit(void);
rt_err_t Uart1_input(rt_device_t dev, rt_size_t size);
rt_err_t Uart3_input(rt_device_t dev, rt_size_t size);
#endif /* APPLICATIONS_INCLUDES_UART_H_ */
