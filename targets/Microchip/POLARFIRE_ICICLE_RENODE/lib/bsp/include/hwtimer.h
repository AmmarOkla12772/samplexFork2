/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef POLARFIRE_HWTIMER_H
#define POLARFIRE_HWTIMER_H

#include <stdint.h>

#define CLINT_BASE           0x02000000ULL
#define MTIME_REG            (*(volatile uint64_t *)(CLINT_BASE + 0xBFF8))
#define HART1_MTIMECMP_REG   (*(volatile uint64_t *)(CLINT_BASE + 0x4008))

#define CLINT_TIME_FREQ_HZ   1000000ULL /* 1 MHz clock in Renode */
#define THREADX_TICK_RATE_HZ 100ULL     /* 100 Hz = 10ms tick interval */
#define TICK_CYCLES          (CLINT_TIME_FREQ_HZ / THREADX_TICK_RATE_HZ) /* 10,000 cycles */

void hwtimer_init(void);
void hwtimer_ack(void);

#endif /* POLARFIRE_HWTIMER_H */
