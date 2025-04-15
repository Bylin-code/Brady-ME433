#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include "hardware/adc.h"



int LED_PIN = 15;
int POT_PIN = 26;
int BTN_PIN = 2;

int main() {
    stdio_init_all();

    gpio_init(LED_PIN); // PIN_NUM without the GP
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(BTN_PIN); // PIN_NUM without the GP
    gpio_set_dir(BTN_PIN, GPIO_IN);

    adc_init(); // init the adc module
    adc_gpio_init(POT_PIN); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(POT_PIN - 26); // select to read from ADC0

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");
 
    while (1) {
        bool btn_state = gpio_get(BTN_PIN);
        gpio_put(LED_PIN, 1);
        while (btn_state == 1) {
            gpio_put(LED_PIN, 0);
            char user_input[100];
            printf("How many samples?: ");
            scanf("%s", user_input);   // Waits for input until space or newline
            int int_user_input = atoi(user_input);
            
            for (int i = 0; i < int_user_input; i++) {
                uint16_t adc_value = adc_read();
                float pot_voltage = adc_value * 3.3f / 4095.0f;  // Use floats for accurate math
                printf("ADC Voltage: %.2f V\n", pot_voltage);   // Correct format specifier
                sleep_ms(10);
            }
            btn_state = gpio_get(BTN_PIN);
        }
    }
}