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

typedef struct
{
    uint8_t const * p_data;           /**< A pointer to the buffer with received data. */
    uint16_t        length;           /**< Length of received data. */
} ble_rec_evt_rx_data_t;

// Event structure for rec
typedef struct {
    ble_rec_evt_type_t evt_type;
    union{
        ble_rec_evt_rx_data_t rx_data;
    } params;
} ble_rec_evt_t;

// Forward delcaration for the rec type
typedef struct ble_rec_s ble_rec_t;

typedef void (*ble_rec_data_handler_t) (ble_rec_t * p_hrs);

typedef struct {
    ble_rec_data_handler_t data_handler;
} ble_rec_init_t;

// Structure for the receive service
struct ble_rec_s {
    ble_rec_data_handler_t evt_handler;
    uint16_t service_handle;
    uint16_t conn_handle;

};

uint32_t ble_rec_init(ble_rec_t* p_hrs);