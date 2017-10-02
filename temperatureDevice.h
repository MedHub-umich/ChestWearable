// returns the temperature of the patient
int temperatureDeviceRead(TemperatureDeviceStruct * this);

// initializes temperature monitoring hardware
// returns true on failure
bool temperatureDeviceInit(TemperatureDeviceStruct * this);

// prepares the temperature monitoring hardware
// for sleep mode
// returns true on failure
bool temperatureDeviceSleep(TemperatureDeviceStruct * this);

// prepares the temperature monitoring hardware
// for active mode
// returns true on failure
bool temperatureDeviceActive(TemperatureDeviceStruct * this);

typedef struct TemperatureDeviceStruct
{
    // Pins, etc...
} TemperatureDeviceStruct;
