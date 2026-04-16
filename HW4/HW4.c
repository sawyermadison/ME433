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
#define I2C_SDA 0
#define I2C_SCL 1


void drawChar(unsigned char x, unsigned char y, char c) {
    for (int i = 0; i < 5; i++) {
        ssd1306_buffer[1 + x + (y/8)*128] = ASCII[c-32][i];
        x++;
    }
    //ssd1306_update();
}

void drawString(unsigned char x, unsigned char y, char *str) {
    while (*str != '\0') {
        if (x + 5 >= 128) {  // wrap horizontally
            x = 0;
            y += 8;          // move down one page
        }
        if (y >= 32) {
            return;          // stop if out of screen
        }
        drawChar(x, y, *str);
        x += 6;
        str++;
    }
}


int main()
{
    stdio_init_all();

    gpio_init(17);
    gpio_set_dir(17, GPIO_OUT);
    gpio_put(17, 0);

    adc_init();

    adc_gpio_init(26);
    adc_select_input(0);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    ssd1306_setup();
    ssd1306_clear();

    uint32_t t0, t1;

    while (true) {
        ssd1306_clear();
        t0 = to_us_since_boot(get_absolute_time());
        // gpio_put(17, 1);
        // ssd1306_drawPixel(12, 12, 1);
        // ssd1306_update();
        // sleep_ms(1000);
        // gpio_put(17, 0);
        // ssd1306_clear();
        // ssd1306_update();
        // sleep_ms(1000);

        // for (int letter = 32; letter < 127; letter++) {
        //     drawChar(0, 0, letter);
        //     sleep_ms(100);
        // }

        char str[100];
        sprintf(str, "%s", "Sawyer's ME433 HW4 :)");
        drawString(0, 0, str);

        char voltage_str[25];
        float voltage;
        voltage = adc_read() * (3.3f / 4095.0f);
        sprintf(voltage_str, "Voltage: %.2f V", voltage);
        drawString(0, 16, voltage_str);

        ssd1306_update();
        
        char fps_str[25];
        t1 = to_us_since_boot(get_absolute_time()); 
        sprintf(fps_str, "FPS: %.2f", 1000000.0f / (t1 - t0));
        drawString(0, 24, fps_str);
        
        ssd1306_update();
    }
}
