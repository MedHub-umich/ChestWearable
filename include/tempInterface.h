// tempInterface.h

#ifndef TEMPINTERFACE_H
#define TEMPINTERFACE_H

#include "nrf_drv_saadc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "nrf_timer.h"
#include "arm_const_structs.h"



struct tempObject_t {};

// Public
int tempInit(struct tempObject_t*);
float32_t * tempGetDataBuffer();

// Private
void saadc_init(void);
void saadc_sampling_event_init(void);
void saadc_sampling_event_enable(void);
void saadc_callback(nrf_drv_saadc_evt_t const * p_event);
void timer_handler(nrf_timer_event_t event_type, void * p_context);

#endif //TEMPINTERFACE_H