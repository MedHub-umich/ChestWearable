// ecgInterface.c

#include "ecgInterface.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "boards.h"
#include "bsp.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_gpio.h"
#include "pendingMessages.h"
// SAADC
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "notification.h"
// FILTER
#include "app_util_platform.h"
#include "arm_const_structs.h"

// This
static struct ecgObject_t * this = 0;
static struct ecgObject_t ecgObject;

// SAADC
#define SAMPLE_PERIOD_MILLI             2
#define SAMPLES_IN_BUFFER               34 // MUST BE DIVISIBLE BY DOWNSAMPLE FACTOR
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS   600                                     /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION    6                                       /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define ADC_RES_10BIT                   1024                                    /**< Maximum digital value for 10-bit ADC conversion. */
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)
static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1);
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;
static nrf_saadc_value_t dummy = 0xBEEF;

// Filter
#define NUM_TAPS              27
#define BLOCK_SIZE SAMPLES_IN_BUFFER
TaskHandle_t  taskFIRHandle;
static arm_fir_instance_f32 S;
static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
static const uint32_t blockSize = BLOCK_SIZE;
static float32_t dataBuffer[SAMPLES_IN_BUFFER];
static float32_t dataBufferFiltered[SAMPLES_IN_BUFFER];

static float32_t firCoeffs32[NUM_TAPS] = {
  -0.019008758166749587,
  0.015889163933487414,
  0.03263961578701652,
  0.04683589875623874,
  0.04544033608407664,
  0.02255944841411092,
  -0.014989388364962738,
  -0.04810788606906059,
  -0.0533870784592822,
  -0.01550298589628618,
  0.06255063924866554,
  0.15770946311133774,
  0.23556353376462763,
  0.26552471337330985,
  0.23556353376462763,
  0.15770946311133774,
  0.06255063924866554,
  -0.01550298589628618,
  -0.0533870784592822,
  -0.04810788606906059,
  -0.014989388364962738,
  0.02255944841411092,
  0.04544033608407664,
  0.04683589875623874,
  0.03263961578701652,
  0.015889163933487414,
  -0.019008758166749587
};

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
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, SAMPLE_PERIOD_MILLI); // TOM
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
        APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER));

        int i;
        for (i = 0; i < SAMPLES_IN_BUFFER; ++i)
        {
            dataBuffer[i] = (float32_t) ADC_RESULT_IN_MILLI_VOLTS(p_event->data.done.p_buffer[i]);
        }
        //NRF_LOG_INFO("%d",p_event->data.done.p_buffer[0]);

        // Signal the data processing task
        sendNotification(SAADC_BUFFER_NOTIFICATION);
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
void taskFIR (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    NRF_LOG_INFO("Checkpoint: beginning of taskFIR");

    uint16_t dataBufferFilteredDownSampled[SAMPLES_IN_BUFFER/2];

    while (true)
    {
        waitForNotification(SAADC_BUFFER_NOTIFICATION);

        //bsp_board_led_on(1);
        arm_fir_f32(&S, dataBuffer, dataBufferFiltered, blockSize);
        //bsp_board_led_off(1);

        int i;
        for(i = 0; i < SAMPLES_IN_BUFFER; ++i)
        {
            if (i % 2 == 0)
            {
                dataBufferFilteredDownSampled[i/2] = (uint16_t)dataBufferFiltered[i];
                //NRF_LOG_INFO("%d", dataBufferFilteredDownSampled[i/2]);
            }
        }
        int size = sizeof(dataBufferFilteredDownSampled);
        pendingMessagesPush(size, (char*)dataBufferFilteredDownSampled, &globalQ);
    }
}


int ecgInit(struct ecgObject_t * inEcgObject_ptr)
{
    // ********** DSP ************//
    arm_fir_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);
    // ************ END DSP ************ //

    this = &ecgObject;

    // initialize ADC if not already initialized
    // Actually currently not checking
    saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();

    // create FreeRtos tasks
    BaseType_t retVal = xTaskCreate(taskFIR, "LED0", configMINIMAL_STACK_SIZE + 60, NULL, 2, &taskFIRHandle);
    if (retVal == pdPASS)
    {
        NRF_LOG_INFO("ecg SENSOR GPIO TOGGLE THREAD CREATED");
    }
    else if (retVal == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        NRF_LOG_INFO("ecg SENSOR GPIO TOGGLE NEED MORE HEAP!!!!!!!!");
    }
    else
    {
        NRF_LOG_INFO("ecg SENSOR GPIO TOGGLE THREAD DID NOT PASS XXXXXXXXX");
    }

    NRF_LOG_INFO("Checkpoint: end of ecgInit");

    inEcgObject_ptr = this;
    return 0;
}
