// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II


#include <stdint.h>
#include <stdbool.h>
#include "MPU9250_reg.h"
#include "mpu9250.h"
#include "hw_funcs.h"
#include "i2c0.h"

// Global variable for calibration
float gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0};
float magCalibration[3] = {0, 0, 0};
float mag_ASA[3];
void calibrate_mag();
void read_mag(int16_t *dest);
void get_mag(float* dest);


void init_mpu9250(void){
    // Reset all gyro digital signal path, accel digital signal path, and temp digital signal path.
    writeRegister(MPUREG_PWR_MGMT_1,0x80);
    waitMicrosecond(100); //add delay
    // Reset I2C Master module. Reset is asynchronous.
    writeRegister(MPUREG_PWR_MGMT_1,0x01);
    waitMicrosecond(1000);//add delay
    // Configure Gyro and Thermometer
    // Disable FSYNC and set thermometer and gyro bandwidth to 41 and 42 Hz, respectively;
    // minimum delay time for this setting is 5.9 ms, which means sensor fusion update rates cannot
    // be higher than 1 / 0.0059 = 170 Hz
    // DLPF_CFG = bits 2:0 = 011; this limits the sample rate to 1000 Hz for both
    // With the MPU9250, it is possible to get gyro sample rates of 32 kHz (!), 8 kHz, or 1 kHz


    writeRegister(MPUREG_SMPLRT_DIV,0x04);  // sample rate 200hz

    // Gyro Configuration
    uint8_t config = readRegister(MPUREG_GYRO_CONFIG);
//    uint8_t gscale = 0;   // 250DPS
////    uint8_t gscale = 1; // 500DPS
////    uint8_t gscale = 2; // 1000DPS
////    uint8_t gscale = 3; // 2000DPS
//    config &= ~ 0x03; //clear Fchoise bits
//    config &= ~ 0x18; // clear GFS bits
//    config |= gscale << 3;
    // 500 degree/s and FCHOICE_B
    writeRegister(MPUREG_GYRO_CONFIG,0x0B);
    // Turn on the internal low-pass filter for gyroscope with 10Hz bandwidth
    writeRegister(MPUREG_CONFIG, 0x05);

    // Acceleration configuration
    config = readRegister(MPUREG_ACCEL_CONFIG);
    config &= ~0x18; // clear AFS bits;
    uint8_t ascale = 0; //2g
//    uint8_t ascale = 1; //4g
//    uint8_t ascale = 2; //8g
//    uint8_t ascale = 3; //16g
    config |= ascale << 3;
    writeRegister(MPUREG_ACCEL_CONFIG,config);

    // Acceleration rate configuration
    config = readRegister(MPUREG_ACCEL_CONFIG_2);
    config &= ~0x0F; // clear accel_fchoise_b and A_DLPFG
    config |= 0x03; // set bandwidth 41hz and acc rate 1khz
    writeRegister(MPUREG_ACCEL_CONFIG_2,config);

    // INT interrupt pin configuration
    // Register 55 - INT Pin / Bypass Enable Configuration (MPUREG_INT_PIN_CFG)
    // Register 56 – Interrupt Enable (MPUREG_INT_ENABLE)
    // Register 58 – Interrupt Status (0x3A)

//    writeRegister(MPUREG_INT_PIN_CFG,0x22); // set bypass
    // Clear interrupt status
    writeRegister(0x3A, 0x00);
    waitMicrosecond(1000);//add delay
    // Interrupt enable: ACTL, active low; BYPASS_EN
    writeRegister(MPUREG_INT_PIN_CFG, 0x82);
    waitMicrosecond(1000);//add delay
    // Interrupt enable: WOM_EN, wake on motion
    writeRegister(MPUREG_INT_ENABLE, 0x40);


    return;
}

void init_AK8963(float* dest){
     uint8_t rawData[3];
     writeI2c0Register(AK8963_ADDR,AK8963_CNTL1, 0x00);
     waitMicrosecond(10);
     writeI2c0Register(AK8963_ADDR, AK8963_CNTL1, 0x0F);
     waitMicrosecond(10);
     readI2c0Registers(AK8963_ADDR, AK8963_ASAX, 3, rawData);
     dest[0] = (float)(rawData[0] - 128)/256. +1;
     dest[1] = (float)(rawData[1] - 128)/256. +1;
     dest[2] = (float)(rawData[2] - 128)/256. +1;
     writeI2c0Register(AK8963_ADDR, AK8963_CNTL1, 0x00);
     waitMicrosecond(10);
     int8_t Mmode = 0x02;
     int8_t Mscale = 1;
     writeI2c0Register(AK8963_ADDR, AK8963_CNTL1, Mscale << 4 | Mmode);
     waitMicrosecond(10);
}

