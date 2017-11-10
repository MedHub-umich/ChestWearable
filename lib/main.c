// Main.c

// C stdlib
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "sdk_errors.h"

#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_bas.h"
#include "ble_hrs.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "sensorsim.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_freertos.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "fds.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_common.h"
#include "nrf_delay.h"
#include "boards.h"
#include "bsp.h"
#include "nrf_ble_gatt.h"
#include "nrf_gpio.h"

#include "portmacro_cmsis.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_drv_saadc.h"
#include "nrf_timer.h"

// Interfaces
#include "tempInterface.h"
#include "blinkyInterface.h"
#include "bleInterface.h"

// SAADC ********************************************************
// FreeRTOS

TaskHandle_t  taskSendBleHandle;
static void taskSendBle(void * pvParameter);


struct tempObject_t * tempObject_ptr;

// END SAADC ****************************************************

#define HEART_RATE_MEAS_INTERVAL            1000                                    /**< Heart rate measurement interval (ms). */
#define MIN_HEART_RATE                      140                                     /**< Minimum heart rate as returned by the simulated measurement function. */
#define MAX_HEART_RATE                      300                                     /**< Maximum heart rate as returned by the simulated measurement function. */
#define HEART_RATE_INCREMENT                10                                      /**< Value by which the heart rate is incremented/decremented for each call to the simulated measurement function. */

#define SENSOR_CONTACT_DETECTED_INTERVAL    5000                                    /**< Sensor Contact Detected toggle interval (ms). */

#define OSTIMER_WAIT_FOR_QUEUE              2                                       /**< Number of ticks to wait for the timer queue to be ready */



//static TimerHandle_t m_heart_rate_timer;                            /**< Definition of heart rate timer. */
static TimerHandle_t m_sensor_contact_timer;                        /**< Definition of sensor contact detected timer. */

static TaskHandle_t m_logger_thread;                                /**< Definition of Logger thread. */

/**@brief Function for handling the Heart rate measurement timer time-out.
 *
 * @details This function will be called IN THE FREERTOS THREAD 
 *
 * @param[in] xTimer Handler to the timer that called this function.
 *                   You may get identifier given to the function xTimerCreate using pvTimerGetTimerID.
 */
