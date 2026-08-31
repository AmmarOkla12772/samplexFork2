/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#include "bsp/console.h"
#include "board_config.h"
#include "stm32f4xx_hal.h"

UART_HandleTypeDef UartHandle;

void bsp_console_init(void)
{
#if BSP_HAS_CONSOLE
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    UartHandle.Instance          = USART2;
    UartHandle.Init.BaudRate     = BSP_UART_BAUDRATE;
    UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits     = UART_STOPBITS_1;
    UartHandle.Init.Parity       = UART_PARITY_NONE;
    UartHandle.Init.Mode         = UART_MODE_TX_RX;
    UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&UartHandle);
#endif
}

void bsp_console_write(const char *data, size_t length)
{
#if BSP_HAS_CONSOLE
    if (data == NULL || length == 0) {
        return;
    }
    HAL_UART_Transmit(&UartHandle, (uint8_t *)data, (uint16_t)length, HAL_MAX_DELAY);
#else
    (void)data;
    (void)length;
#endif
}

int __io_putchar(int ch)
{
#if BSP_HAS_CONSOLE
    uint8_t c = (uint8_t)ch;
    HAL_UART_Transmit(&UartHandle, &c, 1, HAL_MAX_DELAY);
    return ch;
#else
    (void)ch;
    return ch;
#endif
}

int __io_getchar(void)
{
#if BSP_HAS_CONSOLE
    uint8_t ch = 0;
    HAL_UART_Receive(&UartHandle, &ch, 1, HAL_MAX_DELAY);
    return (int)ch;
#else
    return -1;
#endif
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++) {
        ptr[i] = (char)__io_getchar();
    }
    return len;
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    bsp_console_write(ptr, (size_t)len);
    return len;
}
