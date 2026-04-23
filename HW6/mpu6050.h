#ifndef MPU6050_H_
#define MPU6050_H_

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

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define ADDR 0x68

void write(uint8_t reg, uint8_t val);
uint8_t read(uint8_t reg);
void init_6050();
void read_6050(int16_t raw[7]);
void scale_6050(int16_t raw[7], float scaled[7]);

#endif /* MPU6050_H_ */