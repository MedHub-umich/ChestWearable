// respirationRate.c

#include "respirationRate.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "pendingMessages.h"


TaskHandle_t  taskSendHandle;
SemaphoreHandle_t respirationRateSemaphore;

static uint8_t averageRespirationRateGlobal = 0;
static const TickType_t sendPeriodMilli = 30000; // one minute is 30000 for some reason

void respirationRateAddPair(float32_t inMagnitude, float32_t inTime, respirationRate_t * this)
{
    peakPair_t newPeak;
    newPeak.magnitude = inMagnitude;
    newPeak.time = inTime;
    if(this->numPeaks >= PEAKS_SIZE)
    {
        NRF_LOG_ERROR("BAADDD: too many peaks, BUFFER OVERFFLOOW ATTACK");
    }
    this->peaks[this->numPeaks] = newPeak;
    this->numPeaks++;

    if(this->numPeaks >= PEAKS_SIZE)
    {
        respirationRateProcess(this);
    }
}

static float32_t calcLongTermAverage(float32_t new, float32_t average)
{
    static const float32_t num = 3.0;

    average -= (average / num);
    average += (new / num);

    return average;
}

void respirationRateProcess(respirationRate_t * this)
{

    static float32_t averageRespirationRate = 15.0;

    float32_t numCrossings = calcNumCrossingsInData(this);
    
    float32_t totalTimeInSeconds = calcTotalTimeElapsedDuringData(this);

    float32_t currentRespirationRate = numCrossings / CROSSINGS_PER_BREATH * SECONDS_PER_MINUTE / totalTimeInSeconds;

    averageRespirationRate = calcLongTermAverage(currentRespirationRate, averageRespirationRate);

    NRF_LOG_INFO("AVERAGE RESPIRATION RATE FROM FUNCTION: %d", (int) averageRespirationRate);

    xSemaphoreTake( respirationRateSemaphore, portMAX_DELAY );
    averageRespirationRateGlobal = (uint8_t) averageRespirationRate;
    //NRF_LOG_INFO("UINT8_T AVERAGE RESPIRATION RATE FROM FUNCTION: %d", averageRespirationRateGlobal);
    xSemaphoreGive( respirationRateSemaphore );

    this->numPeaks = 0;
}


void taskSend(void * pvParameter)
{
    respirationRate_t* breathingRateSensor = (respirationRate_t*) pvParameter;
    

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount ();

    uint8_t sendingRespirationRate = 0;

    while (true)
    {
        vTaskDelayUntil( &xLastWakeTime, sendPeriodMilli );

        xSemaphoreTake( respirationRateSemaphore, portMAX_DELAY );
        sendingRespirationRate = averageRespirationRateGlobal;
        xSemaphoreGive( respirationRateSemaphore );

        NRF_LOG_INFO("SENDING RESPIRATION RATE: %d", sendingRespirationRate);
        addToPackage((char*) &sendingRespirationRate, sizeof(sendingRespirationRate), &breathingRateSensor->breathingRatePackager);
    }
}


static void checkReturn(BaseType_t retVal)
{
    if (retVal == pdPASS)
    {
        NRF_LOG_INFO("BREATHING SEND THREAD CREATED");
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


void respirationRateInit(respirationRate_t * this)
{
    this->numPeaks = 0;

    respirationRateSemaphore = xSemaphoreCreateMutex();
    uint8_t sendingRespirationRate = 10;
    packagerInit(BREATHING_RATE_DATA_TYPE, BR_DATA_PACKET_SIZE, &this->breathingRatePackager);
    addToPackage((char*) &sendingRespirationRate, sizeof(sendingRespirationRate), &this->breathingRatePackager);
    NRF_LOG_INFO("Sent Brathing");

    // create FreeRtos tasks
    checkReturn(xTaskCreate(taskSend, "x", configMINIMAL_STACK_SIZE + 60, (void*) this, 2, &taskSendHandle));
}

float32_t calcTotalTimeElapsedDuringData(respirationRate_t * this)
{
    float32_t totalTimeOutput;
    int i;
    for(i = 0; i < PEAKS_SIZE; ++i)
    {
        totalTimeOutput += (this->peaks[i].time)/500.0;
    }
    return totalTimeOutput;
}

float32_t calcAverageValueOfData(respirationRate_t * this)
{
    float32_t sum = 0;

    int i;
    for(i = 0; i < PEAKS_SIZE; ++i)
    {
        sum += this->peaks[i].magnitude;
    }

    float32_t average = sum / PEAKS_SIZE;

    return average;
}

float32_t calcNumCrossingsInData(respirationRate_t * this)
{
    float32_t average = calcAverageValueOfData(this);

    int isAbove, crossings = 0;

    if(this->peaks[0].magnitude > average)
    {
        isAbove = 1;
    }
    else
    {
        isAbove = 0;
    }

    int i;
    for(i = 1; i < PEAKS_SIZE; ++i)
    {
        if (this->peaks[i].magnitude > average && isAbove == 0)
        {
            isAbove = 1;
            crossings++;
        }
        else if (this->peaks[i].magnitude > average && isAbove == 1)
        {
            // Nothing
        }
        else if (this->peaks[i].magnitude < average && isAbove == 1)
        {
            isAbove = 0;
            crossings++;
        }
        else // (this->peaks[i] < average && isAbove = 0)
        {
            // Nothing
        }
    }

    return (float32_t) crossings;
}
