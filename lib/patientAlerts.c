// patientAlerts.c

#include "FreeRTOS.h"
#include "task.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "bsp.h"
#include "nrf_drv_pwm.h"
#include "nrf_drv_gpiote.h"

#include "patientAlerts.h"
#include "notification.h"


TaskHandle_t  taskAlertLEDHandle;
TaskHandle_t  taskAlertSpeakerHandle;

static bool buttonHandled = false;

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);


void buttonHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    // if (buttonHandled == false) {
    //     buttonHandled = true;
        BaseType_t * Woken;
        sendNotificationFromISR(LED_ALERT_NOTIFICATION, Woken);
    // }
    // else
    // {
    //     buttonHandled = false;
    // }
}


static void initButton(void)
{
    ret_code_t err_code;

    //int retVal;

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    err_code = nrf_drv_gpiote_in_init(BUTTON_INPUT_PIN, &in_config, buttonHandler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(BUTTON_INPUT_PIN, true);

    NRF_LOG_INFO("Initialized Button");
}


static void initBlink()
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
}

static void initBeep()
{
    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            SPEAKER_ALERT_PIN
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_125kHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = BEEP_PERIOD_TICKS,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, NULL));
}


static void startBeep(void)
{
    static nrf_pwm_values_individual_t /*const*/ seq_values[] =
    {
        {BEEP_DUTY_CYCLE_TICKS}
    };

    nrf_pwm_sequence_t const seq =
    {
        .values.p_individual = seq_values,
        .length              = NRF_PWM_VALUES_LENGTH(seq_values),
        .repeats             = 0,
        .end_delay           = 0
    };

    (void)nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, NRF_DRV_PWM_FLAG_LOOP);
}


static void stopBeep()
{
    nrf_drv_pwm_stop(&m_pwm0, 0);
}


void taskAlertLED (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    while (true)
    {
        waitForNotification(LED_ALERT_NOTIFICATION);

        NRF_LOG_INFO("BLINK");

        // buttonHandled = false;

        //LED on
        //nrf_gpio_pin_write(LED_ALERT_PIN, 1);

        //vTaskDelay(BLINK_DURATION);

        // LED off
        //nrf_gpio_pin_write(LED_ALERT_PIN, 0);
    }
}


void taskAlertSpeaker (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    while (true)
    {
        waitForNotification(SPEAKER_ALERT_NOTIFICATION);

        NRF_LOG_INFO("Alert.");

        startBeep();

        vTaskDelay(BEEP_DURATION);
        
        stopBeep();
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


int patientAlertsInit(PatientAlerts * this)
{
    initBeep();

    initBlink();

    initButton();

    // Create alert tasks
    checkReturn(xTaskCreate(taskAlertLED, "LED", configMINIMAL_STACK_SIZE + 50, NULL, 2, &taskAlertLEDHandle));
    checkReturn(xTaskCreate(taskAlertSpeaker, "SP", configMINIMAL_STACK_SIZE + 50, NULL, 2, &taskAlertSpeakerHandle));

    return 0;
}

