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

void hwtimer_init(void) {
    /* Program initial mtimecmp target to current mtime + 10,000 cycles (10ms) */
    uint64_t current_mtime = MTIME_REG;
    HART1_MTIMECMP_REG = current_mtime + TICK_CYCLES;
}

void hwtimer_ack(void) {
    /* Advance mtimecmp by 10,000 cycles for the next tick */
    HART1_MTIMECMP_REG = HART1_MTIMECMP_REG + TICK_CYCLES;
}
