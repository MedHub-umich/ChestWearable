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
static const int heartRateLowPassBlockSize = BLOCK_SIZE;
static const int numHeartRateLowPassTaps = 157;
static float32_t heartRateLowPassState[heartRateLowPassBlockSize + numHeartRateLowPassTaps - 1];
static float32_t heartRateLowPassData[heartRateLowPassBlockSize];
static float32_t heartRateLowPassTaps[numHeartRateLowPassTaps] = {
  0.005747275144764403,
  0.0023468683116059,
  0.002659789308478933,
  0.002869614374419817,
  0.0029496592187077553,
  0.0028818374834934154,
  0.0026532577263599154,
  0.00226197957979133,
  0.001724495774383823,
  0.0010598609598767386,
  0.0003086216865700871,
  -0.00048591271941989076,
  -0.0012709340868750955,
  -0.0019901661155818273,
  -0.0025883777896462128,
  -0.0030154898708769848,
  -0.00322926529617709,
  -0.003203588916486325,
  -0.002926506964294118,
  -0.0024062771506167474,
  -0.0016698625906981903,
  -0.0007624748972384352,
  0.00025396978433584995,
  0.0013047062543759672,
  0.002308084199564374,
  0.003179671413138975,
  0.0038411504071846434,
  0.004225176106027155,
  0.004282016584712055,
  0.003984898149176956,
  0.0033330387410852222,
  0.002353788097670533,
  0.0011030475075215437,
  -0.0003383437954851411,
  -0.0018679819470013596,
  -0.0033693393156674317,
  -0.004720052540245941,
  -0.005800553078085297,
  -0.0065044542979652,
  -0.006747578918271766,
  -0.006475770831752035,
  -0.005672136416107766,
  -0.00436021554214034,
  -0.002605270032851261,
  -0.0005128286910271003,
  0.0017774834939584517,
  0.004099077008248188,
  0.006269488409354166,
  0.008103400506924105,
  0.009425867694491405,
  0.010086872370188488,
  0.009975021007762385,
  0.009027804406053492,
  0.007242091880153391,
  0.0046779045681573815,
  0.001460080625886543,
  -0.002224900539969955,
  -0.00613747557821139,
  -0.009997075422535688,
  -0.013496196083622368,
  -0.01632110078613998,
  -0.01816804973241476,
  -0.018766063942260547,
  -0.01789525257727796,
  -0.015403761225477654,
  -0.011222005307857406,
  -0.005372760798296031,
  0.002030163299932881,
  0.010776394405298581,
  0.020575699334018392,
  0.031063278896639908,
  0.041819240240292835,
  0.052390908117101107,
  0.06231832456731873,
  0.07114682868005769,
  0.07849309980326598,
  0.08399354844130537,
  0.08740531991102928,
  0.08856330760179881,
  0.08740531991102928,
  0.08399354844130537,
  0.07849309980326598,
  0.07114682868005769,
  0.06231832456731873,
  0.052390908117101107,
  0.041819240240292835,
  0.031063278896639908,
  0.020575699334018392,
  0.010776394405298581,
  0.002030163299932881,
  -0.005372760798296031,
  -0.011222005307857406,
  -0.015403761225477654,
  -0.01789525257727796,
  -0.018766063942260547,
  -0.01816804973241476,
  -0.01632110078613998,
  -0.013496196083622368,
  -0.009997075422535688,
  -0.00613747557821139,
  -0.002224900539969955,
  0.001460080625886543,
  0.0046779045681573815,
  0.007242091880153391,
  0.009027804406053492,
  0.009975021007762385,
  0.010086872370188488,
  0.009425867694491405,
  0.008103400506924105,
  0.006269488409354166,
  0.004099077008248188,
  0.0017774834939584517,
  -0.0005128286910271003,
  -0.002605270032851261,
  -0.00436021554214034,
  -0.005672136416107766,
  -0.006475770831752035,
  -0.006747578918271766,
  -0.0065044542979652,
  -0.005800553078085297,
  -0.004720052540245941,
  -0.0033693393156674317,
  -0.0018679819470013596,
  -0.0003383437954851411,
  0.0011030475075215437,
  0.002353788097670533,
  0.0033330387410852222,
  0.003984898149176956,
  0.004282016584712055,
  0.004225176106027155,
  0.0038411504071846434,
  0.003179671413138975,
  0.002308084199564374,
  0.0013047062543759672,
  0.00025396978433584995,
  -0.0007624748972384352,
  -0.0016698625906981903,
  -0.0024062771506167474,
  -0.002926506964294118,
  -0.003203588916486325,
  -0.00322926529617709,
  -0.0030154898708769848,
  -0.0025883777896462128,
  -0.0019901661155818273,
  -0.0012709340868750955,
  -0.00048591271941989076,
  0.0003086216865700871,
  0.0010598609598767386,
  0.001724495774383823,
  0.00226197957979133,
  0.0026532577263599154,
  0.0028818374834934154,
  0.0029496592187077553,
  0.002869614374419817,
  0.002659789308478933,
  0.0023468683116059,
  0.005747275144764403
};

