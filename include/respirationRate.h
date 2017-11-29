// respirationRate.h

#ifndef RESPIRATIONRATE_H_
#define RESPIRATIONRATE_H_

#include "app_util_platform.h"
#include "arm_const_structs.h"
#include "packager.h"

#define PEAKS_SIZE 20
#define SEND_PERIOD_MILLI 1000
#define CROSSINGS_PER_BREATH 2
#define SECONDS_PER_MINUTE 60
#define BR_DATA_PACKET_SIZE 1

typedef struct peakPair
{
    float32_t magnitude;
    float32_t time;
} peakPair_t;

typedef struct respirationRate
{
    int numPeaks;
    int averageValue;
    peakPair_t peaks[PEAKS_SIZE];
    Packager breathingRatePackager;
} respirationRate_t;

void respirationRateInit(respirationRate_t * this);
void respirationRateAddPair(float32_t inMagnitude, float32_t inTime, respirationRate_t * this);
void respirationRateProcess(respirationRate_t * this);


// Private
float32_t calcTotalTimeElapsedDuringData(respirationRate_t * this);
float32_t calcAverageValueOfData(respirationRate_t * this);
float32_t calcNumCrossingsInData(respirationRate_t * this);

#endif // respirationRate_H_