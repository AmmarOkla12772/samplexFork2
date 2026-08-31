/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/stat.h>

#include "board_config.h"
#include "stm32f4xx.h"

extern int _end;

void *_sbrk(ptrdiff_t incr)
{
    static uint8_t *heap_ptr = NULL;
    uint8_t *prev_heap;

    /* Mask interrupts to ensure thread-safe heap pointer mutation on Cortex-M4 */
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (heap_ptr == NULL) {
        heap_ptr = (uint8_t *)&_end;
    }

    /* Reject negative increments below heap base */
    if (incr < 0) {
        if ((heap_ptr + incr) < (uint8_t *)&_end) {
            __set_PRIMASK(primask);
            errno = EINVAL;
            return (void *)-1;
        }
    }

    /* Guard against SRAM upper bound overflow */
    if ((uintptr_t)(heap_ptr + incr) > (uintptr_t)BSP_RAM_END) {
        __set_PRIMASK(primask);
        errno = ENOMEM;
        return (void *)-1;
    }

    prev_heap = heap_ptr;
    heap_ptr += incr;

    __set_PRIMASK(primask);
    return (void *)prev_heap;
}

int _close(int file)
{
    (void)file;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

void _exit(int status)
{
    (void)status;
    __disable_irq();
    while (1) {
        __WFI();
    }
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int _getpid(void)
{
    return 1;
}
