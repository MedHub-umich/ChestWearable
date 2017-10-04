#include "messageService.h"
#include "RespirationDevice.h"

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
void RespirationInit(RespirationMonitorStruct * this/*pins, initial task rate?, etc*/);

// ************** Tasks ************** //

// task that reads from sensor
void RespirationModuleReadTask(void *);

// task that processes signals
void RespirationModuleDSPTask(void *);

// task that recieves messages from other modules
void RespirationModuleIncomingMessageTask(void *);

// task sends messages to other modules (e.g send sensor readings to communication/BLE module)
void RespirationModuleOutgoingMessageTask(void *);

struct RespirationModuleStruct
{
    RespirationDeviceStruct * RespirationDevicePtr;

}
