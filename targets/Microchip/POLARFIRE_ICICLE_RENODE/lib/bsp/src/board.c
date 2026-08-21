/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#include "uart.h"
#include "hwtimer.h"
#include "plic.h"

void board_init(void) {
    /* 1. Initialize MMUART1 at 0x20100000 */
    uart_init();

    /* 2. Initialize SiFive PLIC interrupt controller */
    plic_init();

    /* 3. Initialize 64-bit MTIME machine timer (10ms tick interval) */
    hwtimer_init();
}
