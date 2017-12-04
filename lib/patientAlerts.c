// patientAlerts.c

#include "FreeRTOS.h"
#include "task.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "patientAlerts.h"
#include "notification.h"

TaskHandle_t  taskAlertLEDHandle;
TaskHandle_t  taskAlertSpeakerHandle;

void taskAlertLED (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    while (true)
    {
        waitForNotification(LED_ALERT_NOTIFICATION);

        //LED on
        nrf_gpio_pin_write(LED_ALERT_PIN, 1);

        vTaskDelay(100);

        // LED off
        nrf_gpio_pin_write(LED_ALERT_PIN, 0);
    }
}

void taskAlertSpeaker (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    while (true)
    {
        waitForNotification(SPEAKER_ALERT_NOTIFICATION);

        NRF_LOG_INFO("Alert.");

        vTaskDelay(100);
    }
}

static void checkReturn(BaseType_t retVal)
{
    if (retVal == pdPASS)
    {
        NRF_LOG_INFO("Checkpoint: created an Alert task");
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

int patientAlertsInit(/*PatientAlerts * this*/)
{
    //Initialize LED for alerts
    nrf_gpio_cfg(
        LED_ALERT_PIN,
        GPIO_PIN_CNF_DIR_Output,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        GPIO_PIN_CNF_DRIVE_S0H1,
        NRF_GPIO_PIN_NOSENSE
    );
    nrf_gpio_pin_clear(LED_ALERT_PIN);

    // Initialize PWM

    // Create alert tasks
    checkReturn(xTaskCreate(taskAlertLED, "LED", configMINIMAL_STACK_SIZE + 50, NULL, 2, &taskAlertLEDHandle));
    checkReturn(xTaskCreate(taskAlertSpeaker, "SP", configMINIMAL_STACK_SIZE + 50, NULL, 2, &taskAlertSpeakerHandle));

    return 0;
}

