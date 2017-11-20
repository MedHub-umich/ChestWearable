#include "saadcInterface.h"

// Public
void saadcInterfaceInit(void)
{
    saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();
}

// Private

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
    //nrf_gpio_pin_write(27, 1);

    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_TOTAL));

        int i;
        for (i = 0; i < SAMPLES_TOTAL; ++i)
        {
            if(i%2 == 0) // even
            {
                temperatureDataBuffer[i/2] = (uint16_t) ADC_RESULT_IN_MILLI_VOLTS(p_event->data.done.p_buffer[i]);
                //temperatureDataBuffer[i/2] = (uint16_t) p_event->data.done.p_buffer[i];
            }
            else // odd
            {
                ecgDataBuffer[i/2] = (float32_t) ADC_RESULT_IN_MILLI_VOLTS(p_event->data.done.p_buffer[i]);
            }
        }
        //NRF_LOG_INFO("%d",p_event->data.done.p_buffer[0]);

        // Signal the data processing task
        sendNotification(TEMPERATURE_BUFFER_FULL_NOTIFICATION);
        sendNotification(ECG_BUFFER_FULL_NOTIFICATION);
    }

    //nrf_gpio_pin_write(27, 0);
}


void saadc_init(void)
{
    ret_code_t err_code;

    nrf_saadc_channel_config_t channel_config_temperature =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(TEMPERATURE_CHANNEL);

    nrf_saadc_channel_config_t channel_config_ecg =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(ECG_CHANNEL);

    // Init SAADC
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    // Init temperature channel
    err_code = nrf_drv_saadc_channel_init(0, &channel_config_temperature);
    APP_ERROR_CHECK(err_code);

    // Init ecg channel
    err_code = nrf_drv_saadc_channel_init(1, &channel_config_ecg);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_TOTAL);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_TOTAL);
    APP_ERROR_CHECK(err_code);
}


void timer_handler(nrf_timer_event_t event_type, void * p_context)
{
    NRF_LOG_INFO("\n\n\n\nTIMER HANDLER RAN (IT SHOULD NOT)\n\n\n\n");
}

