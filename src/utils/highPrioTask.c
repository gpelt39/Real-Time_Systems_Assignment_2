#include "utils/highPrioTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "utils/traces/traces.h"
#include "utils/delay.h"

void highPrioTask(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(100));
    
    const TickType_t period = pdMS_TO_TICKS(700);
    TickType_t previousWakeTime = xTaskGetTickCount();

    uint32_t taskID = 3;
    for (;;)
    {
        // record the time at which the task was woken up
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // do the task
        busyDelay(100);
        // record when the task finished
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // wait for a period (measured from the last wake up time)
        // note that the first parameter is updated by vTaskDelayUntil to the time when the function terminates
        vTaskDelayUntil(&previousWakeTime, period);
    }
}

void addHighPriorityTask() {
    xTaskCreate(highPrioTask, "high prio task", 8192, NULL, 20, NULL);
}