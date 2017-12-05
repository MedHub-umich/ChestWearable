// patientAlerts.c

#include "FreeRTOS.h"
#include "task.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "bsp.h"
#include "app_timer.h"
#include "low_power_pwm.h"

#include "patientAlerts.h"
#include "notification.h"


TaskHandle_t  taskAlertLEDHandle;
TaskHandle_t  taskAlertSpeakerHandle;

/*Ticks before change duty cycle of each LED*/
#define TICKS_BEFORE_CHANGE_0   500
#define TICKS_BEFORE_CHANGE_1   400

static low_power_pwm_t low_power_pwm_0;

void taskAlertLED (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    while (true)
    {
        waitForNotification(LED_ALERT_NOTIFICATION);

        //LED on
        nrf_gpio_pin_write(LED_ALERT_PIN, 1);

        vTaskDelay(BLINK_DURATION);

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

        low_power_pwm_start((&low_power_pwm_0), low_power_pwm_0.bit_mask);
        //APP_ERROR_CHECK(err_code);

        NRF_LOG_INFO("Alert.");

        vTaskDelay(BEEP_DURATION);

        low_power_pwm_stop(&low_power_pwm_0);
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




static void pwm_handler(void * p_context)
{
}
/**
 * @brief Function to initalize low_power_pwm instances.
 *
 */

static void pwm_init(void)
{
    uint32_t err_code;
    low_power_pwm_config_t low_power_pwm_config;

    APP_TIMER_DEF(lpp_timer_0);
    low_power_pwm_config.active_high    = false;
    low_power_pwm_config.period         = APP_TIMER_TICKS(4);
    low_power_pwm_config.bit_mask       = SPEAKER_BIT_MASK; 
    low_power_pwm_config.p_timer_id     = &lpp_timer_0;
    low_power_pwm_config.p_port         = NRF_GPIO;

    err_code = low_power_pwm_init((&low_power_pwm_0), &low_power_pwm_config, pwm_handler);
    APP_ERROR_CHECK(err_code);

    err_code = low_power_pwm_duty_set(&low_power_pwm_0, APP_TIMER_TICKS(2));
    APP_ERROR_CHECK(err_code);
}

int patientAlertsInit(/*PatientAlerts * this*/)
{
    uint8_t new_duty_cycle;
    uint32_t err_code;

    // Start APP_TIMER to generate timeouts.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    /*Initialize low power PWM for all 3  channels of RGB or 3 channels of leds on pca10028*/
    pwm_init();

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

