#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "ws2812.pio.h"
#include <math.h>

#define NUM_PIXELS 4
#define IS_RGBW false
#define WS2812_PIN 17
#define SERVO_PIN 19
#define CYCLE_TIME_MS 5000 // 5 seconds

static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | b;
}

// Convert hue (0-360) to RGB
void hsv_to_rgb(float h, uint8_t *r, uint8_t *g, uint8_t *b) {
    float s = 1.0f, v = 1.0f;
    int i = (int)(h / 60.0f) % 6;
    float f = h / 60.0f - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    float rf, gf, bf;
    switch (i) {
        case 0: rf = v; gf = t; bf = p; break;
        case 1: rf = q; gf = v; bf = p; break;
        case 2: rf = p; gf = v; bf = t; break;
        case 3: rf = p; gf = q; bf = v; break;
        case 4: rf = t; gf = p; bf = v; break;
        case 5: rf = v; gf = p; bf = q; break;
    }

    *r = (uint8_t)(rf * 255);
    *g = (uint8_t)(gf * 255);
    *b = (uint8_t)(bf * 255);
}

void show_rainbow(PIO pio, uint sm, float base_hue) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        float hue = fmodf(base_hue + (360.0f / NUM_PIXELS) * i, 360.0f);
        uint8_t r, g, b;
        hsv_to_rgb(hue, &r, &g, &b);
        put_pixel(pio, sm, urgb_u32(r, g, b));
    }
    sleep_ms(1); // reset latch
}

void setup_servo_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 64.0f);  // ~305Hz
    pwm_config_set_wrap(&config, 39062);    // 20ms period (1.25MHz / 64 / 39062 ≈ 20ms)
    pwm_init(slice, &config, true);
}

void set_servo_angle(uint pin, float angle) {
    uint slice = pwm_gpio_to_slice_num(pin);
    // Map 0–180° to 1ms–2ms pulse width
    float pulse_ms = 1.0f + angle / 180.0f; // 1ms to 2ms
    uint level = (pulse_ms / 20.0f) * 39062;
    pwm_set_gpio_level(pin, (uint16_t)level);
}

int main() {
    stdio_init_all();

    // WS2812 setup
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Servo setup
    setup_servo_pwm(SERVO_PIN);

    absolute_time_t start = get_absolute_time();

    while (true) {
        int elapsed = to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(start);
        int t = elapsed % CYCLE_TIME_MS;

        // Normalize to 0.0–1.0
        float phase = (float)t / CYCLE_TIME_MS;

        // Base hue from 0–360 over the cycle
        float base_hue = phase * 360.0f;
        show_rainbow(pio, sm, base_hue);

        // Servo angle from 0–180 over 5 seconds
        float angle = phase * 360.0f;
        set_servo_angle(SERVO_PIN, angle);

        sleep_ms(20);
    }
}
