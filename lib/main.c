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
#include "sdInterface.h"
#include "notification.h"
#include "pendingMessages.h"
#include "ble_rec.h"

// SAADC ********************************************************
// FreeRTOS

TaskHandle_t  taskSendBleHandle;
static void taskSendBle(void * pvParameter);

BLE_HRS_DEF(m_hrs);                                                 /**< Heart rate service instance. */
BLE_REC_DEF(m_rec);

TaskHandle_t testTaskHandle;
static void testTask(void* pvParameter);

TaskHandle_t testTask2Handle;
static void testTask2(void* pvParameter);

pendingMessages_t globalQ;
struct tempObject_t * tempObject_ptr;

static void checkTaskCreate(BaseType_t retVal);

// END SAADC ****************************************************

#define HEART_RATE_MEAS_INTERVAL            1000                                    /**< Heart rate measurement interval (ms). */
#define MIN_HEART_RATE                      140                                     /**< Minimum heart rate as returned by the simulated measurement function. */
#define MAX_HEART_RATE                      300                                     /**< Maximum heart rate as returned by the simulated measurement function. */
#define HEART_RATE_INCREMENT                10                                      /**< Value by which the heart rate is incremented/decremented for each call to the simulated measurement function. */

#define OSTIMER_WAIT_FOR_QUEUE              2                                       /**< Number of ticks to wait for the timer queue to be ready */

static TaskHandle_t m_logger_thread;                                /**< Definition of Logger thread. */

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

static void handle_rec0(rec_data_t* rec_data) {
    NRF_LOG_INFO("I am here!");
    if (rec_data->data_length != 2) {
        return;
    }
    // NRF_LOG_INFO("%d", rec_data->data);
    if (rec_data->data[0]) {
        bsp_board_led_on(0);
    } else {
        bsp_board_led_off(0);
    }
    if (rec_data->data[1]) {
        bsp_board_led_on(1);
    } else {
        bsp_board_led_off(1);
    }
}

static void handle_rec1(rec_data_t* rec_data) {
    NRF_LOG_INFO("I am here!");
    UNUSED_PARAMETER(rec_data);
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
    bleInit(&m_hrs, &m_rec);
    buttons_leds_init(&erase_bonds);
    initNotification();
    pendingMessagesCreate(&globalQ);

    registerDataHook(0, handle_rec0);
    registerDataHook(1, handle_rec1);

    // Create a FreeRTOS task for the BLE stack.
    // The task will run advertising_start() before entering its loop.
    nrf_sdh_freertos_init(bleBegin, &erase_bonds);

    BaseType_t retVal = xTaskCreate(taskSendBle, "LED0", configMINIMAL_STACK_SIZE+200, NULL, 3, &taskSendBleHandle);
    checkTaskCreate(retVal);

    retVal = xTaskCreate(testTask, "TestTask", configMINIMAL_STACK_SIZE+200, NULL, 3, &testTaskHandle);
    checkTaskCreate(retVal);

    retVal = xTaskCreate(testTask2, "TestTask2", configMINIMAL_STACK_SIZE+200, NULL, 3, &testTask2Handle);
    checkTaskCreate(retVal);


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

static void checkTaskCreate(BaseType_t retVal) {
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
    char reqData[WAIT_MESSAGE_SIZE];

    nrf_gpio_cfg_output(27);
    nrf_gpio_pin_clear(27);
    ret_code_t err_code;

    while (true)
    {
        // Wait for Signal
        pendingMessagesWaitAndPop(reqData, &globalQ);

        nrf_gpio_pin_write(27, 1);

        // call this function to SEND DATA OVER BLE
        err_code = sendData(&m_hrs, (uint8_t*)reqData, sizeof(reqData));
        // debugErrorMessage(err_code);
        UNUSED_VARIABLE(err_code);
        
        nrf_gpio_pin_write(27, 0);

        //vTaskDelay(1000);
    }
}


static void testTask(void* pvParameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    char userData[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    // char userData[1] = {0xAA};
    for (;;) {
        pendingMessagesPush(sizeof(userData), userData, &globalQ);
        // bsp_board_led_invert(BSP_BOARD_LED_1);
        vTaskDelayUntil(&xLastWakeTime, 5);
    }
}

static void testTask2(void* pvParameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // char userData[4] = {0x11, 0x22, 0x33, 0x44};
    for (;;) {
        // pendingMessagesPush(sizeof(userData), userData, &globalQ);
        // bsp_board_led_invert(BSP_BOARD_LED_1);
        vTaskDelayUntil(&xLastWakeTime, 1000);
    }
}