// ****************** Heart Rate ******************** //

static float32_t calcAverageAmplitudeForThisBuffer(uint16_t * inBuffer, uint16_t inSize)
{
    uint32_t i, runningTotal = 0;
    for(i = 0; i < inSize; ++i)
    {
      runningTotal += (uint32_t) inBuffer[i];
    }
    return (float32_t) (runningTotal / (uint32_t) inSize);
}

static float32_t calcPeakAmplitudeThreshold(float32_t averageAmplitudeForThisBuffer, float32_t peakAmplitudeThreshold)
{
    static const float32_t numAverageAmplitudes = 100.0;

    peakAmplitudeThreshold += averageAmplitudeForThisBuffer / numAverageAmplitudes;
    peakAmplitudeThreshold -= peakAmplitudeThreshold / numAverageAmplitudes;

    return peakAmplitudeThreshold;
}


void taskFIR (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    uint16_t ecgDataBufferFilteredDownSampled[SAMPLES_PER_CHANNEL/2];
    uint16_t ecgDataBufferCopy[SAMPLES_PER_CHANNEL];

    // Heart Rate Amplitude Stuff
    float32_t averageAmplitudeForThisBuffer = 0;
    float32_t peakAmplitudeThreshold = 4000;

    // Heart Rate Time Stuff
    static int samplesFromPreviousPeakToEnd;
    static int samplesFromBeginningToThisPeak;
    static int samplesFromPreviousPeakToThisPeak;
    static const int peakTimeThreshold = 256; // 250 ms
    static const int peakSampleThreshold = peakTimeThreshold / SAMPLE_PERIOD_MILLI;

    while (true)
    {
        waitForNotification(ECG_BUFFER_FULL_NOTIFICATION);

        nrf_gpio_pin_write(27, 1);
        int i;
        for(i = 0; i < SAMPLES_PER_CHANNEL; ++i)
        {
            ecgDataBufferCopy[i] = (uint16_t)ecgDataBuffer[i];
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

        // Heart Rate
        averageAmplitudeForThisBuffer = calcAverageAmplitudeForThisBuffer(ecgDataBufferCopy, SAMPLES_PER_CHANNEL);
        peakAmplitudeThreshold = calcPeakAmplitudeThreshold(averageAmplitudeForThisBuffer, peakAmplitudeThreshold);
        NRF_LOG_INFO("averageAmplitudeForThisBuffer %d", (uint32_t) averageAmplitudeForThisBuffer);
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
    arm_fir_init_f32(&heartRateLowPassInstance, numHeartRateLowPassTaps, (float32_t *)&heartRateLowPassTaps[0], &heartRateLowPassState[0], heartRateLowPassBlockSize);

    // create FreeRtos tasks
    checkReturn(xTaskCreate(taskFIR, "LED0", configMINIMAL_STACK_SIZE + 60, NULL, 2, &taskFIRHandle));

    return 0;
}

