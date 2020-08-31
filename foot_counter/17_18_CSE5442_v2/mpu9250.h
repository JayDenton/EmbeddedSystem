// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

#ifndef MPU9250_H_
#define MPU9250_H_

#include <stdint.h>
#include "i2c0.h"

// MPU I2C bus is manufactured with value 0x68
#define MPU9250_ADDR 0x68
// Magnetometer I2C bus is manufactured with value 0x0C
#define AK8963_ADDR 0x0C

#define writeRegister(x,y) writeI2c0Register(MPU9250_ADDR,x,y)
#define readRegister(x) readI2c0Register(MPU9250_ADDR,x)

void init_mpu9250(void);
void init_AK8963(float* dest);
void calibrate_mag();
void read_mag(int16_t *dest);
void get_mag(float* dest);
void calibrateMPU9250(float* dest1,float* dest2);

void read_gyro(int16_t *buffer);
void read_acc(int16_t *buffer);
void get_acc(float *buffer);
void get_gyro(float* buffer);

// Functions called by commands
void initMPU9250();
void getAcc_mpu9250();
void getGyro_mpu9250();

// Setup Wake-On-Motion interrupt configuration
void setMPU9250WOM();


#endif /* MPU9250_H_ */
