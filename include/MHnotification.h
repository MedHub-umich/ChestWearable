#ifndef MHNOTIFICATION_H
#define MHNOTIFICATION_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "portmacro_cmsis.h"


#define NUM_NOTIFICATIONS 1  //Change whenever you add a new semaphore

#define TEMPERATURE_NOTIFICATION 0


SemaphoreHandle_t semphMap[NUM_NOTIFICATIONS]; 


void MHWaitForNotification(int notificationHandle);
void MHInitNotification();


void MHSendNotification(int notificationHandle);

void MHSendNotificationFromISR(int notificationHandle, BaseType_t* xHigherPriortyTaskWoken);


#endif //NOTIFICATION_H