// ecgInterface.h

#ifndef ECGINTERFACE_H
#define ECGINTERFACE_H

#include "nrf_drv_saadc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "nrf_timer.h"
#include "arm_const_structs.h"

struct ecgObject_t {};

// Public
int ecgInit(struct ecgObject_t*);

// Private
void saadc_init(void);
void saadc_sampling_event_init(void);
void saadc_sampling_event_enable(void);
void saadc_callback(nrf_drv_saadc_evt_t const * p_event);
void timer_handler(nrf_timer_event_t event_type, void * p_context);

#endif //ECGINTERFACE_H