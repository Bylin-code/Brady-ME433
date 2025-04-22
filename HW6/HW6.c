#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 12
#define I2C_SCL 13
#define MY_ADDR 0x20  // 7-bit address (A2:A0 = GND)

// IO direction address and address for GPIO
#define IODIR  0x00
#define GPIO   0x09
#define OLAT   0x0A

void chip_write(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = { reg, value };
    i2c_write_blocking(I2C_PORT, MY_ADDR, buffer, 2, false);
}

uint8_t chip_read(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(I2C_PORT, MY_ADDR, &reg, 1, true);   // Write register address
    i2c_read_blocking(I2C_PORT, MY_ADDR, &value, 1, false); // Read data
    return value;
}

int main() {
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    sleep_ms(500);  

    // GP1 = input, GP0 = output
    chip_write(IODIR, 0b11111110);  // Bit 1 = 1 (GP1 input), Bit 0 = 0 (GP0 output)

    while (true) {
        uint8_t gpio_val = chip_read(GPIO);
        bool btn_pressed = (gpio_val & (1 << 1));  // GP1 is bit 1
        if (btn_pressed) {
            chip_write(GPIO, 0b00000001);  // Set GP0 high
        }
        else {
            chip_write(OLAT, 0b00000000);  // Set GP0 low
        }

        sleep_ms(10);
    }
}
