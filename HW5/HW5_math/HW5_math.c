#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

#define ITERATIONS 1000
#define CLOCK_HZ 150000000         // 150 MHz clock
#define NS_PER_CYCLE 6.667f        // Each clock cycle is 6.667 nanoseconds

int main() {
    stdio_init_all();

    // Wait for USB to connect
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("USB connected, starting math operations...\n");

    // Use volatile to prevent compiler from optimizing away these variables
    volatile float a = 23.33f;
    volatile float b = 45.88f;
    volatile float result = 0.0f;

    absolute_time_t start;
    uint64_t time_us;
    uint64_t cycles;

    // Addition
    start = get_absolute_time();
    for (int i = 0; i < ITERATIONS; i++) {
        result = a + b;
    }
    if (result == 123456.0f) printf("Prevent optimization\n");
    time_us = absolute_time_diff_us(start, get_absolute_time());
    cycles = (uint64_t)((time_us * 1000.0f) / NS_PER_CYCLE);
    printf("Addition: %llu cycles total, about %llu per loop\n", cycles, cycles / ITERATIONS);

    // Subtraction
    start = get_absolute_time();
    for (int i = 0; i < ITERATIONS; i++) {
        result = a - b;
    }
    if (result == 123456.0f) printf("Prevent optimization\n");
    time_us = absolute_time_diff_us(start, get_absolute_time());
    cycles = (uint64_t)((time_us * 1000.0f) / NS_PER_CYCLE);
    printf("Subtraction: %llu cycles total, about %llu per loop\n", cycles, cycles / ITERATIONS);

    // Multiplication
    start = get_absolute_time();
    for (int i = 0; i < ITERATIONS; i++) {
        result = a * b;
    }
    if (result == 123456.0f) printf("Prevent optimization\n");
    time_us = absolute_time_diff_us(start, get_absolute_time());
    cycles = (uint64_t)((time_us * 1000.0f) / NS_PER_CYCLE);
    printf("Multiplication: %llu cycles total, about %llu per loop\n", cycles, cycles / ITERATIONS);

    // Division
    start = get_absolute_time();
    for (int i = 0; i < ITERATIONS; i++) {
        result = a / b;
    }
    if (result == 123456.0f) printf("Prevent optimization\n");
    time_us = absolute_time_diff_us(start, get_absolute_time());
    cycles = (uint64_t)((time_us * 1000.0f) / NS_PER_CYCLE);
    printf("Division: %llu cycles total, about %llu per loop\n", cycles, cycles / ITERATIONS);

    return 0;
}
