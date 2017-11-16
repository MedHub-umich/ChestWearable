// Implementation of our receive service Nordic

#include "ble_rec.h"
#include "sdk_common.h"
#include "ble.h"
#include "ble_srv_common.h"


#define BLE_UUID_NUS_RX_CHARACTERISTIC 0x0002                      /**< The UUID of the RX Characteristic. */

#define BLE_NUS_MAX_RX_CHAR_LEN        BLE_NUS_MAX_DATA_LEN        /**< Maximum length of the RX Characteristic (in bytes). */

#define NUS_BASE_UUID                  {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E}} /**< Used vendor specific UUID. */

// Function to handle the BLE_GAP_EVT_CONNECTTED event
static void on_connect(ble_rec_t *p_rec, ble_evt_t const * p_ble_evt){
    p_rec->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

// Function to handle the BLE_GAP_EVT_DISCONNECTED EVENT
static void on_disconnect(ble_rec_t* p_rec, ble_evt_t const * p_ble_evt) {
    UNUSED_PARAMETER(p_ble_evt);
    p_rec->conn_handle = BLE_CONN_HANDLE_INVALID;
}


// Function to handle write events (when our device is written to (I think))
static void on_write(ble_rec_t* p_rec, ble_evt_t const * p_ble_evt) {
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    ble_rec_evt_t evt;
    evt.p_rec = p_rec;
}