/* Initializes the EcgDevice 
PARAMETERS:
	- Pointer to the struct of this ecg device instance
	- Properly configured ADC hardware

This function will return true on error
*/
bool ecgDeviceInit(ecgDeviceStruct * this);

/* Triggers one sample from ECG
PARAMETERS:
	- Pointer to the struct of this ecg device instance

This function triggers collecting data from the ADC connected 
to the ECG circuit. Returns true on error.
*/
bool ecgDeviceSample(ecgDeviceStruct * this);

/* Sets the device to sleep by power gating the circuit
PARAMETERS:
	- Pointer to the struct of this ecg device instance
Returns true on error.
*/
bool ecgDeviceSleep(EcgDeviceStruct * this);

/* Sets the sample rate for reading the data
PARAMETERS:
	- Pointer to the struct of this ecg device instance
*/
void ecgDeviceSampleRate(EcgDeviceStruct * this);

/* Outputs the current value of the sensor based on the filtering
PARAMETERS:
	- Pointer to the struct of this ecg device instance
*/
int ecgDeviceFilteredValue(ecgDeviceStruct * this);

/* Calls ecgFilteredVlaue() to filter the incoming signal digitally
Returns true on error
PARAMETERS:
	- Pointer to the struct of this ecg device instance
	- bool to turn on or off filtering
Returns true on error.
*/
bool ecgDeviceDigitalFilter(ecgDeviceStruct * this, bool set);

typedef struct EcgDeviceStruct{
	// pins

	// array of data points

	// current heartrate

	// current filter status

	// current mode (active, sleep, etc)
}EcgDeviceStruct;

