/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#include "board_config.h"
#include "bsp/console.h"
#include <errno.h>
#include <sys/stat.h>

int _write(int file, char *ptr, int len) {
  (void)file;
  if (ptr == NULL || len <= 0)
    return 0;
  bsp_console_write(ptr, (size_t)len);
  return len;
}

void *_sbrk(int incr) {
  extern char __end; /* Symbol set by linker at end of BSS */
  static char *heap_ptr = NULL;
  char *prev_heap_ptr;

  if (heap_ptr == NULL) {
    heap_ptr = &__end;
  }
  prev_heap_ptr = heap_ptr;

  if ((uintptr_t)(heap_ptr + incr) > BSP_RAM_END) {
    errno = ENOMEM;
    return (void *)-1;
  }

  heap_ptr += incr;
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
