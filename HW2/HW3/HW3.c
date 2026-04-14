#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

#define IODIR 0x00
#define GPIO 0x09

#define ADDR 0b0100000 // A0, A1, A2 all tied to GND

typedef struct {
    uint8_t gp0 : 1;
    uint8_t gp1 : 1;
    uint8_t gp2 : 1;
    uint8_t gp3 : 1;
    uint8_t gp4 : 1;
    uint8_t gp5 : 1;
    uint8_t gp6 : 1;
    uint8_t gp7 : 1;
} GPIOBits;

void write(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(I2C_PORT, ADDR, buf, 2, false);
}

void initMCP() {
    write(IODIR, 0b01111111); // Set GP7 as output, the rest as inputs
}

uint8_t read(uint8_t reg) {
    uint8_t buf;
    i2c_write_blocking(I2C_PORT, ADDR, &reg, 1, true);  // true to keep host control of bus
    i2c_read_blocking(I2C_PORT, ADDR, &buf, 1, false);  // false - finished with bus
    
    return buf;
}

GPIOBits unpack_bits(uint8_t val) {
    GPIOBits bits;
    bits.gp0 = (val >> 0) & 0x01;
    bits.gp1 = (val >> 1) & 0x01;
    bits.gp2 = (val >> 2) & 0x01;
    bits.gp3 = (val >> 3) & 0x01;
    bits.gp4 = (val >> 4) & 0x01;
    bits.gp5 = (val >> 5) & 0x01;
    bits.gp6 = (val >> 6) & 0x01;
    bits.gp7 = (val >> 7) & 0x01;
    return bits;
}

void heartbeat() {
    gpio_put(25, 1);
    sleep_ms(50);
    gpio_put(25, 0);
    sleep_ms(50);
}

int main()
{
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    initMCP();

    while (true) {
        heartbeat();

        uint8_t pins = read(GPIO);
        //printf("GPIO state: 0b%08b\n", pins);

        if (unpack_bits(pins).gp0) {
            printf("GP0 is high\n");
            write(GPIO, 0b10000000); // Set GP7 high, the rest low
        } else {
            printf("GP0 is low\n");
            write(GPIO, 0b00000000); // Set all pins low
        }
    }
}
