#include "utils/delay.h"

#define VECTORSIZE 16
unsigned int v[VECTORSIZE] = {1};
// function that keeps the task busy for `time_in_ms`
void busyDelay(int time_in_ms) {
    for (int i = 0; i < time_in_ms; i++) {
        int j = 0;
        int k = 0;
        // calibrated to take approximately 1 ms
        while (j < 5944) {
            k = v[j % VECTORSIZE] * v[(k + j) % VECTORSIZE];
            v[j % VECTORSIZE] = k;
            j++;
        }
    }
}