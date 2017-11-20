// ecgInterface.h

#ifndef ECGINTERFACE_H
#define ECGINTERFACE_H

#include "nrf_drv_saadc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "nrf_timer.h"
#include "arm_const_structs.h"

// Public
int ecgInit();

#endif //ECGINTERFACE_H