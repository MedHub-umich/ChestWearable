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

// Filter
#define NUM_TAPS              27
#define BLOCK_SIZE SAMPLES_PER_CHANNEL
TaskHandle_t  taskFIRHandle;
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

static arm_fir_instance_f32 heartRateLowPassInstance;
#define HEART_RATE_LOW_PASS_BLOCK_SIZE BLOCK_SIZE
#define HEART_RATE_LOW_PASS_NUM_TAPS 101
static float32_t heartRateLowPassState[HEART_RATE_LOW_PASS_BLOCK_SIZE + HEART_RATE_LOW_PASS_NUM_TAPS - 1];
static float32_t heartRateLowPassData[HEART_RATE_LOW_PASS_BLOCK_SIZE];
static float32_t heartRateLowPassTaps[HEART_RATE_LOW_PASS_NUM_TAPS] = {
0.00011276,-2.6888e-05,-0.00019975,-0.00022221,-1.9591e-05,0.00012975,1.3397e-05,-5.6568e-05,0.00034716,0.00086583,0.00049531,-0.00092667,-0.0019435,-0.0011019,0.00084958,0.0016241,0.00061903,1.3829e-05,0.0013603,0.0023409,-0.00048678,-0.0056509,-0.0067251,-0.00052808,0.0070909,0.007639,0.0014375,-0.0024605,0.00034771,0.0019882,-0.0059008,-0.016076,-0.011358,0.010608,0.02787,0.019104,-0.0067881,-0.019744,-0.0092368,-5.503e-05,-0.014531,-0.030718,-0.0042047,0.061894,0.09481,0.029389,-0.096252,-0.15581,-0.068213,0.097694,0.18101,0.097694,-0.068213,-0.15581,-0.096252,0.029389,0.09481,0.061894,-0.0042047,-0.030718,-0.014531,-5.503e-05,-0.0092368,-0.019744,-0.0067881,0.019104,0.02787,0.010608,-0.011358,-0.016076,-0.0059008,0.0019882,0.00034771,-0.0024605,0.0014375,0.007639,0.0070909,-0.00052808,-0.0067251,-0.0056509,-0.00048678,0.0023409,0.0013603,1.3829e-05,0.00061903,0.0016241,0.00084958,-0.0011019,-0.0019435,-0.00092667,0.00049531,0.00086583,0.00034716,-5.6568e-05,1.3397e-05,0.00012975,-1.9591e-05,-0.00022221,-0.00019975,-2.6888e-05,0.00011276
};

// ****************** Heart Rate ******************** //

static float32_t calcAverageAmplitudeForThisBuffer(float32_t * inBuffer, uint16_t inSize)
{
    uint32_t i;
    float32_t runningTotal = 0;
    for(i = 0; i < inSize; ++i)
    {
      runningTotal += inBuffer[i];
    }
    return (runningTotal / inSize);
}

static float32_t calcLongTermAverage(float32_t averageAmplitudeForThisBuffer, float32_t longTermAverage)
{
    static const float32_t numAverageAmplitudes = 100.0;

    longTermAverage -= (longTermAverage / numAverageAmplitudes);
    longTermAverage += (averageAmplitudeForThisBuffer / numAverageAmplitudes);

    return longTermAverage;
}


