#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>


#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   20
#define PIN_SCK  18
#define PIN_MOSI 19

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

void writeDAC(int channel, float voltage);
void sineWave(float frequency);
void triangleWave(float frequency);

int main() {
    stdio_init_all();
    sleep_ms(2000);

    spi_init(SPI_PORT, 1000 * 1000); // 1 MHz
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    float t = 0;
    
    while (true) {
        triangleWave(1);
        // sineWave(2);
    }
    
}


void writeDAC(int channel, float voltage) {
    uint16_t value = (uint16_t)((voltage / 3.3f) * 1023);  // 10-bit resolution

    uint16_t command = 0;
    command |= (channel & 0x01) << 15;  // A/B Select: 0 for A, 1 for B
    command |= 1 << 14;                // Buffer bit: 1 = buffered
    command |= 1 << 13;                // Gain: 1 = 1x (3.3V range)
    command |= 1 << 12;                // Shutdown: 1 = active
    command |= (value & 0x3FF) << 2;   // 10-bit data in bits 11:2

    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

void sineWave(float frequency) {
    float t = 0;
    while (true) {
        for (int i = 0; i < 100; i++) {
            sleep_ms(1); // 100Hz
            t += 0.001f;
            float v = (sinf(2 * M_PI * frequency * t) + 1) * (3.3f / 2);  // Sine wave
            writeDAC(0, v);
        }
    }
}

void triangleWave(float frequency) {
    float t = 0;
    while (true) {
        for (int i = 0; i < 100; i++) {
            sleep_ms(10); // 100Hz update rate
            float t = i / 100.0f; // t goes from 0.0 to 0.99 seconds
    
            // Triangle wave formula: ranges from 0 to 3.3V over 1 second
            float v;
            if (t < 0.5f) {
                v = (t / 0.5f) * 3.3f; // rising edge: 0V to 3.3V
            } else {
                v = ((1.0f - t) / 0.5f) * 3.3f; // falling edge: 3.3V to 0V
            }
    
            writeDAC(0, v);
        }
    }
}