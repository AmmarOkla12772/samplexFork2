/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#include "hwtimer.h"
#include "plic.h"

void board_init(void) {
    /* Initialize 64-bit MTIME machine timer (10ms tick interval) */
    hwtimer_init();

    /* Initialize PLIC (Hart 1 Context 2, enable MMUART1 IRQ 91) */
    plic_init();
}
