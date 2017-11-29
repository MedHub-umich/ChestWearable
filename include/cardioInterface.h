// ecgInterface.h

#ifndef ECGINTERFACE_H
#define ECGINTERFACE_H

#include "nrf_drv_saadc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "nrf_timer.h"
#include "arm_const_structs.h"
#include "packager.h"

#define ECG_DATA_PACKET_SIZE SAMPLES_PER_CHANNEL
// Public
typedef struct ecg {
	Packager ecgPackager;
} ecg;

int cardioInit();



#endif //ECGINTERFACE_H