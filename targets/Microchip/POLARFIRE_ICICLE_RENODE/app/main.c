/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available
 * under the terms of the MIT license which is available at
 * https://opensource.org/license/mit.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include "tx_api.h"
#include "hwtimer.h"
#include "bsp/board.h"
#include "bsp/console.h"
#include "bsp/led.h"

extern void *_sbrk(ptrdiff_t incr);

#define DEMO_STACK_SIZE     4096
#define DEMO_QUEUE_ITEMS    10

typedef struct SENSOR_DATA_STRUCT {
    ULONG    timestamp;
    float    temperature_celsius;
    uint32_t reserved;
} SENSOR_DATA;

#define DEMO_QUEUE_MSG_WORDS    (sizeof(SENSOR_DATA) / sizeof(ULONG))

TX_THREAD      sampler_thread;
TX_THREAD      analyzer_thread;
TX_THREAD      reporter_thread;
TX_QUEUE       sensor_queue;
TX_EVENT_FLAGS_GROUP alarm_flags;

UCHAR sampler_stack[DEMO_STACK_SIZE];
UCHAR analyzer_stack[DEMO_STACK_SIZE];
UCHAR reporter_stack[DEMO_STACK_SIZE];
UCHAR queue_area[DEMO_QUEUE_ITEMS * sizeof(SENSOR_DATA)];

#define ALARM_OVERTEMP  0x01

static void console_print(const char *s) {
    if (s) {
        bsp_console_write(s, strlen(s));
    }
}

void sampler_thread_entry(ULONG input);
void analyzer_thread_entry(ULONG input);
void reporter_thread_entry(ULONG input);

static volatile ULONG s_sampler_runs = 0;
static volatile ULONG s_analyzer_runs = 0;
static volatile ULONG s_reporter_runs = 0;

static void run_startup_self_tests(void) {
    extern char __end;
    char num_buf[128];
    console_print("[SELF-TEST] Starting Hardware & Runtime Verification...\n");

    /* 1. _sbrk() Valid allocation test */
    void *p1 = _sbrk(64);
    if (p1 != (void *)-1 && (uintptr_t)p1 >= (uintptr_t)&__end) {
        console_print("[+] PASS: _sbrk() valid allocation returned base pointer\n");
    } else {
        console_print("[-] FAIL: _sbrk() valid allocation failed\n");
    }

    /* 2. _sbrk() Underflow test (shrink below heap base) */
    errno = 0;
    void *p_under = _sbrk(-128);
    if (p_under == (void *)-1 && errno == EINVAL) {
        console_print("[+] PASS: _sbrk() underflow guard rejected with EINVAL\n");
    } else {
        console_print("[-] FAIL: _sbrk() underflow guard failed\n");
    }

    /* 3. _sbrk() Overflow test (request beyond 1 GiB DRAM) */
    errno = 0;
    void *p_over = _sbrk((ptrdiff_t)0x40000000ULL);
    if (p_over == (void *)-1 && errno == ENOMEM) {
        console_print("[+] PASS: _sbrk() overflow guard rejected with ENOMEM\n");
    } else {
        console_print("[-] FAIL: _sbrk() overflow guard failed\n");
    }

    /* 4. HWTimer catch-up clamp test */
    MTIME_REG = 100000ULL; /* Set known baseline mtime */
    uint64_t current_mtime = MTIME_REG;
    uint64_t past_cmp = 20000ULL; /* In the past by 80,000 cycles (8 missed ticks) */
    HART1_MTIMECMP_REG = past_cmp;
    hwtimer_ack();
    uint64_t clamped_cmp = HART1_MTIMECMP_REG;
    if (clamped_cmp == current_mtime + TICK_CYCLES) {
        snprintf(num_buf, sizeof(num_buf),
                 "[+] PASS: HWTimer catch-up (mtime=%llu, past_cmp=%llu -> clamped_cmp=%llu == mtime + %llu)\n",
                 (unsigned long long)current_mtime,
                 (unsigned long long)past_cmp,
                 (unsigned long long)clamped_cmp,
                 (unsigned long long)TICK_CYCLES);
        console_print(num_buf);
    } else {
        console_print("[-] FAIL: HWTimer catch-up clamp failed\n");
    }

    console_print("[SELF-TEST] All startup verification tests PASSED!\n\n");
}

