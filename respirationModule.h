#include "messageService.h"
#include "respirationDevice.h"

// ************** Summary ************** //
// The RespirationModule object owns everything that has 
// to do with respiration monitoring. 

// ************** Init ************** //
// This function sets up the respiration monitoring
// functionality. This includes creating tasks to read
// sensors, send messages to the BLE module, perform
// signal processing, and set up sensor hardware. 
// This "class" calls on the respirationDevice "class"
// to interact with respiration hardware.
void respirationInit(RespirationMonitorStruct * this/*pins, initial task rate?, etc*/);

RespirationDeviceStruct * respirationDevicePtr;

// ************** Tasks ************** //

// task that reads from sensor
// periodic
void respirationReadTask(void *);

// task that accumulates and sends data to BLE
// periodic
void respirationSendTask(void *);

// task that does on-device alerts
// periodic? event driven?
void respirationIncomingMessageTask(void *);

// task that acts on commands from medHub
// event driven
void respirationOutgoingMessageTask(void *);

struct RespirationMonitorStruct
{
    RespirationDeviceStruct * RespirationDevicePtr;

}
