/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#include "utils/traces/traces.h"
#include "utils/highPrioTask.h"
#include "utils/delay.h"

// function that makes the LED blink
void vLedBlink(void *pvParameters) {
    const uint32_t taskID = 1;
    const TickType_t xFrequency = pdMS_TO_TICKS(2000);
    for (;;) {
        // record the time at which the task started the execution of a job
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // switch the LED on
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        // busy wait for 200 ms
        busyDelay(pdMS_TO_TICKS(200));
        // switch the LED off
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        // busy wait for 200 ms
        busyDelay(pdMS_TO_TICKS(200));
        // record the time at which the task completed the execution of a job
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        vTaskDelay(xFrequency);
    }
    // should never reach here
    vTaskDelete(NULL);
}

int main() {
    stdio_init_all();
    // wait for the usb connection to be completed
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Connected to Raspberry Pi Pico\n");

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return -1;
    }   

    printf("Menu:\ns -> start the scheduler\nq -> quit\n");

    int input_char;

    while (true) {
        input_char = getchar();
        switch (input_char) {
            case 's':
                // turn off the LED before starting the scheduler
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
                // initialize the logger and start the logging task
                initLogger();
                // start a high priority task running at priority 20
                // addHighPriorityTask();
                
                // create the LED blink task (executing vLedBlink at priority 2)
                // xTaskCreate(...);
                
                // start the scheduler
                printf("Scheduler started\n");
                // ...
                break;
            case 'q':
                printf("Program stopped\n");
                cyw43_arch_deinit();
                return 0;
            default:
                printf("Invalid input\n");
        }
    }
    return 0;
}

