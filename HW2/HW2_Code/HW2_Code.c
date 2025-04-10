#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN 25            // Default onboard LED
#define BUTTON_PIN 2          // GPIO pin for the external button

volatile uint32_t press_count = 0;
volatile bool led_state = false;

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        press_count++;
        led_state = !led_state;
        gpio_put(LED_PIN, led_state);
        printf("Button pressed %lu times\n", press_count);
    }
}

int main() {
    stdio_init_all();                     // USB output init
    gpio_init(LED_PIN);                  // LED setup
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_PIN);              // Button setup
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);         // Pull down resistor

    gpio_set_irq_enabled_with_callback(
        BUTTON_PIN,
        GPIO_IRQ_EDGE_RISE,
        true,
        &gpio_callback
    );

    while (1) {
        tight_loop_contents(); // Keeps CPU busy while waiting for IRQ
    }
}
