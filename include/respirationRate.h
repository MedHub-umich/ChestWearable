// respirationRate.h

#ifndef RESPIRATIONRATE_H_
#define RESPIRATIONRATE_H_

#include "app_util_platform.h"
#include "arm_const_structs.h"

#define PEAKS_SIZE 20
#define SEND_PERIOD_MILLI 1000

typedef struct peakPair
{
    float32_t magnitude;
    float32_t time;
} peakPair_t;

typedef struct respirationRate
{
    int numPeaks;
    peakPair_t peaks[PEAKS_SIZE];
} respirationRate_t;

void respirationRateInit(respirationRate_t * this);
void respirationRateAddPair(float32_t inMagnitude, float32_t inTime, respirationRate_t * this);
void respirationRateProcess(respirationRate_t * this);

#endif // respirationRate_H_