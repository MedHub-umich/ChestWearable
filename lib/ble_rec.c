// Implementation of our receive service Nordic

#include "ble_rec.h"
#include "sdk_common.h"
#include "ble.h"
#include "ble_srv_common.h"


#define BLE_UUID_REC_RX_CHARACTERISTIC 0x0002                      /**< The UUID of the RX Characteristic. */

#define BLE_REC_MAX_RX_CHAR_LEN        BLE_NUS_MAX_DATA_LEN        /**< Maximum length of the RX Characteristic (in bytes). */

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

    if ( (p_evt_write->handle == p_rec->rx_handles.cccd_handle)
            && (p_evt_write->len == 2)) {
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;
        evt.evt_type = BLE_REC_EVT_RX_DATA;
        p_rec->data_handler(&evt);    
    }
}

// Function to initialize our rx characterisitc
static uint32_t rx_char_add(ble_rec_t* p_rec, const ble_rec_init_t * p_rec_init) {
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t attr_char_value;
    ble_uuid_t ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Define the character attributes for the rx
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write            = 1;
    char_md.char_props.write_wo_resp    = 1;
    // TODO Figure out the below
    char_md.p_char_user_desc            = NULL; // TODO: Change this to be more descriptive
    char_md.p_char_pf                   = NULL; 
    char_md.p_cccd_md                   = NULL;
    char_md.p_sccd_md                   = NULL;

    ble_uuid.type = p_rec->uuid_type;
    ble_uuid.uuid = BLE_UUID_REC_RX_CHARACTERISTIC;

    // Set the attribute metadata qualities
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen= 1;

    // Set the attributecharacter values
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid = &ble_uuid; // Pointer to the attribute uuid
    attr_char_value.p_attr_md = &attr_md; // Pointer to the attribute metatdata
    attr_char_value.init_len = 1; // Initial lenght of attribute
    attr_char_value.init_offs = 0; // Offset of where the attribute stats (ignore)
    attr_char_value.max_len = BLE_REC_MAX_RX_CHAR_LEN; // Maximum size the attribute can be

    // Register the attribute with the device
    return sd_ble_gatts_characteristic_add(p_rec->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_rec->rx_handles);
}

// Handler for bluetooth events
void ble_rec_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context) {
    if ((p_context == NULL) || (p_ble_evt == NULL)) {
        return;
    }

    ble_rec_t * p_rec = (ble_rec_t *)p_context;

    switch(p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_rec, p_ble_evt);
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_rec, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            on_write(p_rec, p_ble_evt);
            break;
        default:
            break;
    }
}