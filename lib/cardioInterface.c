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
// #define NUM_TAPS              27
#define NUM_TAPS              202
#define BLOCK_SIZE SAMPLES_PER_CHANNEL
TaskHandle_t  taskCardioProcessingHandle;
static arm_fir_instance_f32 S;
static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
static const uint32_t blockSize = BLOCK_SIZE;
static float32_t ecgDataBufferFiltered[SAMPLES_PER_CHANNEL];
// static float32_t firCoeffs32[NUM_TAPS] = {
//   -0.019008758166749587,
//   0.015889163933487414,
//   0.03263961578701652,
//   0.04683589875623874,
//   0.04544033608407664,
//   0.02255944841411092,
//   -0.014989388364962738,
//   -0.04810788606906059,
//   -0.0533870784592822,
//   -0.01550298589628618,
//   0.06255063924866554,
//   0.15770946311133774,
//   0.23556353376462763,
//   0.26552471337330985,
//   0.23556353376462763,
//   0.15770946311133774,
//   0.06255063924866554,
//   -0.01550298589628618,
//   -0.0533870784592822,
//   -0.04810788606906059,
//   -0.014989388364962738,
//   0.02255944841411092,
//   0.04544033608407664,
//   0.04683589875623874,
//   0.03263961578701652,
//   0.015889163933487414,
//   -0.019008758166749587
// };

static float32_t firCoeffs32[NUM_TAPS] = {
-0.001036,-0.0014349,-0.0021691,-0.0028883,-0.0034458,-0.0036928,-0.0035091,-0.0028389,-0.0017155,
-0.00026678,0.0012988,0.002728,0.003773,0.0042478,0.0040739,0.0033057,0.0021254,0.00080855,-0.000336,
-0.001038,-0.0011324,-0.00060337,0.00040781,0.0016328,0.0027405,0.0034204,0.003465,0.0028285,0.0016463,
0.0002063,-0.001121,-0.0019779,-0.0021169,-0.0014729,-0.00019061,0.0014037,0.0028817,0.0038242,0.0039362,
0.0031344,0.0015816,-0.00034351,-0.002138,-0.0033057,-0.0034934,-0.0025967,-0.00080346,0.0014406,0.0035388,
0.0048987,0.0050954,0.0039986,0.001827,-0.00089083,-0.0034417,-0.0051131,-0.0053893,-0.0041063,-0.00152,
0.0017374,0.0048057,0.0068192,0.0071445,0.0055709,0.0023977,-0.0016119,-0.0054083,-0.0079233,-0.008363,
-0.0064466,-0.0025232,0.0024756,0.0072452,0.010436,0.01102,0.0086015,0.0035774,-0.0028972,-0.0091564,
-0.013429,-0.014312,-0.011186,-0.004468,0.0043879,0.013166,0.019389,0.020947,0.016699,0.0069026,
-0.0066513,-0.020863,-0.031881,-0.035891,-0.029973,-0.012842,0.014708,0.049774,0.087744,0.12309,
0.15035,0.16518,0.16518,0.15035,0.12309,0.087744,0.049774,0.014708,-0.012842,-0.029973,-0.035891,
-0.031881,-0.020863,-0.0066513,0.0069026,0.016699,0.020947,0.019389,0.013166,0.0043879,-0.004468,
-0.011186,-0.014312,-0.013429,-0.0091564,-0.0028972,0.0035774,0.0086015,0.01102,0.010436,0.0072452,
0.0024756,-0.0025232,-0.0064466,-0.008363,-0.0079233,-0.0054083,-0.0016119,0.0023977,0.0055709,
0.0071445,0.0068192,0.0048057,0.0017374,-0.00152,-0.0041063,-0.0053893,-0.0051131,-0.0034417,
-0.00089083,0.001827,0.0039986,0.0050954,0.0048987,0.0035388,0.0014406,-0.00080346,-0.0025967,
-0.0034934,-0.0033057,-0.002138,-0.00034351,0.0015816,0.0031344,0.0039362,0.0038242,0.0028817,
0.0014037,-0.00019061,-0.0014729,-0.0021169,-0.0019779,-0.001121,0.0002063,0.0016463,0.0028285,
0.003465,0.0034204,0.0027405,0.0016328,0.00040781,-0.00060337,-0.0011324,-0.001038,-0.000336,
0.00080855,0.0021254,0.0033057,0.0040739,0.0042478,0.003773,0.002728,0.0012988,-0.00026678,
-0.0017155,-0.0028389,-0.0035091,-0.0036928,-0.0034458,-0.0028883,-0.0021691,-0.0014349,-0.001036
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
                //NRF_LOG_INFO("%d", ecgDataBufferFilteredDownSampled[i/2]);
            }
        }

        // Package up the cardio data
        //int size = sizeof(ecgDataBufferFilteredDownSampled);
        //pendingMessagesPush(size, (char*)ecgDataBufferFilteredDownSampled, &globalQ);
        addToPackage((char*) ecgDataBufferFilteredDownSampled, sizeof(ecgDataBufferFilteredDownSampled), &ecgDevice.ecgPackager);

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

