#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

#define MPU6050_ADDR 0x68

// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75

void mpu6050_init() {
    // Wake up MPU6050 by writing 0x00 to PWR_MGMT_1
    uint8_t buf[] = {PWR_MGMT_1, 0x00};
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
    sleep_ms(100);  // Give time to settle
}

uint8_t read_whoami() {
    uint8_t reg = WHO_AM_I;
    uint8_t value = 0;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, &value, 1, false);
    return value;  // Should be 0x68
}

void read_accel(int16_t* ax, int16_t* ay, int16_t* az) {
    uint8_t reg = ACCEL_XOUT_H;
    uint8_t buffer[6];

    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, buffer, 6, false);

    *ax = (buffer[0] << 8) | buffer[1];
    *ay = (buffer[2] << 8) | buffer[3];
    *az = (buffer[4] << 8) | buffer[5];
}

int main() {
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    mpu6050_init();
    ssd1306_setup();

    uint8_t whoami = read_whoami();
    printf("WHO_AM_I = 0x%02X\n", whoami);  // Should print 0x68

    int16_t ax, ay, az;

    // Function to draw a single pixel
    void drawPixel(int x, int y) {
        // Make sure we don't try to draw outside the display boundaries
        if (x >= 0 && x < 128 && y >= 0 && y < 64) {
            ssd1306_drawPixel(x, y, 1);  // 1 for white/on
        }
    }

    // Bresenham line algorithm for drawing lines
    void drawLine(int x0, int y0, int x1, int y1) {
        int dx = abs(x1 - x0);
        int dy = -abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        int e2;
        
        while (true) {
            drawPixel(x0, y0);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) {
                if (x0 == x1) break;
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                if (y0 == y1) break;
                err += dx;
                y0 += sy;
            }
        }
    }

    while (true) {
        read_accel(&ax, &ay, &az);
        printf("Accel X: %d, Y: %d, Z: %d\n", ax, ay, az);

        // Clear the display
        ssd1306_clear();
        
        // Define the center of the display
        int center_x = 64;  // SSD1306 is typically 128x64, so center is at (64, 32)
        int center_y = 16;
        
        // Scale the accelerometer values to get reasonable line lengths
        // The scaling factor may need adjustment based on your specific accelerometer sensitivity
        float scale = 0.004;
        int line_end_x = center_x - (int)(ax * scale);  // Invert X to make the line point opposite to acceleration
        int line_end_y = center_y + (int)(ay * scale);  // Invert Y to make the line point opposite to acceleration
        
        // Draw a line from center to the calculated endpoint
        drawLine(center_x, center_y, line_end_x, line_end_y);
        
        // Update the display
        ssd1306_update();
        
        // Short delay for faster updates
        sleep_ms(10);
    }
}

