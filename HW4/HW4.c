



// Include standard input/output functions (for USB serial output like printf)
#include <stdio.h>

// Include Pico SDK functions for GPIO and timing
#include "pico/stdlib.h"

// Include hardware-specific SPI functions
#include "hardware/spi.h"

// Include math functions (like sin, M_PI, etc.)
#include <math.h>

// ----------------------------------------
// SPI pin definitions for the Raspberry Pi Pico
// These are connected to the DAC (MCP4912)
#define SPI_PORT spi0     // Use SPI0 peripheral
#define PIN_MISO 16       // Not used (DAC doesn't send data back)
#define PIN_CS   20       // Chip Select (can be any GPIO)
#define PIN_SCK  18       // Serial Clock pin (SPI clock)
#define PIN_MOSI 19       // Master Out, Slave In (sends data to DAC)

// ----------------------------------------
// Function: cs_select
// Pulls the CS (chip select) line LOW to begin SPI communication
static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // Tiny delays before pulling CS low
    gpio_put(cs_pin, 0);               // Set CS LOW to select the DAC
    asm volatile("nop \n nop \n nop"); // Tiny delays after pulling CS low
}

// Function: cs_deselect
// Pulls the CS (chip select) line HIGH to end SPI communication
static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // Tiny delays before pulling CS high
    gpio_put(cs_pin, 1);               // Set CS HIGH to deselect the DAC
    asm volatile("nop \n nop \n nop"); // Tiny delays after pulling CS high
}

// ----------------------------------------
// Declare functions used later in the code
void writeDAC(int channel, float voltage);
void sineWave(float frequency);
void triangleWave(float frequency);

// ----------------------------------------
// Function: main
// This is the entry point of the program. It initializes SPI and runs the waveform generator.
int main() {
    stdio_init_all(); // Initializes USB serial for debug printing
    sleep_ms(2000);   // Wait 2 seconds to ensure USB is ready

    // Initialize SPI0 at 1 MHz baud rate (speed of communication)
    spi_init(SPI_PORT, 1000 * 1000);

    // Set up SPI pin functions
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI); // Unused, but required for SPI setup
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO); // CS is a normal GPIO output
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI); // SPI Clock pin
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI); // SPI Data Out

    // Set CS as output and initialize it HIGH (inactive)
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // Variable used for time tracking (not used here, but left for future use)
    float t = 0;

    while (true) {
        triangleWave(1); // Generate a 1Hz triangle wave continuously on DAC channel A
        // sineWave(2);  // Uncomment to generate a 2Hz sine wave instead
    }
}

// ----------------------------------------
// Function: writeDAC
// Sends a voltage value to the DAC over SPI, using the MCP4912 command format
void writeDAC(int channel, float voltage) {
    // Convert voltage (0 to 3.3V) into a 10-bit number (0 to 1023)
    uint16_t value = (uint16_t)((voltage / 3.3f) * 1023);

    // Build the 16-bit command to send to the DAC
    uint16_t command = 0;
    command |= (channel & 0x01) << 15;   // Bit 15: DAC channel select (0=A, 1=B)
    command |= 1 << 14;                  // Bit 14: Buffer bit (1 = buffered)
    command |= 1 << 13;                  // Bit 13: Gain (1 = 1x gain, full 0–3.3V range)
    command |= 1 << 12;                  // Bit 12: Shutdown control (1 = active)
    command |= (value & 0x3FF) << 2;     // Bits 11–2: 10-bit data left-aligned

    // Split the 16-bit command into two bytes to send over SPI
    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;     // High byte
    data[1] = command & 0xFF;            // Low byte

    // Send the data to the DAC over SPI
    cs_select(PIN_CS);                  // Begin SPI transmission
    spi_write_blocking(SPI_PORT, data, 2); // Send 2 bytes
    cs_deselect(PIN_CS);                // End SPI transmission
}

// ----------------------------------------
// Function: sineWave
// Generates a sine wave at a given frequency on DAC channel A
void sineWave(float frequency) {
    float t = 0; // Time variable in seconds

    while (true) {
        for (int i = 0; i < 100; i++) {
            sleep_ms(1); // 100 updates per second (100Hz update rate)
            t += 0.001f; // Advance time by 1/100th of a second

            // Calculate sine wave value using sin(2πft)
            // Range is -1 to 1 → convert to 0 to 3.3V
            float v = (sinf(2 * M_PI * frequency * t) + 1) * (3.3f / 2);

            // Output the voltage to DAC channel 0 (channel A)
            writeDAC(0, v);
        }
    }
}

// ----------------------------------------
// Function: triangleWave
// Generates a triangle wave at a given frequency on DAC channel A
void triangleWave(float frequency) {
    while (true) {
        for (int i = 0; i < 100; i++) {
            sleep_ms(10); // 100Hz update rate (10ms between updates)

            // Convert loop index to time in seconds (0.0 to ~1.0s)
            float t = i / 100.0f;

            float v;
            if (t < 0.5f) {
                // Rising edge of triangle (0V to 3.3V)
                v = (t / 0.5f) * 3.3f;
            } else {
                // Falling edge of triangle (3.3V to 0V)
                v = ((1.0f - t) / 0.5f) * 3.3f;
            }

            // Output the voltage to DAC channel 0 (channel A)
            writeDAC(0, v);
        }
    }
}
