#include "bleInterface.h"

void bleInit(ble_hrs_t* m_hrs) {
    ble_stack_init();
    gap_params_init();
    gatt_init();
    advertising_init();
    services_init(m_hrs);
    conn_params_init(m_hrs);
    peer_manager_init();    
}

void bleBegin(void * p_erase_bonds) {
    advertising_start(p_erase_bonds);
}

int sendData(ble_hrs_t* m_hrs, void* data, size_t length) {
    return NRF_ERROR_NULL;
}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
void services_init(ble_hrs_t* m_hrs)
{
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