
/* triggers a read of the state of charge (SoC)
REQUIRES:
    -pointer to battery struct
MODIFIES
    - BatteryStruct *this
EFFECTS:
    -returns true on error
*/
bool batterySoCSample(BatteryStruct * this);

/*returns the battery SoC by pointer
REQUIRES:
    -pointer to battery struct
MODIFIES:
    - BatteryStruct *this
EFFECTS:
    -returns true on error
*/
bool batterySoCRead();


typdef struct BatteryStruct{
  //pins for ADC converter, etc
} BatteryStruct;