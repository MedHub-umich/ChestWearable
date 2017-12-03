// cardioInterface.c

#include "cardioInterface.h"
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
#include "saadcInterface.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "notification.h"
// FILTER
#include "app_util_platform.h"
#include "arm_const_structs.h"
#include "heartRate.h"


ecg ecgDevice;
// Filter
#define NUM_TAPS              27
#define BLOCK_SIZE SAMPLES_PER_CHANNEL
TaskHandle_t  taskCardioProcessingHandle;
static arm_fir_instance_f32 S;
static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
static const uint32_t blockSize = BLOCK_SIZE;
static float32_t ecgDataBufferFiltered[SAMPLES_PER_CHANNEL];
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


void taskCardioProcessing (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    float32_t ecgDataBufferCopy[SAMPLES_PER_CHANNEL];
    uint16_t ecgDataBufferFilteredDownSampled[SAMPLES_PER_CHANNEL/2];

    while (true)
    {
        waitForNotification(ECG_BUFFER_FULL_NOTIFICATION);

        //nrf_gpio_pin_write(7, 1);

        // Make a copy, probably not necessary
        int i;
        for(i = 0; i < SAMPLES_PER_CHANNEL; ++i)
        {
            ecgDataBufferCopy[i] = ecgDataBuffer[i];
        }

        // Low Pass Filter
        arm_fir_f32(&S, ecgDataBuffer, ecgDataBufferFiltered, blockSize);

        // Down Sample
        for(i = 0; i < SAMPLES_PER_CHANNEL; ++i)
        {
            if (i % 2 == 0)
            {
                ecgDataBufferFilteredDownSampled[i/2] = (uint16_t)ecgDataBufferFiltered[i];
                NRF_LOG_INFO("%d", ecgDataBufferFilteredDownSampled[i/2]);
            }
        }

        // Package up the cardio data
        addToPackage((char*) ecgDataBufferFilteredDownSampled, sizeof(ecgDataBufferFilteredDownSampled), &ecgDevice.ecgPackager);

        //vTaskDelay(  5000);

        // Heart Rate
        heartRateExtract(ecgDataBufferCopy , SAMPLES_PER_CHANNEL);

        //nrf_gpio_pin_write(7,0);
    }
}


static void checkReturn(BaseType_t retVal)
{
    if (retVal == pdPASS)
    {
        NRF_LOG_INFO("ECG THREAD CREATED");
    }
    else if (retVal == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        NRF_LOG_INFO("SENSOR THREAD NEED MORE HEAP!!!!!!!!");
    }
    else
    {
        NRF_LOG_INFO("SENSOR THREAD DID NOT PASS XXXXXXXXX");
    }
}


int cardioInit()
{
    //nrf_gpio_cfg_output(7);
    //nrf_gpio_pin_clear(7);

    saadcInterfaceInit();
    heartRateInit();

    arm_fir_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);

    packagerInit(ECG_DATA_TYPE, ECG_DATA_PACKET_SIZE, &ecgDevice.ecgPackager);

    // create FreeRtos tasks
    checkReturn(xTaskCreate(taskCardioProcessing, "LED0", configMINIMAL_STACK_SIZE + 800, NULL, 2, &taskCardioProcessingHandle));



    return 0;
}

