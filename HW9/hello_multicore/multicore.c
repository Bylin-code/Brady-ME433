/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

const uint LED_PIN = 15;

void core1_entry() {
    while (1) {
        // Wait for a command from core0
        uint32_t c = multicore_fifo_pop_blocking();

        switch (c) {
            case 0: {
                uint16_t A0V = adc_read();
                printf("ADC value: %d\n", A0V);
                break;
            }
            case 1: {
                gpio_put(15, 1);
                printf("Pin 15 set high\n");
                break;
            }
            case 2: {
                gpio_put(15, 0);
                printf("Pin 15 set low\n");
                break;
            }
        }
    }
}

int main() {
    stdio_init_all();
    adc_init();
    adc_select_input(0);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);  // Start with LED off    

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("Hello, multicore!\n");

    /// \tag::setup_multicore[]

    multicore_launch_core1(core1_entry);

    while (1) {
        uint32_t c;
        while (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {}
        printf("Enter a character: ");
        int result = scanf("%d", &c);

        if (result == 1) {  // If we successfully read a number
            printf("\nYou entered: %u\n", c);
            multicore_fifo_push_blocking(c);
        } else {
            printf("Please enter a valid number\n");
            // Clear any remaining input
            while (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {}
        }
    }   
    /// \end::setup_multicore[]
}
