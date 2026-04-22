#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "math.h"

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

#define PI 3.14159265358979323846

#define ADDR 0x68

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



void write(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(I2C_PORT, ADDR, buf, 2, false);
}

uint8_t read(uint8_t reg) {
    uint8_t buf;
    i2c_write_blocking(I2C_PORT, ADDR, &reg, 1, true);  // true to keep host control of bus
    i2c_read_blocking(I2C_PORT, ADDR, &buf, 1, false);  // false - finished with bus
    
    return buf;
}


void ssd1306_drawLine(unsigned char x, unsigned char y, float theta, unsigned char length){
    int i = 0;
    while (i<length){
        ssd1306_drawPixel(x + (float)i * cos(theta), y + (float)i * sin(theta), 1);
        i++;
    }
}

void init_6050(){
    write(PWR_MGMT_1, 0x00);
    write(ACCEL_CONFIG, 0b00000000);
    write(GYRO_CONFIG, 0b00000000);
}

void read_6050(int16_t combined_data[7]) {
    uint8_t all_data[14];
    uint8_t start_reg = 0x3B;

    i2c_write_blocking(I2C_PORT, ADDR, &start_reg, 1, true);
    i2c_read_blocking(I2C_PORT, ADDR, all_data, 14, false);

    for (int j = 0; j < 7; j++) {
        combined_data[j] = (int16_t)((all_data[j*2] << 8) | all_data[j*2 + 1]);
    }
}

int main()
{
    stdio_init_all();

    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    init_6050();

    ssd1306_setup();
    ssd1306_clear();

    gpio_init(17);
    gpio_set_dir(17, GPIO_OUT);

    while (true) {
        uint8_t val = read(WHO_AM_I);
        //printf("%x",val);

        while (val != 0x68) {
            printf("Not connected\n");
            gpio_put(17, 1);
            sleep_ms(100);
            gpio_put(17, 0);
            sleep_ms(100);

            val = read(WHO_AM_I); // retry
        }

        int16_t data[7];
        read_6050(data);

        float x_acc = data[0] * 0.000061;
        float y_acc = data[1] * 0.000061;
        float z_acc = data[2] * 0.000061;
        float temp = data[3] / 340.00 + 36.53;
        float x_gyro = data[4] * 0.007630;
        float y_gyro = data[5] * 0.007630;
        float z_gyro = data[6] * 0.007630;

        printf("Accelerations: X: %.3f g, Y: %.3f g, Z: %.3f g \t Temperature: %.3f degrees C \t Gyros: X: %.3f, Y: %.3f, Z: %.3f\n", x_acc, y_acc, z_acc, temp, x_gyro, y_gyro, z_gyro);
        sleep_ms(10);

        ssd1306_clear();
        ssd1306_drawLine(64, 16, -atan2(y_acc, x_acc) + PI, 15*(sqrt(pow(x_acc,2) + pow(y_acc,2))));
        ssd1306_update();
    }
}
