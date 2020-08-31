// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------
#ifndef SHELL_H_
#define SHELL_H_

#include <stdbool.h>
#include <stdint.h>

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
int stringlen(char* str);
int substrlen(char* str);
bool isDigit(char n);
bool isNumber(int pos);
bool isFloat(int pos);
bool isHeximal(char n);
bool isHex(int pos);
int my_atoi(int pos);
bool isSubstring(char* str, int pos);
int digitCount(int num);
void my_reverse(char str[], int len);
void my_itoa(int num, char* str, int base, int digitNum);
int my_power(int base, int exponent);
float my_fpower(float base, int exponent);
void my_ftoa(float n, char* res, int afterpoint);
int my_asciiToUint(int pos);
void my_uint8ToHex(uint8_t u8, char* hex);
void getsUart0();

//-----------------------------------------------------------------------------
// Shell()
//-----------------------------------------------------------------------------
bool isCommand(char* str, int argument);
void shell(void);

#endif /* SHELL_H_ */
