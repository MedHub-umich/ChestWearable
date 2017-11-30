// heartRate.h

#ifndef HEARTRATE_H_
#define HEARTRATE_H_

#include "app_util_platform.h"
#include "arm_const_structs.h"
#include "respirationRate.h"
#include "packager.h"

#define HEARTRATE_DATA_PACKET_SIZE 1

typedef struct heartRate 
{
	Packager heartRatePackager;
	
} heartRate_t;

heartRate_t heartRateDevice;



void heartRateInit();
void heartRateExtract(float32_t * inHeartRateBuffer, int inSize);


#endif // HEARTRATE_H_