// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL with LCD/Keyboard Interface
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// I2C devices on I2C bus 0 with 2kohm pullups on SDA and SCL

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "i2c0.h"

// PortB masks
#define SDA_MASK 8
#define SCL_MASK 4

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initI2c0()
{
    // Enable clocks
    SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;

    // Configure I2C
    GPIO_PORTB_DIR_R |= SDA_MASK | SCL_MASK;            // make bits 2 and 3 outputs
    GPIO_PORTB_DR2R_R |= SDA_MASK | SCL_MASK;           // set drive strength to 2mA
    GPIO_PORTB_DEN_R |= SDA_MASK | SCL_MASK;            // enable digital
    GPIO_PORTB_ODR_R |= SDA_MASK;                       // enable open drain outputs
    GPIO_PORTB_AFSEL_R |= SDA_MASK | SCL_MASK;          // configure auxiliary fn
    GPIO_PORTB_PCTL_R &= ~(GPIO_PCTL_PB2_M | GPIO_PCTL_PB3_M);
    GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB2_I2C0SCL | GPIO_PCTL_PB3_I2C0SDA;

    // Configure I2C0 peripheral
    I2C0_MCR_R = I2C_MCR_MFE;                           // master
    I2C0_MTPR_R = 19;                                   // (40MHz/2) / (19+1) = 100kbps
}

void writeI2c0Register(uint8_t add, uint8_t reg, uint8_t data)
{
    I2C0_MSA_R = add*2;
    I2C0_MDR_R = reg;
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    I2C0_MDR_R = data;
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_STOP | I2C_MCS_RUN;
    while (!(I2C0_MRIS_R & I2C_MRIS_RIS));
}

void writeI2c0Registers(uint8_t add, uint8_t reg, uint8_t data[], uint8_t size)
{
    uint8_t i;
    I2C0_MSA_R = add*2;
    I2C0_MDR_R = reg;
    if (size == 0)
    {
        I2C0_MICR_R = I2C_MICR_IC;
        I2C0_MCS_R = I2C_MCS_STOP | I2C_MCS_START | I2C_MCS_RUN;
        while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    }
    else
    {
        I2C0_MICR_R = I2C_MICR_IC;
        I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN;
        while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
        for (i = 0; i < size-1; i++)
        {
            I2C0_MDR_R = data[i];
            I2C0_MICR_R = I2C_MICR_IC;
            I2C0_MCS_R = I2C_MCS_RUN;
            while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
        }
        I2C0_MDR_R = data[size-1];
        I2C0_MICR_R = I2C_MICR_IC;
        I2C0_MCS_R = I2C_MCS_STOP | I2C_MCS_RUN;
        while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    }
}

uint8_t readI2c0Register(uint8_t add, uint8_t reg)
{
    I2C0_MSA_R = add*2;
    I2C0_MDR_R = reg;
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    I2C0_MSA_R = add*2 + 1;
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_STOP | I2C_MCS_START | I2C_MCS_RUN;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    return I2C0_MDR_R;
}

void burstReadI2C0Registers(uint8_t add, uint8_t reg,uint8_t size,uint8_t data[])
{
    uint8_t index = 0;
    for(; index < size ; ++index){
        *(data+index) = readI2c0Register(add,reg);
    }
    return;
}

void readI2c0Registers(uint8_t add, uint8_t reg, uint8_t count, uint8_t* dest)
{
    uint8_t index = 0;
    for(; index < count ; ++index){
        *(dest+index) = readI2c0Register(add,reg+index);
    }
    return;
}

bool pollI2c0Address(uint8_t add)
{
    I2C0_MSA_R = add*2 + 1;
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_STOP | I2C_MCS_START | I2C_MCS_RUN;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    return !(I2C0_MCS_R & I2C_MCS_ERROR);
}

bool isI2c0Error()
{
    return !(I2C0_MCS_R & I2C_MCS_ERROR);
}

