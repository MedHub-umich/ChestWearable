#include "messageService.h"
#include "TemperatureDevice.h"

// ************** Summary ************** //
// The TemperatureModule object owns everything that has 
// to do with Temperature monitoring. 

// ************** Init ************** //
// This function sets up the Temperature monitoring
// functionality. This includes creating tasks to read
// sensors, send messages to the BLE module, perform
// signal processing, and set up sensor hardware. 
// This "class" calls on the TemperatureDevice "class"
// to interact with Temperature hardware.
void TemperatureModuleInit(TemperatureMonitorStruct * this/*pins, initial task rate?, etc*/);

// ************** Tasks ************** //

// task that reads from sensor
void TemperatureModuleReadTask(void *);

// task that recieves messages from other modules
void TemperatureModuleIncomingMessageTask(void *);

// task sends messages to other modules (e.g send sensor readings to communication/BLE module)
void TemperatureModuleOutgoingMessageTask(void *);

struct TemperatureModuleStruct
{
    TemperatureDeviceStruct * temperatureDevicePtr;

}
