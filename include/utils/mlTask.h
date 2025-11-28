#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "utils/traces/traces.h"
#include "utils/delay.h"

void ml_task() {
    const TickType_t period = pdMS_TO_TICKS(20000);
    TickType_t previousWakeTime = xTaskGetTickCount();

    uint32_t taskID = 17;
    for (;;)
    {
        // record the time at which the task was woken up
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // do the task
        printf("Doing ML stuff...\n");
        busyDelay(4000);
        printf("Done with ML stuff!\n");
        // record when the task finished
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // wait for a period (measured from the last wake up time)
        // note that the first parameter is updated by vTaskDelayUntil to the time when the function terminates
        vTaskDelayUntil(&previousWakeTime, period);
    }
}


void addMLTask() {
    xTaskCreate(ml_task, "high", 4096, NULL, 0, NULL);
}
