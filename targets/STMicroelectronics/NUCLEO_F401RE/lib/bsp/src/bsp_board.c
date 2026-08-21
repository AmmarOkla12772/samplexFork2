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
#include "bsp/board.h"
#include "board_config.h"

static TIM_HandleTypeDef hal_tick_tim_handle;
static void SystemClock_Config(void);
static void Error_Handler(void);



void bsp_board_init(void)
{
    /* Core system and clock initialization */
    HAL_Init();
    SystemClock_Config();
    SystemCoreClockUpdate();
}

/**
 * Configure System Clocks: SYSCLK = 84 MHz from 8 MHz HSE Bypass (Nucleo ST-LINK MCO).
 * PLL: M=8, N=336, P=4 -> 84 MHz.
 */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_BYPASS;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM       = 8;
    osc.PLL.PLLN       = 336;
    osc.PLL.PLLP       = RCC_PLLP_DIV4;
    osc.PLL.PLLQ       = 7;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        Error_Handler();
    }

    clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

static void Error_Handler(void)
{
    while (1)
    {
    }
}

/**
 * Configure TIM2 for HAL tick timer so SysTick remains free for ThreadX.
 */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t tim_clock;
    uint32_t prescaler;
    uint32_t latency;

    HAL_NVIC_SetPriority(TIM2_IRQn, TickPriority, 0U);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    __HAL_RCC_TIM2_CLK_ENABLE();

    HAL_RCC_GetClockConfig(&clkconfig, &latency);
    tim_clock = (clkconfig.APB1CLKDivider == RCC_HCLK_DIV1)
                    ? HAL_RCC_GetPCLK1Freq()
                    : (2U * HAL_RCC_GetPCLK1Freq());

    prescaler = (tim_clock / 1000000U) - 1U;

    hal_tick_tim_handle.Instance               = TIM2;
    hal_tick_tim_handle.Init.Prescaler         = prescaler;
    hal_tick_tim_handle.Init.Period            = (1000000U / 1000U) - 1U;
    hal_tick_tim_handle.Init.ClockDivision     = 0U;
    hal_tick_tim_handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    hal_tick_tim_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&hal_tick_tim_handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    uwTickPrio = TickPriority;

    return HAL_TIM_Base_Start_IT(&hal_tick_tim_handle);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        HAL_IncTick();
    }
}

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&hal_tick_tim_handle);
}

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_DISABLE();
    }
}
