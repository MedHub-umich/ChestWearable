/* collects one sample
REQUIRES:
  - Pointer to the struct of this respiration device instance
  - Properly configured ADC hardware
MODIFIES:
  - RespirationDeviceStruct * this
EFFECTS:
  - Returns true on error
*/
bool respirationDeviceSample(RespirationDeviceStruct * this);

/* processes data if necessary and returns (by pointer) the respiration status
REQUIRES:
  - Pointer to the struct of this respiration device instance
MODIFIES:
  - RespirationDeviceStruct * this
  - bool * status (breathing or not breathing)
EFFECTS:
  - Returns true on error (I.E. not enough readings taken)
*/
bool respirationDeviceRead(RespirationDeviceStruct * this, bool * status);

/* initializes respiration hardware and software
REQUIRES:
  - Pointer to the struct of this respiration device instance
MODIFIES:
  - RespirationDeviceStruct * this
EFFECTS:
  - Returns true on error (I.E. invalid pins)
*/
bool respirationDeviceInit(RespirationDeviceStruct * this, int adcPin, int powerGatePin);

/* turns off respiration monitoring hardware
REQUIRES:
  - Pointer to the struct of this respiration device instance
MODIFIES:
  - RespirationDeviceStruct * this
  - Power gates (Hardware)
  - Pauses signal processing
EFFECTS:
  - Returns true on error (I.E. unable to sleep?)
*/
bool respirationDeviceSleep(RespirationDeviceStruct * this);

/* Turns on respiration monitoring hardware,
 * resets signal processing algorithm
REQUIRES:
  - Pointer to the struct of this respiration device instance
MODIFIES:
  - RespirationDeviceStruct * this
  - Power gates (Hardware)
  - Resets signal processing
EFFECTS:
  - Returns true on error (I.E. unable to sleep?)
*/
bool respirationDeviceActive(RespirationDeviceStruct * this);

typedef struct RespirationDeviceStruct
{
    // pins
    // array of data points
    // current respiration status
    // current mode (active, sleep, etc)
} RespirationDeviceStruct;