void taskFIR (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    uint16_t ecgDataBufferFilteredDownSampled[SAMPLES_PER_CHANNEL/2];
    float32_t ecgDataBufferCopy[HEART_RATE_LOW_PASS_BLOCK_SIZE];
    float32_t ecgheartRateLowPass[HEART_RATE_LOW_PASS_BLOCK_SIZE];

    // Heart Rate Amplitude Stuff
    float32_t averageAmplitudeForThisBuffer = 0;
    float32_t longTermAverage = 250;
    float32_t peakAmplitudeThreshold = 0;
    float32_t factor = 25.0;

    // Heart Rate Time Stuff
    int samplesSinceMax = 0;
    int thresholdSamplesSinceMax = 75;
    float32_t currMax = 0;
    float32_t heartRate = 0;
    int samplesSinceLastPeak = 0;

    while (true)
    {
        waitForNotification(ECG_BUFFER_FULL_NOTIFICATION);

        nrf_gpio_pin_write(27, 1);
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

        // BLE
        int size = sizeof(ecgDataBufferFilteredDownSampled);
        pendingMessagesPush(size, (char*)ecgDataBufferFilteredDownSampled, &globalQ);

        // Heart Rate Band Pass
        arm_fir_f32(&heartRateLowPassInstance, ecgDataBufferCopy, ecgheartRateLowPass, HEART_RATE_LOW_PASS_BLOCK_SIZE);

        // Heart Rate Signal Energy
        for(i = 0; i < HEART_RATE_LOW_PASS_BLOCK_SIZE; ++i)
        {
           ecgheartRateLowPass[i] = ecgheartRateLowPass[i] * ecgheartRateLowPass[i];
        }

        for(i = 0; i < HEART_RATE_LOW_PASS_BLOCK_SIZE; ++i)
        {
           //NRF_LOG_INFO("%d", (int16_t)ecgheartRateLowPass[i]);
           //NRF_LOG_INFO("Output: " NRF_LOG_FLOAT_MARKER "\r", NRF_LOG_FLOAT(ecgheartRateLowPass[i]));
        }

       


        // Heart Rate
        averageAmplitudeForThisBuffer = calcAverageAmplitudeForThisBuffer(ecgheartRateLowPass, HEART_RATE_LOW_PASS_BLOCK_SIZE);
        longTermAverage = calcLongTermAverage(averageAmplitudeForThisBuffer, longTermAverage);
        peakAmplitudeThreshold = factor * longTermAverage * 0.25 + 5000 * 0.75; // Adding in a 5000 term to mitigate problems from noise

        for(i = 0; i < HEART_RATE_LOW_PASS_BLOCK_SIZE; ++i)
        {
           if (ecgheartRateLowPass[i] > peakAmplitudeThreshold && ecgheartRateLowPass[i] > currMax) {
              currMax = ecgheartRateLowPass[i];
              samplesSinceMax = 0;
           } else {
              ++samplesSinceMax;
              if (samplesSinceMax >= thresholdSamplesSinceMax && currMax > peakAmplitudeThreshold) {
                heartRate = 60 / (samplesSinceLastPeak * (SAMPLE_PERIOD_MILLI / 1000.0));
                NRF_LOG_INFO("HeartRate: %d", (uint16_t) heartRate);
                NRF_LOG_INFO("The max: %d", (uint16_t) currMax);
                //NRF_LOG_INFO("Samples since last peak: %d", samplesSinceLastPeak);
                //add to list of breathing rate peaks, if greather than a thershold wake up breathing rate algo
                samplesSinceLastPeak = 0;
                currMax = 0;
                samplesSinceMax = 0;
              }
           }
           ++samplesSinceLastPeak;
        }

        //NRF_LOG_INFO("longTermAverage %d", (uint32_t) longTermAverage);
        NRF_LOG_INFO("peakAmplitudeThreshold %d", (uint32_t) peakAmplitudeThreshold);

        nrf_gpio_pin_write(27,0);
    }

}


static void checkReturn(BaseType_t retVal)
{
    if (retVal == pdPASS)
    {
        NRF_LOG_INFO("SENSOR THREAD CREATED");
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
    nrf_gpio_cfg_output(27);
    nrf_gpio_pin_clear(27);

    saadcInterfaceInit();

    arm_fir_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);
    arm_fir_init_f32(&heartRateLowPassInstance, HEART_RATE_LOW_PASS_NUM_TAPS, (float32_t *)&heartRateLowPassTaps[0], &heartRateLowPassState[0], HEART_RATE_LOW_PASS_BLOCK_SIZE);

    // create FreeRtos tasks
    checkReturn(xTaskCreate(taskFIR, "LED0", configMINIMAL_STACK_SIZE + 800, NULL, 2, &taskFIRHandle));

    return 0;
}

