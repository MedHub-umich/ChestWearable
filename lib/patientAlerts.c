// patientAlerts.c

#include "FreeRTOS.h"
#include "task.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "bsp.h"
#include "nrf_drv_pwm.h"

#include "patientAlerts.h"
#include "notification.h"


TaskHandle_t  taskAlertLEDHandle;
TaskHandle_t  taskAlertSpeakerHandle;

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

// This is for tracking PWM instances being used, so we can unintialize only
// the relevant ones when switching from one demo to another.
#define USED_PWM(idx) (1UL << idx)
static uint8_t m_used = 0;

static uint16_t const              m_demo1_top  = 10000;
static uint16_t const              m_demo1_step = 200;
static uint8_t                     m_demo1_phase;
static nrf_pwm_values_individual_t m_demo1_seq_values;

static nrf_pwm_sequence_t const    m_demo1_seq =
{
    .values.p_individual = &m_demo1_seq_values,
    .length              = NRF_PWM_VALUES_LENGTH(m_demo1_seq_values),
    .repeats             = 0,
    .end_delay           = 0
};

static void demo1_handler(nrf_drv_pwm_evt_type_t event_type)
{
    if (event_type == NRF_DRV_PWM_EVT_FINISHED)
    {
        uint8_t channel    = m_demo1_phase >> 1;
        bool    down       = m_demo1_phase & 1;
        bool    next_phase = false;

        uint16_t * p_channels = (uint16_t *)&m_demo1_seq_values;
        uint16_t value = p_channels[channel];
        if (down)
        {
            value -= m_demo1_step;
            if (value == 0)
            {
                next_phase = true;
            }
        }
        else
        {
            value += m_demo1_step;
            if (value >= m_demo1_top)
            {
                next_phase = true;
            }
        }
        p_channels[channel] = value;

        if (next_phase)
        {
            if (++m_demo1_phase >= 2 * NRF_PWM_CHANNEL_COUNT)
            {
                m_demo1_phase = 0;
            }
        }
    }
}
static void demo1(void)
{
    NRF_LOG_INFO("Demo 1");

    /*
     * This demo plays back a sequence with different values for individual
     * channels (LED 1 - LED 4). Only four values are used (one per channel).
     * Every time the values are loaded into the compare registers, they are
     * updated in the provided event handler. The values are updated in such
     * a way that increase and decrease of the light intensity can be observed
     * continuously on succeeding channels (one second per channel).
     */

    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            27//BSP_LED_0 | NRF_DRV_PWM_PIN_INVERTED, // SPEAKER_BIT_MASK
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = m_demo1_top,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, demo1_handler));
    m_used |= USED_PWM(0);

    m_demo1_seq_values.channel_0 = 0;
    m_demo1_seq_values.channel_1 = 0;
    m_demo1_seq_values.channel_2 = 0;
    m_demo1_seq_values.channel_3 = 0;
    m_demo1_phase                = 0;

    (void)nrf_drv_pwm_simple_playback(&m_pwm0, &m_demo1_seq, 1,
                                      NRF_DRV_PWM_FLAG_LOOP);
}

static void demo5(void)
{
    NRF_LOG_INFO("Demo 5");

    /*
     * This demo, similarly to demo1, plays back a sequence with different
     * values for individual channels. Unlike demo 1, however, it does not use
     * an event handler. Therefore, the PWM peripheral does not use interrupts
     * and the CPU can stay in sleep mode.
     * The LEDs (1-4) blink separately. They are turned on for 125 ms each,
     * in counterclockwise order (looking at the board).
     */

    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            SPEAKER_ALERT_PIN
            //BSP_LED_0 | NRF_DRV_PWM_PIN_INVERTED, // channel 0
            // BSP_LED_2 | NRF_DRV_PWM_PIN_INVERTED, // channel 1
            // BSP_LED_3 | NRF_DRV_PWM_PIN_INVERTED, // channel 2
            // BSP_LED_1 | NRF_DRV_PWM_PIN_INVERTED  // channel 3
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_125kHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 15625,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, NULL));
    m_used |= USED_PWM(0);

    // This array cannot be allocated on stack (hence "static") and it must
    // be in RAM (hence no "const", though its content is not changed).
    static nrf_pwm_values_individual_t /*const*/ seq_values[] =
    {
        { 0x8000,      0,      0,      0 },
        {      0, 0x8000,      0,      0 },
        {      0,      0, 0x8000,      0 },
        {      0,      0,      0, 0x8000 }
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

        //low_power_pwm_start((&low_power_pwm_0), low_power_pwm_0.bit_mask);
        //APP_ERROR_CHECK(err_code);

        NRF_LOG_INFO("Alert.");

        demo5();

        vTaskDelay(BEEP_DURATION);

        nrf_drv_pwm_stop(&m_pwm0, 0);
        //low_power_pwm_stop(&low_power_pwm_0);
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
    uint32_t err_code;

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

