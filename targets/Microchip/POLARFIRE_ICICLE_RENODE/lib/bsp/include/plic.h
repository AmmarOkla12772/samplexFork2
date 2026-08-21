/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef POLARFIRE_PLIC_H
#define POLARFIRE_PLIC_H

#include <stdint.h>

#define PLIC_BASE                     0x0C000000ULL
#define PLIC_HART1_M_THRESHOLD_REG    (*(volatile uint32_t *)(PLIC_BASE + 0x0C202000ULL))
#define PLIC_HART1_M_CLAIM_REG        (*(volatile uint32_t *)(PLIC_BASE + 0x0C202004ULL))
#define PLIC_HART1_M_ENABLE_REG0      (*(volatile uint32_t *)(PLIC_BASE + 0x0C002100ULL))

#define MMUART1_PLIC_IRQ              91

void plic_init(void);
uint32_t plic_claim(void);
void plic_complete(uint32_t irq);

#endif /* POLARFIRE_PLIC_H */
