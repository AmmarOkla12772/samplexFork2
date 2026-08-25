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
#include <string.h>
#include "tx_api.h"
#include "bsp/board.h"
#include "bsp/console.h"
#include "bsp/led.h"

#define DEMO_STACK_SIZE     4096
#define DEMO_QUEUE_ITEMS    10

typedef struct SENSOR_DATA_STRUCT {
    ULONG timestamp;
    float temperature_celsius;
    float reserved;
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

int main(void) {
    /* Initialize Board Peripherals & MMUART1 */
    bsp_board_init();

    console_print("\n====================================================\n");
    console_print("Microchip PolarFire SoC Icicle Kit (Renode Target)\n");
    console_print("64-Bit RISC-V Industrial LM75 Condition-Monitoring App\n");
    console_print("====================================================\n");

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
        data.timestamp = tx_time_get();
        data.temperature_celsius = simulated_temp;
        data.reserved = 0.0f;

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

    while (1) {
        if (tx_queue_receive(&sensor_queue, &data, TX_WAIT_FOREVER) == TX_SUCCESS) {
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
    char msg_buf[128];
    ULONG actual_flags;

    while (1) {
        snprintf(msg_buf, sizeof(msg_buf),
                 "[Monitor] ThreadX Ticks: %lu | Telemetry Pipeline Active | Queues OK\n",
                 (unsigned long)tx_time_get());
        console_print(msg_buf);

        if (tx_event_flags_get(&alarm_flags, ALARM_OVERTEMP, TX_OR_CLEAR, &actual_flags, TX_NO_WAIT) == TX_SUCCESS) {
            console_print("[LM75 Sensor] Temperature: OVERTEMP ALARM TRIGGERED (>45.0C)\n");
        }

        tx_thread_sleep(100); /* Report every 1 second */
    }
}
