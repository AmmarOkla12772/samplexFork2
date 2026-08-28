/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/licenses/MIT.
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
    uint64_t current_mtime = MTIME_REG;
    uint64_t next_cmp = HART1_MTIMECMP_REG + TICK_CYCLES;

    /* Clamp to current_mtime + TICK_CYCLES if timer fell behind */
    if (next_cmp <= current_mtime) {
        next_cmp = current_mtime + TICK_CYCLES;
    }

    HART1_MTIMECMP_REG = next_cmp;
}
