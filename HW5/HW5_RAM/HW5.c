#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS_DAC   20
#define PIN_CS_RAM   21
#define PIN_SCK  18
#define PIN_MOSI 19


void init_ram();
void write_ram(uint16_t address, float data);
float read_ram(uint16_t address);
void write_dac(int channel, float voltage);
void init_sine();
static inline void cs_select(uint cs_pin);
static inline void cs_deselect(uint cs_pin);

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 250 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_DAC,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_CS_RAM,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);
    gpio_put(PIN_CS_RAM, 1);

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    // Initialize RAM (if needed)
    init_ram();
    init_sine();

    // infinite loop
    while (true) {
        // We want this loop to run at 1 Hz → one full loop every 1 second (1000 ms)
        // There are 256 iterations (i = 0 to 255)
        // So, each iteration must take: 1000 ms / 256 ≈ 3.90625 ms
        // We'll use sleep_us(3906) to get close to this with microsecond precision:
        // 3906 µs * 256 ≈ 999,936 µs ≈ ~1.000 seconds total
    
        for (uint16_t i = 0; i < 256; i++) {
            float value = read_ram(i * 4);  // Each float is 4 bytes, so step by 4
            printf("Value at address %d: %f\n", i * 4, value);
            write_dac(0, value);
            sleep_us(3906);  // Delay to match 1 Hz total loop rate
        }
    }
    return 0; // Ensure main() returns an integer value
}

// init_ram function
// Sets the RAM chip to sequential operation. To set the mode, send the instruction 0b00000001 (0x01) 
// followed by 0b01000000  for sequential operation.
void init_ram() {
    cs_select(PIN_CS_RAM);

    uint8_t cmd[2] = {0b00000001, 0b01000000}; // Write Mode Register = 0x40 (Sequential Mode)
    spi_write_blocking(SPI_PORT, cmd, 2);

    cs_deselect(PIN_CS_RAM);
    printf("RAM initialized for sequential operation.\n");
}

void init_sine() {
    // Initialize a sine wave in RAM
    for (uint16_t i = 0; i < 256; i++) {
        float value = 1.65f * (1.0f + sinf((2.0f * M_PI * i) / 256.0f)); // Scale to 0-3.3V
        write_ram(i * 4, value); // Write each value to RAM
        sleep_ms(10); // Small delay to allow writing
        printf("Wrote value %f to RAM address %d\n", value, i); // Debug output
    }
}


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

// Function: write_ram
// Writes a float value to a specific address in the RAM chip over SPI
// The address is a 16-bit value that signifies a memory loaction in the RAM, and the data is a float that will be converted to a 16-bit representation
void write_ram(uint16_t address, float data) {
    if (address <= 0x7FFC) { // Leave room for 4 bytes (0x7FFF - 3)
        uint8_t tx_buf[7];
        tx_buf[0] = 0x02;                          // WRITE command
        tx_buf[1] = (address >> 8) & 0xFF;         // Address MSB
        tx_buf[2] = address & 0xFF;                // Address LSB

        // Interpret float as 4 raw bytes
        uint8_t* float_bytes = (uint8_t*)&data;
        tx_buf[3] = float_bytes[0]; // Least significant byte
        tx_buf[4] = float_bytes[1];
        tx_buf[5] = float_bytes[2];
        tx_buf[6] = float_bytes[3]; // Most significant byte

        cs_select(PIN_CS_RAM);
        spi_write_blocking(SPI_PORT, tx_buf, 7);   // 1 cmd + 2 addr + 4 data
        cs_deselect(PIN_CS_RAM);
    }
}





float read_ram(uint16_t address) {
    if (address <= 0x7FFC) {  // Leave room for 4 bytes
        uint8_t tx_buf[3];
        tx_buf[0] = 0x03;                        // READ command
        tx_buf[1] = (address >> 8) & 0xFF;
        tx_buf[2] = address & 0xFF;

        uint8_t rx_buf[4] = {0};

        cs_select(PIN_CS_RAM);
        spi_write_blocking(SPI_PORT, tx_buf, 3);      // Send command + address
        spi_read_blocking(SPI_PORT, 0, rx_buf, 4);    // Read 4 bytes (float)
        cs_deselect(PIN_CS_RAM);

        // Reconstruct float from 4 bytes
        float result;
        uint8_t* float_bytes = (uint8_t*)&result;
        float_bytes[0] = rx_buf[0];
        float_bytes[1] = rx_buf[1];
        float_bytes[2] = rx_buf[2];
        float_bytes[3] = rx_buf[3];

        return result;
    }
    return 0.0f;
}






// Function: write_dac
// Sends a voltage value to the DAC over SPI, using the MCP4912 command format
void write_dac(int channel, float voltage) {
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
    cs_select(PIN_CS_DAC);                  // Begin SPI transmission
    spi_write_blocking(SPI_PORT, data, 2); // Send 2 bytes
    cs_deselect(PIN_CS_DAC);                // End SPI transmission
}