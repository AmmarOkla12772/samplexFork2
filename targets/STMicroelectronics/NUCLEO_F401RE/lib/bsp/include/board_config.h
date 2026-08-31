/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

/* CPU Subsystem Frequencies */
#define BSP_CPU_CLOCK_HZ        84000000UL /* 84 MHz Cortex-M4 SYSCLK */
#define BSP_SYSTEM_CLOCK_HZ     BSP_CPU_CLOCK_HZ

#define BSP_UART_BAUDRATE       115200

/* SRAM Memory Limits (STM32F401RE: 96 KB SRAM from 0x20000000 to 0x20018000) */
#define BSP_RAM_START           0x20000000UL
#define BSP_RAM_SIZE            0x00018000UL /* 96 KB */
#define BSP_RAM_END             (BSP_RAM_START + BSP_RAM_SIZE)

#define BSP_HAS_LED             1
#define BSP_HAS_CONSOLE         1

#endif /* BOARD_CONFIG_H */
