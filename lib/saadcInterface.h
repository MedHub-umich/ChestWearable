#ifndef SAADCINTERFACE_H_
#define SAADCINTERFACE_H_

#include "saadcInterface.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "notification.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_util_platform.h"
#include "arm_const_structs.h"

// SAADC
#define ECG_CHANNEL                     NRF_SAADC_INPUT_AIN5
#define TEMPERATURE_CHANNEL             NRF_SAADC_INPUT_AIN1
#define SAMPLE_PERIOD_MILLI             2
#define SAMPLES_PER_CHANNEL             34 // MUST BE DIVISIBLE BY DOWNSAMPLE FACTOR
#define NUM_CHANNELS                    2
#define SAMPLES_TOTAL                   NUM_CHANNELS*SAMPLES_PER_CHANNEL
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS   600
#define ADC_PRE_SCALING_COMPENSATION    6
#define ADC_RES_10BIT                   1024
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)
static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1);
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_TOTAL];
static nrf_ppi_channel_t     m_ppi_channel;
static nrf_saadc_value_t dummy = 0xBEEF;

// Public
void saadcInterfaceInit(void);
static float32_t ecgDataBuffer[SAMPLES_PER_CHANNEL];
static uint16_t temperatureDataBuffer[SAMPLES_PER_CHANNEL];

// Private
void saadc_sampling_event_init(void);
void saadc_sampling_event_enable(void);
void saadc_callback(nrf_drv_saadc_evt_t const * p_event);
void saadc_init(void);
void timer_handler(nrf_timer_event_t event_type, void * p_context);

#endif // SAADCINTERFACE_H_