/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @file
 *
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
//#include "boards.h"
#include "nrf_gpio.h"

// USE PINS FAR FROM RADIO PINS - NOTE
#define GPIO_ECG    16
#define GPIO_TEMP   27
#define GPIO_LED1    7
#define GPIO_LED2   11
#define GPIO_SPKR   15

// configures all gpio pins in this application
void main_config_gpio(void);

/**
 * @brief Function for application main entry.
 */
int main(void)
{

    /* Configure LEDs. */
    main_config_gpio();

    /* Toggle LEDs. */
    while (true)
    {
        nrf_gpio_pin_toggle(GPIO_ECG);
        nrf_gpio_pin_toggle(GPIO_TEMP);
        nrf_gpio_pin_toggle(GPIO_LED1);
        nrf_gpio_pin_toggle(GPIO_LED2);
        //nrf_gpio_pin_toggle(GPIO_SPKR);
        // for (int i = 0; i < LEDS_NUMBER; i++)
        // {
        //     bsp_board_led_invert(i);
        // }
        nrf_delay_ms(3000);
    }
}

void main_config_gpio(void)
{
    //nrf_gpio_cfg_output(GPIO_ECG);
    //nrf_gpio_pin_clear(GPIO_ECG);
    nrf_gpio_cfg(
        GPIO_ECG,
        GPIO_PIN_CNF_DIR_Output,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        GPIO_PIN_CNF_DRIVE_S0H1,
        NRF_GPIO_PIN_NOSENSE
    );
    nrf_gpio_pin_clear(GPIO_ECG);
    nrf_gpio_cfg_output(GPIO_TEMP);
    nrf_gpio_pin_clear(GPIO_TEMP);
    nrf_gpio_cfg(
        GPIO_LED1,
        GPIO_PIN_CNF_DIR_Output,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        GPIO_PIN_CNF_DRIVE_S0H1,
        NRF_GPIO_PIN_NOSENSE
    );
    nrf_gpio_pin_clear(GPIO_LED1);
    nrf_gpio_cfg(
        GPIO_LED2,
        GPIO_PIN_CNF_DIR_Output,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        GPIO_PIN_CNF_DRIVE_S0H1,
        NRF_GPIO_PIN_NOSENSE
    );
    nrf_gpio_pin_clear(GPIO_LED2);
    //nrf_gpio_cfg_output(GPIO_SPKR);
    //nrf_gpio_pin_clear(GPIO_SPKR);

    // configure LEDs on board. bsp functions may be bad for us
    // bsp_board_leds_init();
}

/**
 *@}
 **/
