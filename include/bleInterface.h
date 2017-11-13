// The interactions we wish to have the BLE
#ifndef BLE_INTERFACE_H
#define BLE_INTERFACE_H
#include "sdInterface.h"

void bleInit(ble_hrs_t* m_hrs);
void bleBegin(void * p_erase_bonds);
int sendData(ble_hrs_t* m_hrs, uint8_t* data, size_t length);


// Private
void services_init(ble_hrs_t* m_hrs);

#endif //BLE_INTERFACE_H