static void heart_rate_meas_timeout_handler(/*TimerHandle_t xTimer*/)
{
    NRF_LOG_INFO("HEART");
    static uint32_t cnt = 0;
    ret_code_t      err_code;
    uint16_t        heart_rate;

    NRF_LOG_INFO("Current connection type is: %d", m_hrs.conn_handle);

    heart_rate = tempGetDataBuffer()[0]; // DATA BUFFER <<<<<<<<<<<<<<<<<<<<
    cnt++;
    err_code = ble_hrs_heart_rate_measurement_send(&m_hrs, heart_rate);
    if ((err_code != NRF_SUCCESS) &&
    (err_code != NRF_ERROR_INVALID_STATE) &&
    (err_code != NRF_ERROR_RESOURCES) &&
    (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
    )
    {
        NRF_LOG_ERROR("ERROR IN SENDING");
        NRF_LOG_INFO("ERROR heart_rate_meas_timeout_handler");
        NRF_LOG_FLUSH();
        APP_ERROR_HANDLER(err_code);
    } else if (err_code == NRF_ERROR_INVALID_STATE) {
        NRF_LOG_INFO("Did not send, currently in invalid state");
    } else if (err_code == NRF_SUCCESS) {
        NRF_LOG_INFO("Send was successful!");
    } else {
        NRF_LOG_INFO("Unknown state handled: %d", err_code);
    }
}


/**@brief Function for handling the Sensor Contact Detected timer time-out.
 *
 * @details This function will be called each time the Sensor Contact Detected timer expires.
 *
 * @param[in] xTimer Handler to the timer that called this function.
 *                   You may get identifier given to the function xTimerCreate using pvTimerGetTimerID.
 */
// static void sensor_contact_detected_timeout_handler(TimerHandle_t xTimer)
// {
//     static bool sensor_contact_detected = false;

//     UNUSED_PARAMETER(xTimer);

//     sensor_contact_detected = !sensor_contact_detected;
//     ble_hrs_sensor_contact_detected_update(&m_hrs, sensor_contact_detected);
// }


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // m_heart_rate_timer = xTimerCreate("HRT",
    //                                   HEART_RATE_MEAS_INTERVAL,
    //                                   pdTRUE,
    //                                   NULL,
    //                                   heart_rate_meas_timeout_handler);

    // m_sensor_contact_timer = xTimerCreate("SCT",
    //                                       SENSOR_CONTACT_DETECTED_INTERVAL,
    //                                       pdTRUE,
    //                                       NULL,
    //                                       sensor_contact_detected_timeout_handler);

    /* Error checking */
    if ( /*(NULL == m_battery_timer)
         || *//*(NULL == m_heart_rate_timer)*/
         /*|| (NULL == m_rr_interval_timer)*/
         /*||*/ (NULL == m_sensor_contact_timer) )
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
static void services_init(void)
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

    err_code = ble_hrs_init(&m_hrs, &hrs_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief   Function for starting application timers.
 * @details Timers are run after the scheduler has started.
 */
static void application_timers_start(void)
{
    // if (pdPASS != xTimerStart(m_heart_rate_timer, OSTIMER_WAIT_FOR_QUEUE))
    // {
    //     APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    // }
    // if (pdPASS != xTimerStart(m_sensor_contact_timer, OSTIMER_WAIT_FOR_QUEUE))
    // {
    //     APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    // }
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


#if NRF_LOG_ENABLED
/**@brief Thread for handling the logger.
 *
 * @details This thread is responsible for processing log entries if logs are deferred.
 *          Thread flushes all log entries and suspends. It is resumed by idle task hook.
 *
 * @param[in]   arg   Pointer used for passing some arbitrary information (context) from the
 *                    osThreadCreate() call to the thread.
 */
static void logger_thread(void * arg)
{
    UNUSED_PARAMETER(arg);

    while (1)
    {
        NRF_LOG_FLUSH();

        vTaskSuspend(NULL); // Suspend myself
    }
}
#endif //NRF_LOG_ENABLED

/**@brief A function which is hooked to idle task.
 * @note Idle hook must be enabled in FreeRTOS configuration (configUSE_IDLE_HOOK).
 */
void vApplicationIdleHook( void )
{
     vTaskResume(m_logger_thread);
}


/**@brief Function for initializing the clock.
 */
static void clock_init(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for application main entry.
 */
int main(void)
{

    bool erase_bonds;

    clock_init();

    // Do not start any interrupt that uses system functions before system initialisation.
    // The best solution is to start the OS before any other initalisation.

    log_init();

#if NRF_LOG_ENABLED
    // Start execution.
    if (pdPASS != xTaskCreate(logger_thread, "LOGGER", 256, NULL, 1, &m_logger_thread))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
#endif

    NRF_LOG_INFO("********** STARTING MAIN *****************");
    NRF_LOG_FLUSH();

    // Activate deep sleep mode.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    // Configure and initialize the BLE stack.
    ble_stack_init();

    timers_init();
    buttons_leds_init(&erase_bonds);
    gap_params_init();
    gatt_init();
    advertising_init();
    services_init();
    conn_params_init();
    peer_manager_init();
    application_timers_start();



    // Create a FreeRTOS task for the BLE stack.
    // The task will run advertising_start() before entering its loop.
    nrf_sdh_freertos_init(advertising_start, &erase_bonds);

    BaseType_t retVal = xTaskCreate(taskSendBle, "LED0", configMINIMAL_STACK_SIZE+200, NULL, 3, &taskSendBleHandle);
    if (retVal == pdPASS)
    {
        NRF_LOG_INFO("Checkpoint: created taskSendBle");
    }
    else if (retVal == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        NRF_LOG_INFO("NEED MORE HEAP !!!!!!!!!!!!!!!!!!!!!!!!!");
    }
    else
    {
        NRF_LOG_INFO("DID NOT PASS XXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }

    UNUSED_VARIABLE(tempInit(tempObject_ptr));
    UNUSED_VARIABLE(blinkyInit());

    // Start FreeRTOS scheduler.
    NRF_LOG_INFO("Checkpoint: right before scheduler starts");
    NRF_LOG_FLUSH();

    vTaskStartScheduler();

    while (true)
    {
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }
}


/**@taskToggleLed
 *
 * Flushes the log buffer to send messages to segger RTT.
 * Deferred interrupt.
 *
 */
static void taskSendBle (void * pvParameter)
{
    //uint16_t millivolts = 0;
    UNUSED_PARAMETER(pvParameter);

    nrf_gpio_cfg_output(27);
    nrf_gpio_pin_clear(27);

    while (true)
    {
        // Wait for Signal
        xSemaphoreTake( tempGetDataSemaphore(tempObject_ptr), portMAX_DELAY );

        nrf_gpio_pin_write(27, 1);

        NRF_LOG_INFO("Checkpoint: taskSendBle got semaphore");
        NRF_LOG_FLUSH();

        // call this function to SEND DATA OVER BLE
        heart_rate_meas_timeout_handler();

        nrf_gpio_pin_write(27, 0);

        //vTaskDelay(1000);
    }
}
