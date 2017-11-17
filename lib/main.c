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
#include "ecgInterface.h"
#include "blinkyInterface.h"
#include "bleInterface.h"
#include "sdInterface.h"
#include "notification.h"
#include "pendingMessages.h"


struct ecgObject_t * ecgObject_ptr;

TaskHandle_t  taskSendBleHandle;
static void taskSendBle(void * pvParameter);

BLE_HRS_DEF(m_hrs);                                                 /**< Heart rate service instance. */

static TaskHandle_t m_logger_thread;                                /**< Definition of Logger thread. */

#if NRF_LOG_ENABLED
/**@brief Thread for handling the logger.
 *
 * @details This thread is responsible for processing log entries if logs are deferred.
 *          Thread flushes all log entries and suspends. It is resumed by idle task hook.
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

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    #if NRF_LOG_ENABLED
    // Start execution.
    if (pdPASS != xTaskCreate(logger_thread, "LOGGER", 256, NULL, 1, &m_logger_thread))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
    #endif

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void checkReturn(BaseType_t retVal)
{
    if (retVal == pdPASS)
    {
        NRF_LOG_INFO("Checkpoint: created task");
    }
    else if (retVal == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        NRF_LOG_INFO("NEED MORE HEAP !!!!!!!!!!!!!!!!!!!!!!!!!");
    }
    else
    {
        NRF_LOG_INFO("DID NOT PASS XXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }
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
    log_init();
    NRF_LOG_INFO("********** STARTING MAIN *****************");
    NRF_LOG_FLUSH();

    bool erase_bonds;

    clock_init();
    bleInit(&m_hrs);
    buttons_leds_init(&erase_bonds);
    initNotification();
    pendingMessagesCreate(&globalQ);

    // Create a FreeRTOS task for the BLE stack.
    // The task will run advertising_start() before entering its loop.
    nrf_sdh_freertos_init(bleBegin, &erase_bonds);

    checkReturn(xTaskCreate(taskSendBle, "taskSendBle", configMINIMAL_STACK_SIZE+200, NULL, 3, &taskSendBleHandle));

    UNUSED_VARIABLE(ecgInit(ecgObject_ptr));
    //UNUSED_VARIABLE(blinkyInit());

    // Start FreeRTOS scheduler.
    NRF_LOG_INFO("Checkpoint: right before scheduler starts");
    NRF_LOG_FLUSH();

    // Activate deep sleep mode.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

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
    UNUSED_PARAMETER(pvParameter);

    char reqData[WAIT_MESSAGE_SIZE];

    nrf_gpio_cfg_output(27);
    nrf_gpio_pin_clear(27);

    while (true)
    {
        // Wait for Signal
        pendingMessagesWaitAndPop(reqData, &globalQ);

        nrf_gpio_pin_write(27, 1);

        debugErrorMessage(sendData(&m_hrs, (uint8_t*)reqData, sizeof(reqData)));
        
        nrf_gpio_pin_write(27, 0);
    }
}
