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
#include "board_config.h"

#define CLINT_BASE           0x02000000ULL
#define MTIME_REG            (*(volatile uint64_t *)(CLINT_BASE + 0xBFF8))
#define HART1_MTIMECMP_REG   (*(volatile uint64_t *)(CLINT_BASE + 0x4008))

#ifndef TX_TIMER_TICKS_PER_SECOND
#define TX_TIMER_TICKS_PER_SECOND 100ULL
#endif

#define TICK_CYCLES          (BSP_CLINT_RTC_FREQ_HZ / (uint64_t)TX_TIMER_TICKS_PER_SECOND)

void hwtimer_init(void);
void hwtimer_ack(void);

#endif /* POLARFIRE_HWTIMER_H */
