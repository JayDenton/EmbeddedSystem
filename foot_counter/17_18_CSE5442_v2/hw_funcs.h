// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------
#ifndef HW_FUNCS_H_
#define HW_FUNCS_H_

#include <stdint.h>

//-----------------------------------------------------------------------------
// Hardware Initials
//-----------------------------------------------------------------------------
void initHW();
void initInterrupts();
void initRTC();
void initADC0Tsens();

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
void waitMicrosecond(uint32_t us);

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
void reloadRTC(uint32_t load);
void RTCMatch();
void toggleRedLED();
void reset_hw();
int16_t readADC0SS3();


#endif /* HW_FUNCS_H_ */
