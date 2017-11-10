// tempInterface.c
#include "tempInterface.h"
#include "FreeRTOS.h"
#include "task.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_gpio.h"

#define TASK_DELAY        400           /**< Task delay. Delays a LED0 task for 200 ms */
TaskHandle_t  taskToggleLedHandle;   /**< Reference to LED0 toggling FreeRTOS task. */


void taskToggleLed(void * pvParameter);

/**@taskToggleLed
 *
 * Blinks an LED
 *
 */
void taskToggleLed (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    int gpioState = 0;

    nrf_gpio_cfg_output(7);
    nrf_gpio_pin_clear(7);

    while (true)
    {

        // toggle gpio pin
        gpioState = !gpioState;
        nrf_gpio_pin_write(7, gpioState);

        // Delay (messy period)
        vTaskDelay(TASK_DELAY);
    }
}


int tempInit(void)
{
    // initialize ADC if not already initialized

    // create FreeRtos tasks
    BaseType_t retVal = xTaskCreate(taskToggleLed, "LED0", configMINIMAL_STACK_SIZE + 60, NULL, 2, &taskToggleLedHandle);

    if (retVal == pdPASS)
    {
        NRF_LOG_INFO("TEMP SENSOR BLINKY THREAD CREATED");
    }
    else if (retVal == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        NRF_LOG_INFO("TEMP SENSOR BLINKY THREAD NEED MORE HEAP!!!!!!!!");
    }
    else
    {
        NRF_LOG_INFO("TEMP SENSOR BLINKY THREAD DID NOT PASS XXXXXXXXX");
    }


    return 0;
}

