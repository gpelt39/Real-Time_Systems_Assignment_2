#ifndef TRACES_H
#define TRACES_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// ========= Configuration parameters ========

// maximum number of log entries that can be saved before a log dump is required
#define MAX_LOG_SIZE 500
// number of log entries at which a log dump is triggered
#define LOG_WATERMARK 600 // (Originally at 450)
// periodicity with which the log is dumped if the watermark is not reached beforehand
#define LOGGING_PERIOD_MS 30000 // 1 minute

// ===== End of configuration parameters =====

typedef enum {
    JOB_START = 1,
    JOB_COMPLETION = 0
} EventType;

// start the logger
void initLogger();
// log an event of type `event` that happened at time `timestamp` for task `taskNum`
void logEvent(uint32_t taskNum, EventType event, uint32_t timestamp);

#endif // TRACES_H