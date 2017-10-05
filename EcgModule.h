#include "messageService.h"
#include "EcgDevice.h"

// *************** Summary ***************//
// The EcgModule object owns everthing that has 
// to do with ecg monitoring

// This function initalizes all the neccessary data 
// and hardware to monitor ECG
bool ecgModuleInit(EcgModuleStruct * this);

// **************** Tasks ******************//

// This task reads from the sensor
void EcgModuleReadTask(void *);

// This task preforms the DSP needed for a monitor waveform
void EcgModuleDSPTask(void *);

// This task receives messages from other modules
void EcgModuleIncomigMessageTask(void *);

// This task sends messages to other modules
void EcgModuleOutgoingMessageTask(void *);

typdef struct EcgModuleStruct{
	EcgDeviceStruct * EcgDevicePtr;

	//True if the signal is currently being filtered digitally
	bool digitalFilter;
	int buff[];  //TODO: serious question: buf or buff?
	int currValue;
	int sampleRate
} EcgModuleStruct;
