#define BLE_REC_DEF(_name)                                                                          \
static ble_rec_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_hrs_on_ble_evt, &_name)

// Event types for rec service
typedef enum {
    BLE_REC_EVT_ALERT,
    BLE_REC_EVT_DIAGNOSTIC
} ble_rec_evt_type_t;

// Event structure for rec
typedef enum {
    ble_rec_evt_type_t evt_type;
} ble_rec_evt_t;