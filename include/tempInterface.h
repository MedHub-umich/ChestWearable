// tempInterface.h

#ifndef TEMPINTERFACE_H
#define TEMPINTERFACE_H

#include "nrf_drv_saadc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "nrf_timer.h"
#include "arm_const_structs.h"

// Public
int tempInit();

#endif //TEMPINTERFACE_H