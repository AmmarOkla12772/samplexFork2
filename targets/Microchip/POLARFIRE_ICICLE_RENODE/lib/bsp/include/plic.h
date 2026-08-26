/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/licenses/MIT.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PLIC_H
#define PLIC_H

#include <stdint.h>

/* SiFive PLIC Base Address on Microchip PolarFire SoC Icicle Kit */
#define PLIC_BASE                       0x0C000000ULL

/* Hart 1 Machine-Mode (Context 2 on PolarFire SoC) Control Registers */
#define PLIC_HART1_M_THRESHOLD_REG      (*(volatile uint32_t *)(PLIC_BASE + 0x202000ULL))
#define PLIC_HART1_M_CLAIM_REG          (*(volatile uint32_t *)(PLIC_BASE + 0x202004ULL))

/* Hart 1 Machine-Mode (Context 2) Interrupt Enable Bitmap for IRQ 64..95 */
#define PLIC_HART1_M_ENABLE_REG2        (*(volatile uint32_t *)(PLIC_BASE + 0x2000ULL + (2 * 0x80ULL) + (2 * 4ULL)))

/* Interrupt Source Priority Register (1..186) */
#define PLIC_PRIORITY_REG(irq)          (*(volatile uint32_t *)(PLIC_BASE + ((irq) * 4ULL)))

/* Microchip PolarFire SoC MMUART1 PLIC Source ID */
#define MMUART1_IRQ                     91

void plic_init(void);
uint32_t plic_claim(void);
void plic_complete(uint32_t irq);

#endif /* PLIC_H */