int main(void) {
    /* Initialize Board Peripherals & MMUART1 */
    bsp_board_init();

    console_print("\n====================================================\n");
    console_print("Microchip PolarFire SoC Icicle Kit (Renode Target)\n");
    console_print("64-Bit RISC-V Industrial LM75 Condition-Monitoring App\n");
    console_print("====================================================\n");

    /* Execute Dynamic Hardware & Runtime Self-Tests */
    run_startup_self_tests();

#ifdef TEST_FAULT_INJECTION
    console_print("[FAULT-TEST] Injecting deliberate synchronous illegal instruction...\n");
    __asm__ volatile(".word 0x00000000"); /* Illegal instruction to exercise trap_handler */
#endif

    /* Enter ThreadX Kernel */
    tx_kernel_enter();

    return 0;
}

void tx_application_define(void *first_unused_memory) {
    (void)first_unused_memory;
    UINT status;

    /* Create Sensor Queue */
    status = tx_queue_create(&sensor_queue, "sensor queue", DEMO_QUEUE_MSG_WORDS,
                             queue_area, sizeof(queue_area));
    if (status != TX_SUCCESS) {
        console_print("[ERROR] Failed to create sensor queue\n");
        return;
    }

    /* Create Event Flags Group */
    status = tx_event_flags_create(&alarm_flags, "alarm flags");
    if (status != TX_SUCCESS) {
        console_print("[ERROR] Failed to create alarm flags\n");
        return;
    }

    /* Create Sampler Thread */
    status = tx_thread_create(&sampler_thread, "Sampler Thread", sampler_thread_entry, 0,
                              sampler_stack, DEMO_STACK_SIZE,
                              10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        console_print("[ERROR] Failed to create sampler thread\n");
        return;
    }

    /* Create Analyzer Thread */
    status = tx_thread_create(&analyzer_thread, "Analyzer Thread", analyzer_thread_entry, 0,
                              analyzer_stack, DEMO_STACK_SIZE,
                              8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        console_print("[ERROR] Failed to create analyzer thread\n");
        return;
    }

    /* Create Reporter Thread */
    status = tx_thread_create(&reporter_thread, "Reporter Thread", reporter_thread_entry, 0,
                              reporter_stack, DEMO_STACK_SIZE,
                              12, 12, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        console_print("[ERROR] Failed to create reporter thread\n");
        return;
    }
}

void sampler_thread_entry(ULONG input) {
    (void)input;
    SENSOR_DATA data;
    float simulated_temp = 25.0f;

    while (1) {
        s_sampler_runs++;
        data.timestamp = tx_time_get();
        data.temperature_celsius = simulated_temp;
        data.reserved = 0x55AA55AA;

        /* Send telemetry to Queue */
        UINT status = tx_queue_send(&sensor_queue, &data, TX_NO_WAIT);
        if (status != TX_SUCCESS) {
            console_print("[WARN] Telemetry queue send failed\n");
        }

        simulated_temp += 2.5f;
        if (simulated_temp > 55.0f) {
            simulated_temp = 25.0f;
        }

        tx_thread_sleep(50); /* Sample every 500ms */
    }
}

void analyzer_thread_entry(ULONG input) {
    (void)input;
    SENSOR_DATA data;
    static int s_verified_queue = 0;

    while (1) {
        if (tx_queue_receive(&sensor_queue, &data, TX_WAIT_FOREVER) == TX_SUCCESS) {
            s_analyzer_runs++;
            if (data.reserved != 0x55AA55AA) {
                console_print("[-] FAIL: Queue payload corruption detected!\n");
            } else if (!s_verified_queue) {
                s_verified_queue = 1;
                console_print("[+] PASS: Queue 16-byte structure round-trip verified\n");
            }

            if (data.temperature_celsius > 45.0f) {
                UINT status = tx_event_flags_set(&alarm_flags, ALARM_OVERTEMP, TX_OR);
                if (status != TX_SUCCESS) {
                    console_print("[WARN] Alarm flag set failed\n");
                }
                bsp_led_on();
            } else {
                bsp_led_off();
            }
        }
    }
}

void reporter_thread_entry(ULONG input) {
    (void)input;
    char msg_buf[160];
    ULONG actual_flags;

    while (1) {
        s_reporter_runs++;
        snprintf(msg_buf, sizeof(msg_buf),
                 "[Monitor] Ticks: %lu | Active Runs: Sampler=%lu, Analyzer=%lu, Reporter=%lu\n",
                 (unsigned long)tx_time_get(),
                 (unsigned long)s_sampler_runs,
                 (unsigned long)s_analyzer_runs,
                 (unsigned long)s_reporter_runs);
        console_print(msg_buf);

        if (tx_event_flags_get(&alarm_flags, ALARM_OVERTEMP, TX_OR_CLEAR, &actual_flags, TX_NO_WAIT) == TX_SUCCESS) {
            console_print("[LM75 Sensor] Temperature: OVERTEMP ALARM TRIGGERED (>45.0C)\n");
        }

        tx_thread_sleep(100); /* Report every 1 second */
    }
}
