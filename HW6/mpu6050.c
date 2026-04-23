#include <stdint.h>
#include "mpu6050.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"


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

void init_6050(){
    write(PWR_MGMT_1, 0x00);
    write(ACCEL_CONFIG, 0b00000000);
    write(GYRO_CONFIG, 0b00000000);
}

void read_6050(int16_t raw[7]) {
    uint8_t all_data[14];
    uint8_t start_reg = 0x3B;

    i2c_write_blocking(I2C_PORT, ADDR, &start_reg, 1, true);
    i2c_read_blocking(I2C_PORT, ADDR, all_data, 14, false);

    for (int j = 0; j < 7; j++) {
        raw[j] = (int16_t)((all_data[j*2] << 8) | all_data[j*2 + 1]);
    }

}

void scale_6050(int16_t raw[7], float scaled[7]){
    for (int j = 0; j < 7; j++) {
        if (j<3){
            scaled[j] = raw[j] * 0.000061;
        }
        else if (j==3){
            scaled[j] = raw[j]  / 340.00 + 36.53;
        }
        else{
            scaled[j] = raw[j] * 0.007630;
        }
    }

}



// float x_acc = data[0] * 0.000061;
// float y_acc = data[1] * 0.000061;
// float z_acc = data[2] * 0.000061;
// float temp = data[3] / 340.00 + 36.53;
// float x_gyro = data[4] * 0.007630;
// float y_gyro = data[5] * 0.007630;
// float z_gyro = data[6] * 0.007630;