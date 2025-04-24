#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "font.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 12
#define I2C_SCL 13
#define LED_BUILTIN 25



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

    gpio_init(LED_BUILTIN);
    gpio_set_dir(LED_BUILTIN, GPIO_OUT);

    ssd1306_setup();

    adc_init();           // Initialize ADC hardware
    adc_gpio_init(26);    // Initialize GPIO26 for ADC use
    adc_select_input(0);

    char voltage_str[32];
    char fps_str[32];  // make sure it's large enough for "ADC: 4095" + null terminator

    while (true) {
        uint16_t adc_value = adc_read();
        float voltage = adc_value * 3.3f / 4095.0f;
        uint16_t time = to_us_since_boot(get_absolute_time());  
        uint16_t fps_value = 1000000 / time; // Calculate FPS based on time since last frame

        ssd1306_clear();

        sprintf(voltage_str, "V: %.2f", voltage);
        drawString(0, 0, voltage_str);

        sprintf(fps_str, "FPS: %u", fps_value);
        drawString(0, 25, fps_str);

        ssd1306_update();

        sleep_ms(200);
    }
}







