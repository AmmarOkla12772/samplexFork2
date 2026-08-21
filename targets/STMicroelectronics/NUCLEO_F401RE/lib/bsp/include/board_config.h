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

#define BSP_SYSTEM_CLOCK_HZ 84000000
#define BSP_UART_BAUDRATE 115200

/* End of SRAM (SRAM starts at 0x20000000, size is 96KB) */
#define BSP_RAM_END 0x20018000

#endif /* BOARD_CONFIG_H */
