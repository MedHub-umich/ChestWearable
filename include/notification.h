#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "portmacro_cmsis.h"


#define NUM_NOTIFICATIONS 2  //Change whenever you add a new semaphore

#define TEMPERATURE_NOTIFICATION 0
#define BLUETOOTH_NOTIFICATION 1


SemaphoreHandle_t semphMap[NUM_NOTIFICATIONS]; 


void waitForNotification(int notificationHandle);
void initNotification();


void sendNotification(int notificationHandle);

void sendNotificationFromISR(int notificationHandle, BaseType_t* xHigherPriortyTaskWoken);


#endif //NOTIFICATION_H