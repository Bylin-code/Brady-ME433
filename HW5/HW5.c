#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

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
static inline void cs_select(uint cs_pin);
static inline void cs_deselect(uint cs_pin);

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
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
    // Initialize RAM (if needed)
    init_ram();

    // infinite loop
    while (true) {
        write_ram(0, 3.14159f); // Write a float value to RAM at address 0
        sleep_ms(200);
        float ram_val = read_ram(0);
        printf("The value is: %f", ram_val);
        printf("\n");
        sleep_ms(100);
    }
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
    if (address <= 0x7FFF) {  // 32KB = 0x0000 to 0x7FFF
        uint16_t data_to_write = (uint16_t)(data * 1000);

        uint8_t tx_buf[5];
        tx_buf[0] = 0x02;                          // WRITE command
        tx_buf[1] = (address >> 8) & 0xFF;         // Address MSB
        tx_buf[2] = address & 0xFF;                // Address LSB
        tx_buf[3] = (data_to_write >> 8) & 0xFF;   // Data MSB
        tx_buf[4] = data_to_write & 0xFF;          // Data LSB

        cs_select(PIN_CS_RAM);
        spi_write_blocking(SPI_PORT, tx_buf, 5);
        cs_deselect(PIN_CS_RAM);
    }
}



float read_ram(uint16_t address) {
    if (address <= 0x7FFF) {
        uint8_t tx_buf[3];
        tx_buf[0] = 0x03;                  // READ command
        tx_buf[1] = (address >> 8) & 0xFF; // Address MSB
        tx_buf[2] = address & 0xFF;        // Address LSB

        uint8_t rx_buf[2] = {0};

        cs_select(PIN_CS_RAM);
        spi_write_blocking(SPI_PORT, tx_buf, 3);
        spi_read_blocking(SPI_PORT, 0, rx_buf, 2);
        cs_deselect(PIN_CS_RAM);

        uint16_t data_read = (rx_buf[0] << 8) | rx_buf[1];
        return (float)data_read / 1000.0f;
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