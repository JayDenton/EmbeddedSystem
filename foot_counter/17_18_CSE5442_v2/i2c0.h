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

#ifndef I2C0_H_
#define I2C0_H_

#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initI2c0();
void writeI2c0Register(uint8_t add, uint8_t reg, uint8_t data);
void writeI2c0Registers(uint8_t add, uint8_t reg, uint8_t data[], uint8_t size);
uint8_t readI2c0Register(uint8_t add, uint8_t reg);
void burstReadI2C0Registers(uint8_t add, uint8_t reg,uint8_t size,uint8_t data[]);
void readI2c0Registers(uint8_t add, uint8_t reg, uint8_t count, uint8_t* dest);
bool pollI2c0Address(uint8_t add);
bool isI2c0Error();

#endif