// Magnetometer
void calibrate_mag(){
    uint8_t response[3];
    float data;
    int i;

    readI2c0Registers(AK8963_I2C_ADDR, AK8963_ASAX, 3, response);

    writeI2c0Register(AK8963_I2C_ADDR,AK8963_CNTL2, 0x01);                                                             // reset AK8963
    waitMicrosecond(10000);
    writeI2c0Register(AK8963_I2C_ADDR,AK8963_CNTL1, BITS_AK8963_POWERDOWN);              // set AK8963 to Power Down
    waitMicrosecond(10000);                                                                 // long wait between AK8963 mode changes
    writeI2c0Register(AK8963_I2C_ADDR,AK8963_CNTL1, BITS_AK8963_ROM);                                // set AK8963 to FUSE ROM access
    waitMicrosecond(10000);                                                         // long wait between AK8963 mode changes
    readI2c0Registers(AK8963_I2C_ADDR, AK8963_ASAX, 3, response);



    for(i = 0; i < 3; i++) {
        data=response[i];
        mag_ASA[i] = ((data-128)/256+1)*Magnetometer_Sensitivity_Scale_Factor;
    }
    writeI2c0Register(AK8963_I2C_ADDR,AK8963_CNTL1, BITS_AK8963_ONESHOT);
}


void read_mag(int16_t *dest){
    uint8_t rawData[7];
    if(readI2c0Register(AK8963_ADDR, AK8963_ST1)& 0x01){
        readI2c0Registers(AK8963_ADDR, AK8963_HXL, 6, rawData);
        uint8_t c = rawData[6];
        if(!(c&0x08)){
            dest[0] = ((int16_t)rawData[1] << 8 ) | rawData[0];
            dest[1] = ((int16_t)rawData[3] << 8 ) | rawData[2];
            dest[2] = ((int16_t)rawData[5] << 8 ) | rawData[4];
        }
    }
}


void get_mag(float* dest){
   int16_t data[3];
   read_mag(data);
   int i = 0;
   for(i=0; i< 3 ; ++i){
       dest[i] = (float)data[i]*mag_ASA[i];
   }
}



