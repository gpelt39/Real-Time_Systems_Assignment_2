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

// Global variables
typedef struct {
    uint32_t missed;
    uint32_t met;
} TaskStats;

TaskStats task_stats[4] = { {0,0}, {0,0}, {0,0}, {0,0} };

// Tasks
void vTask1(void *pvParameters){
    TickType_t xLastWakeTime;
    TickType_t deadline;
    const TickType_t absolute_deadline = pdMS_TO_TICKS(40);
    const TickType_t job_execution_time = pdMS_TO_TICKS(10);
    const uint32_t taskID = 1;
    const TickType_t xFrequency = pdMS_TO_TICKS(50);
    xLastWakeTime = xTaskGetTickCount();

    for(;;){
        // record the time at which the task started the execution of a job
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // Do stuff...
        busyDelay(job_execution_time);
        // Code to detect misses  
        deadline = xLastWakeTime + absolute_deadline;      
        if (xTaskGetTickCount() > deadline) {
            task_stats[taskID-1].missed++;
        } else {
            task_stats[taskID-1].met++;
        }
        // record the time at which the task completed the execution of a job
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        xTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    // should never reach here
    vTaskDelete(NULL);
}

void vTask2(void *pvParameters){
    TickType_t xLastWakeTime;
    TickType_t deadline;
    const TickType_t absolute_deadline = pdMS_TO_TICKS(80);
    const TickType_t job_execution_time = pdMS_TO_TICKS(20);
    const uint32_t taskID = 2;
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    xLastWakeTime = xTaskGetTickCount();

    for(;;){
        // record the time at which the task started the execution of a job
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // Do stuff...
        busyDelay(job_execution_time);
        // Code to detect misses  
        deadline = xLastWakeTime + absolute_deadline;      
        if (xTaskGetTickCount() > deadline) {
            task_stats[taskID-1].missed++;
        } else {
            task_stats[taskID-1].met++;
        }
        // record the time at which the task completed the execution of a job
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        xTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    // should never reach here
    vTaskDelete(NULL);
}

void vTask3(void *pvParameters){
    TickType_t xLastWakeTime;
    TickType_t deadline;
    const TickType_t absolute_deadline = pdMS_TO_TICKS(150);
    const TickType_t job_execution_time = pdMS_TO_TICKS(30);
    const uint32_t taskID = 3;
    const TickType_t xFrequency = pdMS_TO_TICKS(150);
    xLastWakeTime = xTaskGetTickCount();

    for(;;){
        // record the time at which the task started the execution of a job
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // Do stuff...
        busyDelay(job_execution_time);
        // Code to detect misses  
        deadline = xLastWakeTime + absolute_deadline;      
        if (xTaskGetTickCount() > deadline) {
            task_stats[taskID-1].missed++;
        } else {
            task_stats[taskID-1].met++;
        }
        // record the time at which the task completed the execution of a job
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        xTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    // should never reach here
    vTaskDelete(NULL);
}

void vTask4(void *pvParameters){
    TickType_t xLastWakeTime;
    TickType_t deadline;
    const TickType_t absolute_deadline = pdMS_TO_TICKS(140);
    const TickType_t job_execution_time = pdMS_TO_TICKS(60);
    const uint32_t taskID = 4;
    const TickType_t xFrequency = pdMS_TO_TICKS(300);
    xLastWakeTime = xTaskGetTickCount();

    for(;;){
        // record the time at which the task started the execution of a job
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // Do stuff...
        busyDelay(job_execution_time);
        // Code to detect misses  
        deadline = xLastWakeTime + absolute_deadline;      
        if (xTaskGetTickCount() > deadline) {
            task_stats[taskID-1].missed++;
        } else {
            task_stats[taskID-1].met++;
        }
        // record the time at which the task completed the execution of a job
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        xTaskDelayUntil(&xLastWakeTime, xFrequency);
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
                // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
                // initialize the logger and start the logging task
                initLogger();
                // start a high priority task running at priority 20
                // addHighPriorityTask();
                
                // create task 1-4
                xTaskCreate(vTask1, "Task 1", 256, NULL, 3, NULL);
                xTaskCreate(vTask2, "Task 2", 256, NULL, 2, NULL);
                xTaskCreate(vTask3, "Task 3", 256, NULL, 1, NULL);
                xTaskCreate(vTask4, "Task 4", 256, NULL, 0, NULL);
                
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

