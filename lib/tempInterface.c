#include "tempInterface.h"
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

TaskHandle_t  taskTemperatureDataHandle;
TaskHandle_t sendingTemperatureDataHandle;


Temp tempDevice;

SemaphoreHandle_t temperatureSendSemaphore;
static const TickType_t sendPeriodMilli = 3000; // one minute is 30000 for some reason

uint8_t globalTemperatureAverage = 0;

static float32_t calcLongTermAverage(float32_t currMeasurment, float32_t average)
{
    static const float32_t num = 3.0;

    average -= (average / num);
    average += (currMeasurment / num);

    return average;
}

static float32_t milliVoltsToCelsius(float32_t milliVolts)
{
    const float32_t m = -116.0;
    const float32_t b = 60.9;
    const float32_t milliVoltsToVolts = 0.001;
    const float32_t offset = -12.0;

    return m * (milliVoltsToVolts * milliVolts) + b + offset;
}

void taskTemperatureData (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    NRF_LOG_INFO("Checkpoint: beginning of taskFIR");

    float32_t temperatureSum = 0.0;
    float32_t temperatureAverage = 0.0;
    static float32_t runningAverage = 35.0;

    while (true)
    {
        waitForNotification(TEMPERATURE_BUFFER_FULL_NOTIFICATION);

        int i;
        temperatureSum = 0;
        for(i = 0; i < SAMPLES_PER_CHANNEL; ++i)
        {
            temperatureSum += milliVoltsToCelsius( (float32_t) temperatureDataBuffer[i]);
            //NRF_LOG_INFO("%d", temperatureDataBuffer[i]);
        }
        temperatureAverage = temperatureSum / SAMPLES_PER_CHANNEL;

        runningAverage = calcLongTermAverage(temperatureAverage, runningAverage);

        xSemaphoreTake( temperatureSendSemaphore, portMAX_DELAY );
        globalTemperatureAverage = (uint8_t) runningAverage;
        xSemaphoreGive( temperatureSendSemaphore );

        //pendingMessagesPush(sizeof(temperatureAverage), (char*)&temperatureAverage, &globalQ);
    }
}


void temperatureTaskSend(void * pvParameter)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount ();

    uint8_t sendingTemperature = 0;

    while (true)
    {
        vTaskDelayUntil( &xLastWakeTime, sendPeriodMilli ); //run once a minute

        xSemaphoreTake( temperatureSendSemaphore, portMAX_DELAY );
        sendingTemperature = globalTemperatureAverage;
        xSemaphoreGive( temperatureSendSemaphore );

        addToPackage((char*) &sendingTemperature, sizeof(sendingTemperature), &tempDevice.tempPackager);
        NRF_LOG_INFO("Packaging the following temperature: %d", sendingTemperature);
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

int tempInit()
{
    saadcInterfaceInit();
    packagerInit(TEMPERATURE_DATA_TYPE, TEMPERATURE_DATA_PACKET_SIZE, &tempDevice.tempPackager);
    temperatureSendSemaphore = xSemaphoreCreateMutex();
    checkReturn(xTaskCreate(taskTemperatureData, "x", configMINIMAL_STACK_SIZE + 60, NULL, 2, &taskTemperatureDataHandle));
    checkReturn(xTaskCreate(temperatureTaskSend, "x", configMINIMAL_STACK_SIZE + 60, NULL, 2, &sendingTemperatureDataHandle));

    return 0;
}

