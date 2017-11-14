#include "pendingMessages.h"
#include "notification.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"



int pendingMessagesCreate(pendingMessages_t* this) {
	this->queue = xQueueCreate(QUEUE_SIZE, sizeof(char));
	if (!this->queue) {
		NRF_LOG_ERROR("Not enough memory to create the queue");
		NRF_LOG_FLUSH();
		return -1;
	} else {
		return 1;
	}
}

int pendingMessagesWaitAndPop(char* userBuff, pendingMessages_t* this) {
	int i;
	//Sleep until there is an appreciable amount of data
	while (uxQueueMessagesWaiting(this->queue) < WAIT_MESSAGE_SIZE) {
		waitForNotification(BLUETOOTH_NOTIFICATION);
	}
	
	//get data from queue
	for (i = 0; i < WAIT_MESSAGE_SIZE; ++i) {
		if (xQueueReceive(this->queue, &this->tempBuff[i], (TickType_t) 10) == pdFALSE) {
			NRF_LOG_ERROR("Was Not Able to Pop From Queue!");
			NRF_LOG_FLUSH();
		}
	}

	//copy data back to user
	memcpy(userBuff, this->tempBuff, sizeof(this->tempBuff));

	return WAIT_MESSAGE_SIZE;

}


//1. Create a new Entry struct
//2. Add it to the queue
//3. Check capcity of Queue
//4. Notify Bluetooth Task if Appropriate
int pendingMessagesPush(int dataSize, char* userBuff, pendingMessages_t* this) {
	int i;

	//push to queue
	for (i = 0; i < dataSize; ++i) {
		if (xQueueSendToBack(this->queue, (void *) userBuff + i, (TickType_t) 10 )!= pdPASS) {
			NRF_LOG_ERROR("Was Not Able to Push to Queue!");
			NRF_LOG_FLUSH();
		}
	}
	
	//Check to see if we need to notify the bluetooth task
	if (uxQueueMessagesWaiting(this->queue) >= WAIT_MESSAGE_SIZE) {
		sendNotification(BLUETOOTH_NOTIFICATION);
	}

	return 1;
	

}