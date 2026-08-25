/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <sys/stat.h>
#include "board_config.h"
#include "bsp/console.h"

extern char __end; /* Symbol set by linker at end of BSS / top of boot stack */
static char *heap_ptr = NULL;

static inline uintptr_t disable_interrupts(void) {
    uintptr_t mstatus;
    __asm__ volatile("csrrci %0, mstatus, 8" : "=r"(mstatus));
    return mstatus;
}

static inline void restore_interrupts(uintptr_t mstatus) {
    if (mstatus & 8) {
        __asm__ volatile("csrrs zero, mstatus, 8");
    }
}

void __malloc_lock(struct _reent *reent) {
    (void)reent;
}

void __malloc_unlock(struct _reent *reent) {
    (void)reent;
}

int _write(int file, char *ptr, int len) {
    (void)file;
    if (ptr == NULL || len <= 0)
        return 0;
    bsp_console_write(ptr, (size_t)len);
    return len;
}

void *_sbrk(ptrdiff_t incr) {
    char *prev_heap_ptr;
    uintptr_t mstatus = disable_interrupts();

    if (heap_ptr == NULL) {
        heap_ptr = &__end;
    }

    if (incr > 0) {
        if ((uintptr_t)heap_ptr + (uintptr_t)incr > (uintptr_t)BSP_RAM_END ||
            (uintptr_t)heap_ptr + (uintptr_t)incr < (uintptr_t)heap_ptr) {
            restore_interrupts(mstatus);
            errno = ENOMEM;
            return (void *)-1;
        }
    } else if (incr < 0) {
        if ((uintptr_t)heap_ptr < (uintptr_t)&__end + (uintptr_t)(-incr)) {
            restore_interrupts(mstatus);
            errno = EINVAL;
            return (void *)-1;
        }
    }

    prev_heap_ptr = heap_ptr;
    heap_ptr += incr;

    restore_interrupts(mstatus);
    return (void *)prev_heap_ptr;
}

int _read(int file, char *ptr, int len) {
  (void)file;
  (void)ptr;
  (void)len;
  return 0;
}

int _close(int file) {
  (void)file;
  return -1;
}

int _fstat(int file, struct stat *st) {
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty(int file) {
  (void)file;
  return 1;
}

int _lseek(int file, int ptr, int dir) {
  (void)file;
  (void)ptr;
  (void)dir;
  return 0;
}

void _exit(int status) {
  (void)status;
  while (1) {
    /* Hang on fatal exit */
  }
}
