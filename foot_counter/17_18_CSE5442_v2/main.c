// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

// The commands are in shells, and command functions are in commands.c

#include "shell.h"
#include "uart0.h"
#include "i2c0.h"
#include "hw_funcs.h"
#include "commands.h"
#include "mpu9250.h"
#include "interrupts.h"
#include "led.h"
#include "flash.h"

#include <math.h>

// Debug() test temporary command functions
void debug()
{
    temperature_v();
}

int main()
{
    initHW();
    initFlash();
    initUart0();
    initI2c0();
    initMPU9250();
    calibrate_mag();
    initADC0Tsens();
    initRTC();
    reloadRTC(0);
    initLed();
    green_toggle();
    setGPIOFITR();
    putsUart0("Shell()\r\n");
    putcUart0('>');
    shell();
//    debug();
//    waitMicrosecond(10000000);
}
