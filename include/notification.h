#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "portmacro_cmsis.h"


#define NUM_NOTIFICATIONS 5  //Change whenever you add a new semaphore

#define BLUETOOTH_NOTIFICATION                  0
#define ECG_BUFFER_FULL_NOTIFICATION            1
#define TEMPERATURE_BUFFER_FULL_NOTIFICATION    2
#define LED_ALERT_NOTIFICATION                  3
#define SPEAKER_ALERT_NOTIFICATION              4


SemaphoreHandle_t semphMap[NUM_NOTIFICATIONS]; 


void waitForNotification(int notificationHandle);
void initNotification();


void sendNotification(int notificationHandle);

void sendNotificationFromISR(int notificationHandle, BaseType_t* xHigherPriortyTaskWoken);


#endif //NOTIFICATION_H