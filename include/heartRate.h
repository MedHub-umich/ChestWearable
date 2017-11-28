// heartRate.h

#ifndef HEARTRATE_H_
#define HEARTRATE_H_

#include "app_util_platform.h"
#include "arm_const_structs.h"
#include "respirationRate.h"

static respirationRate_t respiration;

void heartRateInit();
void heartRateExtract(float32_t * inHeartRateBuffer, int inSize);


#endif // HEARTRATE_H_