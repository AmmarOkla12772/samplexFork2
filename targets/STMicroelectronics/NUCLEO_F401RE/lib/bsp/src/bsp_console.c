/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#include "stm32f4xx_hal.h"
#include "bsp/console.h"
#include "board_config.h"

/* Global UART handle for Newlib syscall stubs to access */
UART_HandleTypeDef UartHandle;

void bsp_console_init(void)
{
    UartHandle.Instance          = USART2;
    UartHandle.Init.BaudRate     = BSP_UART_BAUDRATE;
    UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits     = UART_STOPBITS_1;
    UartHandle.Init.Parity       = UART_PARITY_NONE;
    UartHandle.Init.Mode         = UART_MODE_TX_RX;
    UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&UartHandle);
}

void bsp_console_write(const char *data, size_t length)
{
    HAL_UART_Transmit(&UartHandle, (uint8_t *)data, (uint16_t)length, HAL_MAX_DELAY);
}

/**
 * HAL UART MSP Initialization
 */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef gpio = {0};

    if (huart->Instance == USART2)
    {
        /* Enable USART2 and GPIOA clock */
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* USART2 TX=PA2, RX=PA3 alternate function */
        gpio.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
        gpio.Mode      = GPIO_MODE_AF_PP;
        gpio.Pull      = GPIO_PULLUP;
        gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &gpio);
    }
}

/**
 * HAL UART MSP De-Initialization
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
    }
}
