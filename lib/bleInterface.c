#include "bleInterface.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "ble_rec.h"

#define OPCODE_LENGTH 1                                                              /**< Length of opcode inside Heart Rate Measurement packet. */
#define HANDLE_LENGTH 2 
#define MAX_HRM_LEN      (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH) /**< Maximum size of a transmitted Heart Rate Measurement. */

void bleInit(ble_hrs_t* m_hrs, ble_rec_t* m_rec) {
    ble_stack_init();
    gap_params_init();
    gatt_init();
    advertising_init();
    services_init(m_hrs, m_rec);
    conn_params_init(m_hrs);
    peer_manager_init();    
}

void bleBegin(void * p_erase_bonds) {
    advertising_start(p_erase_bonds);
}

int sendData(ble_hrs_t* p_hrs, uint8_t* data, size_t length) {
    uint32_t err_code;

    bsp_board_led_invert(BSP_BOARD_LED_1);
    // Send value if connected and notifying
    if (p_hrs->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               len;
        uint16_t               hvx_len;
        ble_gatts_hvx_params_t hvx_params;

        if (length > MAX_HRM_LEN){
            return NRF_ERROR_BUSY;
        }

        // len     = hrm_encode(p_hrs, heart_rate, encoded_hrm);
        len = length;
        hvx_len = len;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_hrs->hrm_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &hvx_len;
        hvx_params.p_data = data;

        err_code = sd_ble_gatts_hvx(p_hrs->conn_handle, &hvx_params);
        if ((err_code == NRF_SUCCESS) && (hvx_len != len))
        {
            err_code = NRF_ERROR_DATA_SIZE;
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }
    bsp_board_led_invert(BSP_BOARD_LED_1);


    return err_code;
}

void debugErrorMessage(ret_code_t err_code) {
    if ((err_code != NRF_SUCCESS) &&
    (err_code != NRF_ERROR_INVALID_STATE) &&
    (err_code != NRF_ERROR_RESOURCES) &&
    (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
    )
    {
        NRF_LOG_ERROR("ERROR IN SENDING");
        NRF_LOG_INFO("ERROR heart_rate_meas_timeout_handler %d", err_code);
        NRF_LOG_FLUSH();
        APP_ERROR_HANDLER(err_code);
    } else if (err_code == NRF_ERROR_INVALID_STATE) {
        NRF_LOG_INFO("Did not send, currently in invalid state");
    } else if (err_code == NRF_SUCCESS) {
        NRF_LOG_INFO("Send was successful!");
    } else if (err_code == NRF_ERROR_RESOURCES) {
        NRF_LOG_ERROR("Not enough resources to send!");
    } else {
        NRF_LOG_INFO("Unknown state handled: %d", err_code);
    }
}

static void service_init_hrs(ble_hrs_t* m_hrs) {
    ret_code_t     err_code;
    ble_hrs_init_t hrs_init;
    ble_dis_init_t dis_init;
    uint8_t        body_sensor_location;

    // Initialize Heart Rate Service.
    body_sensor_location = BLE_HRS_BODY_SENSOR_LOCATION_FINGER;

    memset(&hrs_init, 0, sizeof(hrs_init));

    hrs_init.evt_handler                 = NULL;
    hrs_init.is_sensor_contact_supported = true;
    hrs_init.p_body_sensor_location      = &body_sensor_location;

    // Here the sec level for the Heart Rate Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_hrm_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_hrm_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_hrm_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_bsl_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_bsl_attr_md.write_perm);

    err_code = ble_hrs_init(m_hrs, &hrs_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}

static void service_init_rec(ble_rec_t* m_rec) {

}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
void services_init(ble_hrs_t* m_hrs, ble_rec_t* m_rec)
{
    service_init_hrs(m_hrs);
    service_init_rec(m_rec);
}