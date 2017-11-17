// The interactions we wish to have the BLE
#ifndef BLE_INTERFACE_H
#define BLE_INTERFACE_H
#include "sdInterface.h"
#include "ble_rec.h"

void bleInit(ble_hrs_t* m_hrs, ble_rec_t* m_rec);
void bleBegin(void * p_erase_bonds);
int sendData(ble_hrs_t* m_hrs, uint8_t* data, size_t length);
void debugErrorMessage(ret_code_t err_code);


// Private
void services_init(ble_hrs_t* m_hrs, ble_rec_t* m_rec);

#endif //BLE_INTERFACE_H