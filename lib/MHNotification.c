#include "MHNotification.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "portmacro_cmsis.h"



void MHWaitForNotification(int notificationHandle) {
	xSemaphoreTake( semphMap[notificationHandle], portMAX_DELAY );
}

void MHInitNotification() {
	int i;
	for (i = 0; i < NUM_NOTIFICATIONS; ++i) {
		vSemaphoreCreateBinary(semphMap[i]);
	}
}


void MHSendNotification(int notificationHandle) {
	xSemaphoreGive(semphMap[notificationHandle]);
}

void MHSendNotificationFromISR(int notificationHandle, BaseType_t* xHigherPriortyTaskWoken) {
	xSemaphoreGiveFromISR(semphMap[notificationHandle], xHigherPriortyTaskWoken);
}