void calibrateMPU9250(float* dest1,float* dest2){
    uint8_t data[20];
    uint16_t index, p_count, ff_count;
    int32_t gyro_bias[3] = {0,};
    int32_t accel_bias[3] = {0,};

    writeRegister(MPUREG_PWR_MGMT_1,0x80);
    waitMicrosecond(100);

    writeRegister(MPUREG_PWR_MGMT_1,0x10);
    writeRegister(MPUREG_PWR_MGMT_2,0x00);
    waitMicrosecond(200);

    writeRegister(MPUREG_INT_ENABLE,0x00);
    writeRegister(MPUREG_FIFO_EN,0x00);
    writeRegister(MPUREG_PWR_MGMT_1,0x00);
    writeRegister(MPUREG_I2C_MST_CTRL,0x00);
    writeRegister(MPUREG_USER_CTRL,0x00);
    writeRegister(MPUREG_USER_CTRL,0x0C);
    waitMicrosecond(15);

    writeRegister(MPUREG_CONFIG,0x01);
    writeRegister(MPUREG_SMPLRT_DIV,0x00);
    writeRegister(MPUREG_GYRO_CONFIG,0x00);
    writeRegister(MPUREG_ACCEL_CONFIG,0x00);

    uint16_t gyrosensitivity = 131; // 131 LSB/degree/sec
    uint16_t accelsensitivity = 16384; // 16384 LSB/g

    writeRegister(MPUREG_USER_CTRL,0x40);
    writeRegister(MPUREG_FIFO_EN,0x78);
    waitMicrosecond(40);

//    writeRegister(MPUREG_USER_CTRL,0x00);
    readI2c0Registers(MPU9250_ADDR, MPUREG_FIFO_COUNTH, 2,data);
    ff_count = ((uint16_t)data[0] << 8) | data[1];
    p_count = ff_count/12;
    for(index = 0 ; index < p_count ; ++index){
        int16_t accel_temp[3] = {0,}, gyro_temp[3] = {0,};
        burstReadI2C0Registers(MPU9250_ADDR, MPUREG_FIFO_R_W, 12, data);

        accel_temp[0]   = (int16_t)(((int16_t)data[0] << 8 )    | data[1]);
        accel_temp[1]   = (int16_t)(((int16_t)data[2] << 8 )    | data[3]);
        accel_temp[2]   = (int16_t)(((int16_t)data[4] << 8 )    | data[5]);

        gyro_temp[0]    = (int16_t)(((int16_t)data[6] << 8 )    | data[7]);
        gyro_temp[1]    = (int16_t)(((int16_t)data[8] << 8 )    | data[9]);
        gyro_temp[2]    = (int16_t)(((int16_t)data[10] << 8 )   | data[11]);

        accel_bias[0] += (int32_t) accel_temp[0];
        accel_bias[1] += (int32_t) accel_temp[1];
        accel_bias[2] += (int32_t) accel_temp[2];

        gyro_bias[0] += (int32_t) gyro_temp[0];
        gyro_bias[1] += (int32_t) gyro_temp[1];
        gyro_bias[2] += (int32_t) gyro_temp[2];
    }
    accel_bias[0] /= (int32_t)p_count;
    accel_bias[1] /= (int32_t)p_count;
    accel_bias[2] /= (int32_t)p_count;

    gyro_bias[0] /= (int32_t)p_count;
    gyro_bias[1] /= (int32_t)p_count;
    gyro_bias[2] /= (int32_t)p_count;

    if(accel_bias[2] > 0L ){
        accel_bias[2] -= (int32_t) accelsensitivity;
    }else{
        accel_bias[2] += (int32_t) accelsensitivity;
    }
    data[0] = (-gyro_bias[0]/4 >>8) & 0xFF;
    data[1] = (-gyro_bias[0]/4)     & 0xFF;
    data[2] = (-gyro_bias[1]/4 >>8) & 0xFF;
    data[3] = (-gyro_bias[1]/4)     & 0xFF;
    data[4] = (-gyro_bias[2]/4 >>8) & 0xFF;
    data[5] = (-gyro_bias[2]/4)     & 0xFF;

    writeRegister(MPUREG_XG_OFFS_USRH,data[0]);
    writeRegister(MPUREG_XG_OFFS_USRL,data[1]);
    writeRegister(MPUREG_YG_OFFS_USRH,data[2]);
    writeRegister(MPUREG_YG_OFFS_USRL,data[3]);
    writeRegister(MPUREG_ZG_OFFS_USRH,data[4]);
    writeRegister(MPUREG_ZG_OFFS_USRL,data[5]);

    dest1[0] = (float)gyro_bias[0]/(float)gyrosensitivity;
    dest1[1] = (float)gyro_bias[1]/(float)gyrosensitivity;
    dest1[2] = (float)gyro_bias[2]/(float)gyrosensitivity;

    int32_t accel_bias_reg[3] = {0,};
    readI2c0Registers(MPU9250_ADDR, MPUREG_XA_OFFSET_H, 2, data);
    accel_bias_reg[0] = (int32_t)(((int16_t)data[0] << 8 )| data[1]);

    readI2c0Registers(MPU9250_ADDR, MPUREG_YA_OFFSET_H, 2, data);
    accel_bias_reg[1] = (int32_t)(((int16_t)data[0] << 8 )| data[1]);

    readI2c0Registers(MPU9250_ADDR, MPUREG_ZA_OFFSET_H, 2, data);
    accel_bias_reg[2] = (int32_t)(((int16_t)data[0] << 8 )| data[1]);

    int32_t mask = 1uL;
    int8_t mask_bit[3] = {0,};

    for(index = 0; index < 3 ; ++index){
        if((accel_bias_reg[index] & mask))
            mask_bit[index] = 0x01;
    }

    accel_bias_reg[0] -= (accel_bias[0]/8);
    accel_bias_reg[1] -= (accel_bias[1]/8);
    accel_bias_reg[2] -= (accel_bias[2]/8);

    data[0] = (accel_bias_reg[0] >> 8)  & 0xFF;
    data[1] = (accel_bias_reg[0])       & 0xFF;
    data[1] = data[1] | mask_bit[0];
    data[2] = (accel_bias_reg[1] >> 8)  & 0xFF;
    data[3] = (accel_bias_reg[1])       & 0xFF;
    data[3] = data[3] | mask_bit[1];
    data[4] = (accel_bias_reg[2] >> 8)  & 0xFF;
    data[5] = (accel_bias_reg[2])       & 0xFF;
    data[5] = data[5] | mask_bit[2];
//
//    writeRegister(MPUREG_XA_OFFSET_H,data[0]);
//    writeRegister(MPUREG_XA_OFFSET_L,data[1]);
//    writeRegister(MPUREG_YA_OFFSET_H,data[2]);
//    writeRegister(MPUREG_YA_OFFSET_L,data[3]);
//    writeRegister(MPUREG_ZA_OFFSET_H,data[4]);
//    writeRegister(MPUREG_ZA_OFFSET_L,data[5]);

    dest2[0] = (float)accel_bias[0]/(float)accelsensitivity;
    dest2[1] = (float)accel_bias[1]/(float)accelsensitivity;
    dest2[2] = (float)accel_bias[2]/(float)accelsensitivity;

}

