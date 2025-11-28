#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "utils/traces/traces.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int32_t logArray[MAX_LOG_SIZE][3];
volatile uint32_t logIndex = 0;
SemaphoreHandle_t logMutex = NULL;
SemaphoreHandle_t logSemaphore = NULL;
bool isFull = false;

void resetLogger() {
    logIndex = 0;
    isFull = false;
}

void logEvent(uint32_t taskNum, EventType event, uint32_t timestamp)
{
    if (logMutex == NULL) {
        return;
    }
    if (logIndex < MAX_LOG_SIZE){
        if (xSemaphoreTake(logMutex, portMAX_DELAY) == pdTRUE) {
            logArray[logIndex][0] = taskNum;
            logArray[logIndex][1] = event;
            logArray[logIndex][2] = timestamp; 

            logIndex++;  // Increment the log index 
            xSemaphoreGive(logMutex);
            // Check if watermark is reached
            if (logIndex == LOG_WATERMARK) {
                isFull = true;
                xSemaphoreGive(logSemaphore);
            }
        }
    }
    else {
        isFull = true;
        xSemaphoreGive(logSemaphore);
    }
}

void vLoggingTask(void *pvParameters) {
    const uint32_t taskID = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();  
    const TickType_t xExecutionPeriod = pdMS_TO_TICKS(LOGGING_PERIOD_MS);
    for (;;) {
        // Wait until either the semaphore is given or the timeout occurs
        xSemaphoreTake(logSemaphore, xExecutionPeriod);
        // record the time at which the task started the execution of a job
        logEvent(taskID, JOB_START, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        // Print the logs. Minimize the amount of time the mutex is held
        printf("====Log dump start====\n");
        uint32_t i = 0;
        for (; i < logIndex-1; i++) {
            printf("%d,%d,%d\n", logArray[i][0], logArray[i][1], logArray[i][2]);
        }
        // take the mutex only for the last entry to avoid holding it for too long
        // note that in most cases it will only be needed for the last entry
        // but in case of high logging frequency, there may have been new entries added
        // since we last checked `logIndex`
        if (xSemaphoreTake(logMutex, portMAX_DELAY) == pdTRUE) {
            for (; i < logIndex; i++) {
                printf("%d,%d,%d\n", logArray[i][0], logArray[i][1], logArray[i][2]);
            }
            printf("====Log dump end====\n");
            resetLogger();
            xSemaphoreGive(logMutex);
        }
        // record the time at which the task completed the execution of a job
        logEvent(taskID, JOB_COMPLETION, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
    }
    vTaskDelete(NULL);
}

void initLogger() {
    // Create the mutex only once during initialization
    logMutex = xSemaphoreCreateMutex();    
    if (logMutex == NULL) {
        printf("Failed to create log mutex!\n");
        return;
    }
    logSemaphore = xSemaphoreCreateBinary();
    if (logSemaphore == NULL) {
        printf("Failed to create log semaphore!\n");
        return;
    }
    if (xSemaphoreTake(logMutex, portMAX_DELAY) == pdTRUE) {
        resetLogger();
        xSemaphoreGive(logMutex);
    }
    xTaskCreate(vLoggingTask, "Logging Task", 8192, NULL, configMAX_PRIORITIES - 1, NULL);
    printf("Logging task created\n");
}