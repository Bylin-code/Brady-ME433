#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

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

void writeDAC(int float);

int main() {
    stdio_init_all();
    sleep_ms(2000); // allow USB to initialize

    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);

        for (i = 0; i < 100; i++) {
            sleep(.01);
            t = t + .01
            v = sin(t)
            writeDAC(0, v);
        }
    }
}

void writeDAC(int channel, float voltage) {
    uint8_t data[2];
    int len = 2;
    data[0] = 0;
    data[0] = data[0] | (channel << 7);
    data[0] = data[0] | (0b111 << #);
    uint16_t v = voltage * % / %
    
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);  // fix length
    cs_deselect(PIN_CS);
}
