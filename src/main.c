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
#include "utils/mlTask.h"

// Global variables
uint32_t counter = 0;
typedef enum State {Clear,Running,Paused} State;
State current_state = Clear;
State next_state = Clear;
typedef struct {
    uint32_t missed;
    uint32_t met;
} TaskStats;

// Indexes: 0 = Counter, 1 = Display, 2 = Input
TaskStats task_stats[3] = { {0,0}, {0,0}, {0,0} };

// Task functions
void vCounter(void *pvParameters){
    TickType_t xLastWakeTime;
    TickType_t deadline;
    const uint32_t taskID = 0;
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    xLastWakeTime = xTaskGetTickCount();

    for(;;){
        // record the time at which the task started the execution of a job
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        //Increase counter by 1 if in Running state and clear counter if in Clear state
        if (current_state == Running) {
            counter++;
        } else if (current_state == Clear) {
            counter = 0;
        }
        // Code to detect misses  
        deadline = xLastWakeTime + xFrequency;      
        if (xTaskGetTickCount() > deadline) {
            task_stats[taskID].missed++;
        } else {
            task_stats[taskID].met++;
        }
        busyDelay(2);
        // record the time at which the task completed the execution of a job
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        xTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    // should never reach here
    vTaskDelete(NULL);
}

void vDisplayTask(void *pvParameters){
    TickType_t xLastWakeTime;
    TickType_t deadline;
    const uint32_t taskID = 1;
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    xLastWakeTime = xTaskGetTickCount();
    for(;;){
        // record the time at which the task started the execution of a job
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // display the current state upon state change
        static State last_state = Clear;
        if (current_state != last_state) {
            switch (current_state) {
                case Clear:
                    printf("State: Clear\n");
                    break;
                case Running:
                    printf("State: Running\n");
                    break;
                case Paused:
                    printf("State: Paused\n");
                    break;
            }
            // Print number of deadline hits and misses when Paused or Cleared
            if (current_state == Paused || current_state == Clear) {
                printf("\nNumber of deadlines met and missed:\n");
                printf("Counter Task: Met=%u, Miss=%u\n", task_stats[0].met, task_stats[0].missed);
                printf("Display Task: Met=%u, Miss=%u\n", task_stats[1].met, task_stats[1].missed);
                printf("Input Task:   Met=%u, Miss=%u\n\n", task_stats[2].met, task_stats[2].missed);
            }
            last_state = current_state;
        }
        // If in Running state, display the counter value
        if (current_state != Clear) {
            printf("Counter: %u\n", counter);
        }
        // Code to detect misses  
        deadline = xLastWakeTime + xFrequency;      
        if (xTaskGetTickCount() > deadline) {
            task_stats[taskID].missed++;
        } else {
            task_stats[taskID].met++;
        }
        busyDelay(2);
        // record the time at which the task completed the execution of a job
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        xTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    // should never reach here
    vTaskDelete(NULL);
}

void vInput(void *pvParameters){
    TickType_t xLastWakeTime;
    TickType_t deadline;
    const uint32_t taskID = 2;
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    xLastWakeTime = xTaskGetTickCount();
    int input;
    for(;;){
        // record the time at which the task started the execution of a job
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // check for user input
        input = getchar_timeout_us(10);
        switch (current_state) {
            case Clear:
                if (input == ' ') {
                    next_state = Running;
                }
                break;
            case Running:
                if (input == ' ') {
                    next_state = Paused;
                }
                break;
            case Paused:
                if (input == ' ') {
                    next_state = Running;
                } else if (input == 'r') {
                    next_state = Clear;
                    counter = 0; // reset counter (maybe put this in counter function?)
                }
                break;
        }
        // update current state
        current_state = next_state;
        // Code to detect misses  
        deadline = xLastWakeTime + xFrequency;      
        if (xTaskGetTickCount() > deadline) {
            task_stats[taskID].missed++;
        } else {
            task_stats[taskID].met++;
        }
        busyDelay(2);
        // record the time at which the task completed the execution of a job
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        xTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    // should never reach here
    vTaskDelete(NULL);
}

// Main function
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
                // initialize the logger and start the logging task
                initLogger();
                // start a high priority task running at priority 20
                addMLTask();
                
                // create counter task, display task and input task
                xTaskCreate(vCounter, "Count", 256, NULL, 3, NULL);
                xTaskCreate(vDisplayTask, "Display", 256, NULL, 1, NULL);
                xTaskCreate(vInput, "Input", 256, NULL, 2, NULL);
                
                // start the scheduler
                printf("Scheduler started\n");
                vTaskStartScheduler();
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