void read_acc(int16_t *buffer){
    uint8_t rawData[6];
    readI2c0Registers(MPU9250_ADDR, MPUREG_ACCEL_XOUT_H, 6, rawData);
    buffer[0] = ((int16_t)rawData[0] << 8 )|rawData[1];
    buffer[1] = ((int16_t)rawData[2] << 8 )|rawData[3];
    buffer[2] = ((int16_t)rawData[4] << 8 )|rawData[5];
}

void read_gyro(int16_t *buffer){
    uint8_t rawData[6];
    readI2c0Registers(MPU9250_ADDR, MPUREG_GYRO_XOUT_H, 6, rawData);
    buffer[0] = ((int16_t)rawData[0] << 8 )|rawData[1];
    buffer[1] = ((int16_t)rawData[2] << 8 )|rawData[3];
    buffer[2] = ((int16_t)rawData[4] << 8 )|rawData[5];
}


void get_acc(float* buffer){

    float aRes = MPU9250A_2g;
    int16_t rawData[3];
    read_acc(rawData);
    buffer[0] = (float)rawData[0] * aRes;
    buffer[1] = (float)rawData[1] * aRes;
    buffer[2] = (float)rawData[2] * aRes;

}

void get_gyro(float* buffer){
    float gRes = 250.0/32768.0;
    int16_t rawData[3];
    read_gyro(rawData);
    buffer[0] = (float)rawData[0] * gRes;
    buffer[1] = (float)rawData[1] * gRes;
    buffer[2] = (float)rawData[2] * gRes;

}

void initMPU9250()
{
    calibrateMPU9250(gyroBias,accelBias);
    init_AK8963(magCalibration);
    init_mpu9250();
}

void getAcc_mpu9250(float* acc)
{
    get_acc(acc);
    acc[0] -= accelBias[0];
    acc[1] -= accelBias[1];
    acc[2] -= accelBias[2];
}

void getGyro_mpu9250(float* buffer)
{
    get_gyro(buffer);
    buffer[0] -= gyroBias[0];
    buffer[1] -= gyroBias[1];
    buffer[2] -= gyroBias[2];
}



// Configure Wake-On-Motion interrupt
void setMPU9250WOM(){

    writeI2c0Register(0x0C, 0x0A, 0x00);

    writeRegister(MPUREG_PWR_MGMT_1,0x80);

    waitMicrosecond(1000);

    writeRegister(MPUREG_PWR_MGMT_1,0x00);

    // Interrupt enable: ACTL, active low
    writeRegister(MPUREG_INT_PIN_CFG, 0x80);

    writeRegister(MPUREG_PWR_MGMT_2,0x07);

    writeRegister(MPUREG_ACCEL_CONFIG_2,0x01);

    writeRegister(MPUREG_INT_ENABLE,0x40);

    writeRegister(MPUREG_MOT_DETECT_CTRL,(0x80|0x40));

    // Motion threshold
    writeRegister(MPUREG_MOT_THR,0xC0);

    writeRegister(MPUREG_LP_ACCEL_ODR,(uint8_t)0x7);

    writeRegister(MPUREG_PWR_MGMT_1,0x20);
}
