// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <stdint.h>
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
bool isLeap(int y);
int offsetDays(int d, int m, int y);
void revoffsetDays(int offset, int y, int *d, int *m);
void addDays(int x, char mode);
bool validDate(int d, int m, int y);

//-----------------------------------------------------------------------------
// Command Functions
//-----------------------------------------------------------------------------
void hello();
void reset();
void getTemperature();
void printTime();
void changeTime(uint8_t hour, uint8_t minute, uint8_t second);
void printDate();
void changeDate(uint8_t month, uint8_t day, uint16_t year);
void pollI2C();
void writeI2C(int add, int reg, int data);
void readI2C(int add, int reg);
void getAcc();
void getGyro();
void getMag();
void printGating();
void changeGatingTem(float gt, float lt);
void changeGatingAcc(float gt, float lt);
void periodicTemp(uint8_t t);
void periodic(uint8_t t);
void periodicCL(int time, int count);
void printHys();
void setHys(int temp);
void stepTriggerTemp();
void stepCounter();
void stepCounter2(int sample);

void sleepCmd(bool on);

uint32_t time_values();
uint32_t temperature_v();
void writeTimeData(unsigned int time, unsigned int smaple);

uint32_t acc_time_values();
void gatingDemo();



#endif /* COMMANDS_H_ */
