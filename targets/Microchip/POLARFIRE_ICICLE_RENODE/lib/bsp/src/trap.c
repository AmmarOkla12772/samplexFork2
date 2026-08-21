/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/licenses/MIT.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include "hwtimer.h"
#include "plic.h"

extern void _tx_timer_interrupt(void);

void trap_handler(uint64_t mcause, uint64_t mepc, uint64_t mtval) {
    (void)mepc;
    (void)mtval;

    /* Check if trap is an interrupt (high bit set) */
    if (mcause & (1ULL << 63)) {
        uint64_t irq = mcause & 0xFFULL;
        if (irq == 7) {
            /* Machine Timer Interrupt */
            hwtimer_ack();
            _tx_timer_interrupt();
        } else if (irq == 11) {
            /* Machine External Interrupt (PLIC) */
            uint32_t plic_id = plic_claim();
            if (plic_id > 0) {
                plic_complete(plic_id);
            }
        }
    }
}
