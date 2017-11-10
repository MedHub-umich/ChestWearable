// tempInterface.c
#include "tempInterface.h"
#include "FreeRTOS.h"
#include "task.h"
#include "boards.h"
#include "nrf_log.h"

#define TASK_DELAY        400           /**< Task delay. Delays a LED0 task for 200 ms */
TaskHandle_t  taskBlinkyHandle;   /**< Reference to LED0 toggling FreeRTOS task. */

/**@taskBlinky
 *
 * Blinks an LED
 *
 */
void taskBlinky (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    while (true)
    {
        // Blink Red LED
        bsp_board_led_invert(BSP_BOARD_LED_1);

        // Delay (messy period)
        vTaskDelay(TASK_DELAY);
    }
}


int blinkyInit(void)
{
    // initialize ADC if not already initialized

    // create FreeRtos tasks
    BaseType_t retVal = xTaskCreate(taskBlinky, "LED0", configMINIMAL_STACK_SIZE + 60, NULL, 2, &taskBlinkyHandle);

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