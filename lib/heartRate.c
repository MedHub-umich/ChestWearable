// heartRate.c

#include "heartRate.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static arm_fir_instance_f32 heartRateLowPassInstance;
#define HEART_RATE_LOW_PASS_BLOCK_SIZE 34
#define HEART_RATE_LOW_PASS_NUM_TAPS 101
static float32_t heartRateLowPassState[HEART_RATE_LOW_PASS_BLOCK_SIZE + HEART_RATE_LOW_PASS_NUM_TAPS - 1];
static float32_t heartRateLowPassData[HEART_RATE_LOW_PASS_BLOCK_SIZE];

static float32_t heartRateLowPassTaps[HEART_RATE_LOW_PASS_NUM_TAPS] = {
0.00011276,-2.6888e-05,-0.00019975,-0.00022221,-1.9591e-05,0.00012975,1.3397e-05,-5.6568e-05,0.00034716,0.00086583,0.00049531,-0.00092667,-0.0019435,-0.0011019,0.00084958,0.0016241,0.00061903,1.3829e-05,0.0013603,0.0023409,-0.00048678,-0.0056509,-0.0067251,-0.00052808,0.0070909,0.007639,0.0014375,-0.0024605,0.00034771,0.0019882,-0.0059008,-0.016076,-0.011358,0.010608,0.02787,0.019104,-0.0067881,-0.019744,-0.0092368,-5.503e-05,-0.014531,-0.030718,-0.0042047,0.061894,0.09481,0.029389,-0.096252,-0.15581,-0.068213,0.097694,0.18101,0.097694,-0.068213,-0.15581,-0.096252,0.029389,0.09481,0.061894,-0.0042047,-0.030718,-0.014531,-5.503e-05,-0.0092368,-0.019744,-0.0067881,0.019104,0.02787,0.010608,-0.011358,-0.016076,-0.0059008,0.0019882,0.00034771,-0.0024605,0.0014375,0.007639,0.0070909,-0.00052808,-0.0067251,-0.0056509,-0.00048678,0.0023409,0.0013603,1.3829e-05,0.00061903,0.0016241,0.00084958,-0.0011019,-0.0019435,-0.00092667,0.00049531,0.00086583,0.00034716,-5.6568e-05,1.3397e-05,0.00012975,-1.9591e-05,-0.00022221,-0.00019975,-2.6888e-05,0.00011276
};


float32_t ecgheartRateLowPass[HEART_RATE_LOW_PASS_BLOCK_SIZE];

// Heart Rate Amplitude Stuff
float32_t averageAmplitudeForThisBuffer = 0;
float32_t factor = 10.0;
//float32_t offset = 100.0;
float32_t longTermAverage = 15;
float32_t peakAmplitudeThreshold = 0;


// Heart Rate Time Stuff
int samplesSinceMax = 0;
int thresholdSamplesSinceMax = 75;
float32_t currMax = 0;
float32_t heartRate = 0;
int samplesSinceLastPeak = 0;
const int samplePeriodMilli = 2;

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

void heartRateExtract(float32_t * inEcgDataBuffer, int inSize)
{
    // Heart Rate Band Pass
    arm_fir_f32(&heartRateLowPassInstance, inEcgDataBuffer, ecgheartRateLowPass, HEART_RATE_LOW_PASS_BLOCK_SIZE);

    int i;
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
    peakAmplitudeThreshold = factor * longTermAverage;// + offset; // Adding in a 5000 term to mitigate problems from noise

    for(i = 0; i < HEART_RATE_LOW_PASS_BLOCK_SIZE; ++i)
    {
       if (ecgheartRateLowPass[i] > peakAmplitudeThreshold && ecgheartRateLowPass[i] > currMax)
       {
          currMax = ecgheartRateLowPass[i];
          samplesSinceMax = 0;
       }
       else
       {
          ++samplesSinceMax;
          if (samplesSinceMax >= thresholdSamplesSinceMax && currMax > peakAmplitudeThreshold)
          {
            heartRate = 60 / (samplesSinceLastPeak * (samplePeriodMilli / 1000.0));
            NRF_LOG_INFO("HeartRate: %d", (uint16_t) heartRate);
            NRF_LOG_INFO("The max: %d", (uint16_t) currMax);
            respirationRateAddPair(currMax, samplesSinceLastPeak, &respiration);
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
}

void heartRateInit()
{
    respirationRateInit(&respiration);
    arm_fir_init_f32(&heartRateLowPassInstance, HEART_RATE_LOW_PASS_NUM_TAPS, (float32_t *)&heartRateLowPassTaps[0], &heartRateLowPassState[0], HEART_RATE_LOW_PASS_BLOCK_SIZE);
}

