/* ----------------------------------------------------------------------
 * Copyright (C) 2010-2012 ARM Limited. All rights reserved.
 *
* $Date:         17. January 2013
* $Revision:     V1.4.0
*
* Project:       CMSIS DSP Library
 * Title:        arm_fir_example_f32.c
 *
 * Description:  Example code demonstrating how an FIR filter can be used
 *               as a low pass filter.
 *
 * Target Processor: Cortex-M4/Cortex-M3
 *
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
 * -------------------------------------------------------------------- */
/* ----------------------------------------------------------------------
** Include Files
** ------------------------------------------------------------------- */
#include "app_util_platform.h"
#include "arm_const_structs.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_delay.h"
#include "boards.h"

/* ----------------------------------------------------------------------
** Macro Defines
** ------------------------------------------------------------------- */
#define TEST_LENGTH_SAMPLES  150
#define BLOCK_SIZE            32
#define NUM_TAPS              29

static float32_t testInput_f32_1kHz_15kHzFIR[TEST_LENGTH_SAMPLES];
static float32_t testOutputFIR[TEST_LENGTH_SAMPLES];

static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];

/* ----------------------------------------------------------------------
** FIR Coefficients buffer generated using fir1() MATLAB function.
** fir1(28, 6/24)
** ------------------------------------------------------------------- */
const float32_t firCoeffs32[NUM_TAPS] = {
  -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
  -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
  +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
  +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f
};
/* ------------------------------------------------------------------
 * Global variables for FIR LPF Example
 * ------------------------------------------------------------------- */
uint32_t blockSize = BLOCK_SIZE;
uint32_t numBlocks = TEST_LENGTH_SAMPLES/BLOCK_SIZE;
//float32_t  snr;
float32_t sine_freq = 10000.f;
float32_t sampling_freq = 48000.f;

void createInput()
{

}

/* ----------------------------------------------------------------------
 * FIR LPF Example
 * ------------------------------------------------------------------- */
int main(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("***************** STARTING ********************");
    NRF_LOG_PROCESS();

    createInput();

    int j = 0;
    for (j = 0; j < TEST_LENGTH_SAMPLES; j++)
    {
      testInput_f32_1kHz_15kHzFIR[j] = 5.f*sin(sine_freq * (2.f * 3.1415) * (float32_t)j / sampling_freq);
      NRF_LOG_INFO("Input: " NRF_LOG_FLOAT_MARKER "\r", NRF_LOG_FLOAT(testInput_f32_1kHz_15kHzFIR[j]));
      NRF_LOG_PROCESS();
      //NRF_LOG_INFO("%d",j); // Print index
      //NRF_LOG_PROCESS();
      nrf_delay_ms(20);
    }

    uint32_t i;
    arm_fir_instance_f32 S;
    //arm_status status;
    float32_t  *inputF32, *outputF32;
    /* Initialize input and output buffer pointers */
    inputF32 = &testInput_f32_1kHz_15kHzFIR[0];
    outputF32 = &testOutputFIR[0];
  /* Call FIR init function to initialize the instance structure. */
  arm_fir_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);
  /* ----------------------------------------------------------------------
  ** Call the FIR process function for every blockSize samples
  ** ------------------------------------------------------------------- */
  for(i=0; i < numBlocks; i++)
  {
    arm_fir_f32(&S, inputF32 + (i * blockSize), outputF32 + (i * blockSize), blockSize);
  }


  for(i = 0; i < TEST_LENGTH_SAMPLES; i++)
  {
    NRF_LOG_INFO("Output: " NRF_LOG_FLOAT_MARKER "\r", NRF_LOG_FLOAT(outputF32[i]));
    NRF_LOG_PROCESS();
    //NRF_LOG_INFO("%d",i); //Print Index
    //NRF_LOG_PROCESS();
    nrf_delay_ms(1);
  }
  //NRF_LOG_FLUSH();

  while (1)
  {
    bsp_board_led_invert(2);
    nrf_delay_ms(100);
  }                             /* main function does not return */
}