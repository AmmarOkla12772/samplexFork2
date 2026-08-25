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

void board_init(void) {
    /* Initialize 64-bit MTIME machine timer (10ms tick interval) */
    hwtimer_init();
}
