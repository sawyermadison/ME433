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
#define DAC_B 1

#define WAVEFORM_LENGTH 1000
#define PI 3.141592653589793238

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

void write_DAC(uint8_t channel, uint16_t value) {

    uint16_t word = 0;

    word |= (channel << 15);
    word |= (1 << 14);
    word |= (1 << 13);
    word |= (1 << 12);
    word |= (value << 2);

    uint8_t data[2];
    data[0] = (word >> 8) & 0xFF;
    data[1] = word & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}


uint16_t sine_result(float amp, float freq, float vshift, float time){
    uint16_t sval = (amp * 1023/3.3)*sin(2*PI*freq*time - PI/2) + (vshift * 1023/3.3);
    return sval;
}

uint16_t triangle_result(float amp, float freq, float vshift, float time){
    uint16_t tval = (amp * 1023/3.3)*(2.0 * fabs(2.0*(freq*time - floor(freq*time + 0.5))) - 1.0) + (vshift* 1023/3.3);
    return tval;
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi
    
    uint32_t t0 = to_ms_since_boot(get_absolute_time());

    while (true) {
        float current_time = (to_ms_since_boot(get_absolute_time()) - t0)/1000.0;

        write_DAC(DAC_A, sine_result(3.3/2, 2, 3.3/2, current_time));
        write_DAC(DAC_B, triangle_result(3.3/2, 1, 3.3/2, current_time));
    }
}
