// patientAlerts.c

#include "FreeRTOS.h"
#include "task.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "notification.h"

#define BLINK_DURATION        400
#define BEEP_DURATION         200
TaskHandle_t  taskLEDHandle;


void taskLED (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    while (true)
    {
        waitForNotification(LED_ALERT_NOTIFICATION);

        // LED on
        (BSP_BOARD_LED_1);

        vTaskDelay(BLINK_DURATION);

        // LED off

    }
}


void patientAlertsInit(void)
{
    // initialize ADC if not already initialized
    

    // create FreeRtos tasks
    BaseType_t retVal = xTaskCreate(taskLED, "LED", configMINIMAL_STACK_SIZE + 60, NULL, 2, &taskLEDHandle);

    if (retVal == pdPASS)
    {
        NRF_LOG_INFO("Checkpoint: created blinky thread");
        NRF_LOG_FLUSH();
    }
    else if (retVal == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        NRF_LOG_INFO("BLINKY THREAD NEED MORE HEAP!!!!!!!!");
        NRF_LOG_FLUSH();
    }
    else
    {
        NRF_LOG_INFO("BLINKY THREAD DID NOT PASS XXXXXXXXX");
        NRF_LOG_FLUSH();
    }
}