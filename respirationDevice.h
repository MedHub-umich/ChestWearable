// returns the respiration status of the patient
bool respirationDevicRead(RespirationDeviceStruct * this);

// initializes respiration monitoring hardware
// returns true on failure
bool respirationDeviceInit(RespirationDeviceStruct * this);

// prepares the respiration monitoring hardware
// for sleep mode
// returns true on failure
bool respirationDeviceSleep(RespirationDeviceStruct * this);

// prepares the respiration monitoring hardware
// for active mode
// returns true on failure
bool respirationDeviceActive(RespirationDeviceStruct * this);

typedef struct RespirationDeviceStruct
{
    // pins, etc
} RespirationDeviceStruct;
