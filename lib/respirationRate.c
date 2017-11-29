// respirationRate.c

#include "respirationRate.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// #include "FreeRTOS.h"
// #include "task.h"
// #include "semphr.h"

// #include "pendingMessages.h"

// TaskHandle_t  taskSendHandle;
// SemaphoreHandle_t respirationRateSemaphore;

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

void respirationRateProcess(respirationRate_t * this)
{
    int sum = 0, average = 0;
    int i;
    for(i = 0; i < PEAKS_SIZE; ++i)
    {
        sum += this->peaks[i].magnitude;
    }
    average = sum / PEAKS_SIZE;
    //NRF_LOG_INFO("average: %d", average);

    int isAbove, crossings = 0;

    if(this->peaks[0].magnitude > average)
    {
        isAbove = 1;
    }
    else
    {
        isAbove = 0;
    }

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

    float32_t totalTime = 0;
    for(i = 0; i < PEAKS_SIZE; ++i)
    {
        totalTime += (this->peaks[i].time)/500.0;
    }



    //NRF_LOG_INFO("crossings: %d", crossings);
    //NRF_LOG_INFO("breaths: %d", crossings/2);
    //NRF_LOG_INFO("totalTime: %d", totalTime);
    NRF_LOG_INFO("BREATHS PER MINUTE %d", (int)(crossings/2*60/totalTime));

    this->numPeaks = 0;
}


// void taskSend(void * pvParameter)
// {
//     UNUSED_PARAMETER(pvParameter);

//     const TickType_t xFrequency = 10;

//     TickType_t xLastWakeTime;
//     xLastWakeTime = xTaskGetTickCount ();

//     uint8_t currentRespirationRate = 0;

//     while (true)
//     {
//         xSemaphoreTake( respirationRateSemaphore, portMAX_DELAY );
//         // grab the respiration rate
//         currentRespirationRate = ;
//         xSemaporeGive( respirationRateSemaphore, portMAX_DELAY );

//         NRF_LOG_INFO("Processed respiration rate: %d", currentRespirationRate);

//         vTaskDelayUntil( &xLastWakeTime, xFrequency );
//     }
// }


// static void checkReturn(BaseType_t retVal)
// {
//     if (retVal == pdPASS)
//     {
//         NRF_LOG_INFO("SENSOR THREAD CREATED");
//     }
//     else if (retVal == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
//     {
//         NRF_LOG_INFO("SENSOR THREAD NEED MORE HEAP!!!!!!!!");
//     }
//     else
//     {
//         NRF_LOG_INFO("SENSOR THREAD DID NOT PASS XXXXXXXXX");
//     }
// }


void respirationRateInit(respirationRate_t * this)
{
    this->numPeaks = 0;

    // respirationRateSemaphore = xSemaphoreCreateMutex();

    // // create FreeRtos tasks
    // checkReturn(xTaskCreate(taskSend, "T", configMINIMAL_STACK_SIZE + 60, NULL, 2, &taskSendHandle));
}
