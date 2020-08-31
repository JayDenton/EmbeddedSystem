// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>

void writeWord(uint32_t data, uint32_t address);
void erasePage(uint32_t address);
void initFlash();
void read();
void writeData(uint32_t data);
unsigned createMask(unsigned a, unsigned b);
void readData(unsigned int sample);

#endif /* FLASH_H_ */
