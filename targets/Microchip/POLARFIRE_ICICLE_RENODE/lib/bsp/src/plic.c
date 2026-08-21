/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#include "plic.h"

void plic_init(void) {
    /* Set Hart 1 Machine Mode interrupt threshold to 0 (accept all priorities > 0) */
    PLIC_HART1_M_THRESHOLD_REG = 0;

    /* Set IRQ 91 priority to 1 */
    volatile uint32_t *priority_reg = (volatile uint32_t *)(PLIC_BASE + (MMUART1_PLIC_IRQ * 4));
    *priority_reg = 1;

    /* Enable IRQ 91 in Hart 1 Machine Mode enable register (Register index 91/32 = 2) */
    volatile uint32_t *enable_reg = (volatile uint32_t *)(PLIC_BASE + 0x0C002100ULL + (MMUART1_PLIC_IRQ / 32) * 4);
    *enable_reg |= (1U << (MMUART1_PLIC_IRQ % 32));
}

uint32_t plic_claim(void) {
    return PLIC_HART1_M_CLAIM_REG;
}

void plic_complete(uint32_t irq) {
    PLIC_HART1_M_CLAIM_REG = irq;
}
