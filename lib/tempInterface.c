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

void taskTemperatureData (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    NRF_LOG_INFO("Checkpoint: beginning of taskFIR");

    uint32_t temperatureSum = 0;
    uint16_t temperatureAverage = 0;

    while (true)
    {
        waitForNotification(TEMPERATURE_BUFFER_FULL_NOTIFICATION);

        int i;
        temperatureSum = 0;
        for(i = 0; i < SAMPLES_PER_CHANNEL; ++i)
        {
            temperatureSum += temperatureDataBuffer[i];
            //NRF_LOG_INFO("%d", temperatureDataBuffer[i]);
        }
        temperatureAverage = temperatureSum / SAMPLES_PER_CHANNEL;
        pendingMessagesPush(sizeof(temperatureAverage), (char*)&temperatureAverage, &globalQ);
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

    checkReturn(xTaskCreate(taskTemperatureData, "x", configMINIMAL_STACK_SIZE + 60, NULL, 2, &taskTemperatureDataHandle));

    return 0;
}

