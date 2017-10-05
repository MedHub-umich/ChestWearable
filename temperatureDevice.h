/* triggers a read of temperature
REQUIRES:
  - Pointer to this temperature device struct
MODIFIES:
  - TemperatureDeviceStruct * this
EFFECTS:
  - Returns true on error
*/
bool temperatureDeviceSample(TemperatureDeviceStruct * this);

/* returns (by pointer) the temperature
REQUIRES:
  - Pointer to this temperature device struct
MODIFIES:
  - TemperatureDeviceStruct * this
  - int * temperature
EFFECTS:
  - Returns true on error
*/
bool respirationDeviceRead(RespirationDeviceStruct * this, int * temperature);

/* initializes temperature hardware and software
REQUIRES:
  - Pointer to this temperature device struct
MODIFIES:
  - TemperatureDeviceStruct * this
EFFECTS:
  - Returns true on error (I.E. invalid pins)
*/
bool temperatureDeviceInit(TemperatureDeviceStruct * this);

/* turns off temperature monitoring hardware
REQUIRES:
  - Pointer to this temperature device struct
MODIFIES:
  - TemperatureDeviceStruct * this
  - Power gates (Hardware)
EFFECTS:
  - Returns true on error (I.E. unable to sleep?)
*/
bool temperatureDeviceSleep(TemperatureDeviceStruct * this);

/* Turns on respiration monitoring hardware,
 * resets signal processing algorithm
REQUIRES:
  - Pointer to this temperature device struct
MODIFIES:
  - TemperatureDeviceStruct * this
  - Power gates (Hardware)
EFFECTS:
  - Returns true on error (I.E. unable to wake?)
*/
bool temperatureDeviceActive(TemperatureDeviceStruct * this);

typedef struct TemperatureDeviceStruct
{
    // Pins, etc...
} TemperatureDeviceStruct;
