#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 12
#define I2C_SCL 13
// I2C address of the device
#define MY_ADDR 0b00100000

// MCP23008 Registers
// Register addr for IO direction
#define IODIR   0x00
// Register addr for GPIO 0 
#define GPIO    0x09 

int btn_state;

void chip_write(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = { reg, value };
    i2c_write_blocking(I2C_PORT, MY_ADDR, buffer, 2, false);
}

int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c
    
    // initialize the MCP23008 to set GPIO 0 as output
    chip_write(IODIR, 0b11111110);

    while (true) {

        btn_state = i2c_read_blocking(i2c_default, MY_ADDR, 0b00000010, 1, false);  // false - finished with bus

        chip_write(GPIO, 0b00000001);
        sleep_ms(1000);
        chip_write(GPIO, 0b00000000);
        sleep_ms(1000);
    }
}
