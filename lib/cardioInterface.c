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

void taskFIR (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    uint16_t ecgDataBufferFilteredDownSampled[SAMPLES_PER_CHANNEL/2];

    while (true)
    {
        waitForNotification(ECG_BUFFER_FULL_NOTIFICATION);

        int i;

        //bsp_board_led_on(1);
        arm_fir_f32(&S, ecgDataBuffer, ecgDataBufferFiltered, blockSize);
        //bsp_board_led_off(1);

        for(i = 0; i < SAMPLES_PER_CHANNEL; ++i)
        {
            if (i % 2 == 0)
            {
                ecgDataBufferFilteredDownSampled[i/2] = (uint16_t)ecgDataBufferFiltered[i];
                //NRF_LOG_INFO("%d", ecgDataBufferFilteredDownSampled[i/2]);
            }
        }
        int size = sizeof(ecgDataBufferFilteredDownSampled);
        pendingMessagesPush(size, (char*)ecgDataBufferFilteredDownSampled, &globalQ);
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

    // create FreeRtos tasks
    checkReturn(xTaskCreate(taskFIR, "LED0", configMINIMAL_STACK_SIZE + 60, NULL, 2, &taskFIRHandle));

    return 0;
}

