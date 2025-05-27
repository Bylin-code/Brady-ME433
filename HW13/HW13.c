#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

// SSD1306 OLED display defines
#define SSD1306_ADDR 0x3D  // Try 0x3D instead of 0x3C
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_PAGE_HEIGHT 8 // 8 pixels per page
#define SSD1306_PAGES (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)

// SSD1306 commands
#define SSD1306_COMMAND 0x00
#define SSD1306_DATA 0x40
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_PAGEADDR 0x22
#define SSD1306_COLUMNADDR 0x21

// Buffer for display data
uint8_t display_buffer[SSD1306_WIDTH * SSD1306_PAGES];

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

// SSD1306 Display Functions
void ssd1306_command(uint8_t cmd) {
    uint8_t buf[2] = {SSD1306_COMMAND, cmd};
    i2c_write_blocking(I2C_PORT, SSD1306_ADDR, buf, 2, false);
}

void ssd1306_init() {
    sleep_ms(100);  // Wait for display to power up
    
    uint8_t cmds[] = {
        SSD1306_DISPLAYOFF,             // 0xAE
        SSD1306_SETDISPLAYCLOCKDIV,     // 0xD5
        0x80,                           // Suggested ratio
        SSD1306_SETMULTIPLEX,           // 0xA8
        0x3F,                           // 0x3F for 128x64, 0x1F for 128x32
        SSD1306_SETDISPLAYOFFSET,        // 0xD3
        0x00,                           // No offset
        SSD1306_SETSTARTLINE | 0x00,    // 0x40 | start line
        SSD1306_CHARGEPUMP,             // 0x8D
        0x14,                           // Enable charge pump
        SSD1306_MEMORYMODE,             // 0x20
        0x00,                           // Act like ks0108 (horizontal addressing)
        SSD1306_SEGREMAP | 0x01,        // 0xA0 | bit 0 (flip horizontally)
        SSD1306_COMSCANDEC,             // 0xC8 (flip vertically)
        SSD1306_SETCOMPINS,             // 0xDA
        0x12,                           // 0x12 for 128x64, 0x02 for 128x32
        SSD1306_SETCONTRAST,            // 0x81
        0x8F,                           // Medium contrast (0x8F)
        SSD1306_SETPRECHARGE,           // 0xD9
        0xF1,                           // 0xF1 for external VCC, 0x22 for internal
        SSD1306_SETVCOMDETECT,          // 0xDB
        0x40,                           // 0x40
        SSD1306_DISPLAYALLON_RESUME,    // 0xA4
        SSD1306_NORMALDISPLAY,          // 0xA6
        SSD1306_DISPLAYON               // 0xAF
    };
    
    // Send each command individually with delay
    for (int i = 0; i < sizeof(cmds); i++) {
        ssd1306_command(cmds[i]);
        sleep_ms(1);  // Small delay between commands
    }
    
    printf("SSD1306 initialization complete\n");
}

void ssd1306_clear_buffer() {
    memset(display_buffer, 0, sizeof(display_buffer));
}

void ssd1306_display() {
    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(0);  // Page start address (0 = bottom)
    ssd1306_command(SSD1306_PAGES - 1);  // Page end address
    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(0);  // Column start address
    ssd1306_command(SSD1306_WIDTH - 1);  // Column end address

    // Send the buffer in 16-byte chunks to avoid I2C buffer size limitations
    for (int i = 0; i < SSD1306_WIDTH * SSD1306_PAGES; i += 16) {
        uint8_t buf[17];  // 1 byte for data/command, 16 for data
        buf[0] = SSD1306_DATA;
        for (int j = 0; j < 16 && (i + j) < (SSD1306_WIDTH * SSD1306_PAGES); j++) {
            buf[j + 1] = display_buffer[i + j];
        }
        i2c_write_blocking(I2C_PORT, SSD1306_ADDR, buf, 17, false);
    }
}

// Drawing functions
void set_pixel(int x, int y, bool on) {
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) {
        return;  // Out of bounds
    }

    // Calculate the byte position in the buffer
    int byte_idx = x + (y / 8) * SSD1306_WIDTH;
    uint8_t bit_position = y % 8;

    if (on) {
        display_buffer[byte_idx] |= (1 << bit_position);  // Set bit
    } else {
        display_buffer[byte_idx] &= ~(1 << bit_position);  // Clear bit
    }
}

// Bresenham's line algorithm for drawing lines
void draw_line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true) {
        set_pixel(x0, y0, true);
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

int main() {
    stdio_init_all();

    // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Initialize MPU6050
    mpu6050_init();
    uint8_t whoami = read_whoami();
    printf("WHO_AM_I = 0x%02X\n", whoami);  // Should print 0x68

    // Scan I2C bus to find connected devices
    printf("I2C bus scan:\n");
    for (uint8_t addr = 0; addr < 128; addr++) {
        uint8_t rxdata;
        int ret = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false);
        if (ret >= 0) {
            printf("  Found device at address 0x%02X\n", addr);
        }
    }
    sleep_ms(100);

    // Initialize SSD1306 OLED
    printf("Initializing SSD1306 at address 0x%02X\n", SSD1306_ADDR);
    ssd1306_init();
    printf("Clearing buffer...\n");
    ssd1306_clear_buffer();
    printf("Sending to display...\n");
    ssd1306_display();
    sleep_ms(100);

    // Center point of the display
    int center_x = SSD1306_WIDTH / 2;
    int center_y = SSD1306_HEIGHT / 2;
    
    // Scaling factor for accelerometer readings
    // Adjust this based on your IMU sensitivity and desired line length
    float scale_factor = 0.01;
    
    int16_t ax, ay, az;
    int line_end_x, line_end_y;

    while (true) {
        // Read accelerometer data
        read_accel(&ax, &ay, &az);
        
        // Calculate line endpoints based on acceleration
        // Note: We invert Y to match screen coordinates
        line_end_x = center_x + (int)(ax * scale_factor);
        line_end_y = center_y - (int)(ay * scale_factor);  // Invert Y axis
        
        // Clear buffer, draw lines, and update display
        ssd1306_clear_buffer();
        
        // Draw a crosshair at center (optional)
        draw_line(center_x - 5, center_y, center_x + 5, center_y);
        draw_line(center_x, center_y - 5, center_x, center_y + 5);
        
        // Draw acceleration vector line
        draw_line(center_x, center_y, line_end_x, line_end_y);
        
        // Update display
        ssd1306_display();
        
        printf("Accel X: %d, Y: %d, Z: %d | Line: (%d,%d) to (%d,%d)\n", 
               ax, ay, az, center_x, center_y, line_end_x, line_end_y);
        
        // Shorter delay for more responsive display updates
        sleep_ms(10);
    }
}

