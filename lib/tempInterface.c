// tempInterface.c
#include "tempInterface.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_gpio.h"
// SAADC
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "notification.h"

// *************** Internal State ****************************** //
static struct tempObject_t * this = 0;
static struct tempObject_t tempObject;
// ************************************************************* //

// SAADC
#define SAMPLES_IN_BUFFER 200
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS   600                                     /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION    6                                       /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define ADC_RES_10BIT                   1024                                    /**< Maximum digital value for 10-bit ADC conversion. */
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)

static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1);
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;

static nrf_saadc_value_t dummy = 0xBEEF;
nrf_saadc_value_t * dataBuffer = &dummy;

#define TASK_DELAY        400           /**< Task delay. Delays a LED0 task for 200 ms */
TaskHandle_t  taskToggleLedHandle;   /**< Reference to LED0 toggling FreeRTOS task. */

void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event every 100ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 5); // TOM
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}


void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        dataBuffer = p_event->data.done.p_buffer;

        // Signal the data processing task
        if(this == 0)
        {
            NRF_LOG_INFO("DANGER: THIS USED BEFORE INITIALIZED");
        }


        sendNotification(TEMPERATURE_NOTIFICATION);


        NRF_LOG_INFO("Gave semaphore");
    }
}


void saadc_init(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN1);

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
}


void timer_handler(nrf_timer_event_t event_type, void * p_context)
{
    NRF_LOG_INFO("\n\n\n\nTIMER HANDLER RAN (IT SHOULD NOT)\n\n\n\n");
}

/**@taskToggleLed
 *
 * Blinks an LED
 *
 */
void taskToggleLed (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    int gpioState = 0;

    nrf_gpio_cfg_output(7);
    nrf_gpio_pin_clear(7);

    NRF_LOG_INFO("Checkpoint: beginning of taskToggleLed");

    while (true)
    {

        // toggle gpio pin
        gpioState = !gpioState;
        nrf_gpio_pin_write(7, gpioState);

        // Delay (messy period)
        vTaskDelay(TASK_DELAY);
    }
}

nrf_saadc_value_t * tempGetDataBuffer()
{
    return dataBuffer;
}

int tempInit(struct tempObject_t * inTempObject_ptr)
{
    this = &tempObject;

    // initialize ADC if not already initialized
    // Actually currently not checking
    saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();

    // create FreeRtos tasks
    BaseType_t retVal = xTaskCreate(taskToggleLed, "LED0", configMINIMAL_STACK_SIZE + 60, NULL, 2, &taskToggleLedHandle);
    if (retVal == pdPASS)
    {
        NRF_LOG_INFO("TEMP SENSOR GPIO TOGGLE THREAD CREATED");
    }
    else if (retVal == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        NRF_LOG_INFO("TEMP SENSOR GPIO TOGGLE NEED MORE HEAP!!!!!!!!");
    }
    else
    {
        NRF_LOG_INFO("TEMP SENSOR GPIO TOGGLE THREAD DID NOT PASS XXXXXXXXX");
    }

    NRF_LOG_INFO("Checkpoint: end of tempInit");

    inTempObject_ptr = this;
    return 0;
}

