// The interactions we wish to have the BLE
#ifndef BLE_INTERFACE_H
#define BLE_INTERFACE_H
#include "sdInterface.h"

void bleInit();
void bleBegin(void * p_erase_bonds);


// Private
void services_init(void);

#endif //BLE_INTERFACE_H