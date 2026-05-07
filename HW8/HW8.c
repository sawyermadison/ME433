#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19
#define DAC_A 0

#define PI 3.14159265358979323846

void init_wave(float *fwave, uint16_t *bwave){
    for (int i = 0; i<1000; i++){
        fwave[i] = 3.3 * (sin(2*PI*i/1000) + 1)/2;
        bwave[i] = (uint16_t)(1023/3.3 * fwave[i]);
    }
}

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

void set_SRAM_mode(uint8_t mode){
    cs_select(PIN_CS);
    uint8_t change_mode_cmd = 0b00000001; // change mode command
    spi_write_blocking(SPI_PORT, &change_mode_cmd, 1); // write command for sram
    spi_write_blocking(SPI_PORT, &mode, 1);
    cs_deselect(PIN_CS);
}

void write_SRAM_bytes(uint16_t address, uint8_t *data, size_t len){

    cs_select(PIN_CS);
    uint8_t write_cmd = 0b00000010; // write command
    spi_write_blocking(SPI_PORT, &write_cmd, 1);
    
    uint8_t addr_high = (address >> 8) & 0xFF;
    uint8_t addr_low = address & 0xFF;
    spi_write_blocking(SPI_PORT, &addr_high, 1);
    spi_write_blocking(SPI_PORT, &addr_low, 1);
    spi_write_blocking(SPI_PORT, data, len);

    cs_deselect(PIN_CS);
}

void sram_read(uint16_t address, uint8_t *buffer, size_t len){
    cs_select(PIN_CS);
    uint8_t read_cmd = 0b00000011; // read command
    spi_write_blocking(SPI_PORT, &read_cmd, 1);

    uint8_t addr_high = (address >> 8) & 0xFF;
    uint8_t addr_low = address & 0xFF;
    spi_write_blocking(SPI_PORT, &addr_high, 1);
    spi_write_blocking(SPI_PORT, &addr_low, 1);

    spi_read_blocking(SPI_PORT, 0, buffer, len);
    cs_deselect(PIN_CS);
}

int main()
{
    stdio_init_all();

    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    float fwave[1000];
    uint16_t bwave[1000];

    init_wave(fwave, bwave);
    set_SRAM_mode(0b01000000); // sequential mode

    for (int i = 0; i < 1000; i++){
        
        uint16_t word = 0;
        word |= (DAC_A << 15);
        word |= (1 << 14);
        word |= (1 << 13);
        word |= (1 << 12);
        word |= (bwave[i] << 2);

        uint8_t data[2];
        data[0] = (word >> 8) & 0xFF;
        data[1] = word & 0xFF;

        write_SRAM_bytes(i*2, data, 2);
    }


    while (true) {
        for (int i = 0; i < 1000; i++){
            uint8_t buffer[2];
            sram_read(i*2, buffer, 2);

            //printf("Address: %d, Data: %02x%02x\n", i*2, buffer[0], buffer[1]);

            cs_select(PIN_CS);
            spi_write_blocking(SPI_PORT, buffer, 2);
            cs_deselect(PIN_CS);

            sleep_ms(1);
        }
    }

    
}
