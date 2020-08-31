// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define BLUE_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))
#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))


#define GREEN_LED_MASK 8
#define BLUE_LED_MASK 4
#define RED_LED_MASK 2

void initLed()
{
    // Enable GPIO port F peripherals
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;

    // Configure LED and pushbutton pins
    GPIO_PORTF_DIR_R |= BLUE_LED_MASK | GREEN_LED_MASK | RED_LED_MASK;  // bits 1 and 3 are outputs, other pins are inputs
    GPIO_PORTF_DR2R_R |= BLUE_LED_MASK | GREEN_LED_MASK | RED_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R |= BLUE_LED_MASK | GREEN_LED_MASK | RED_LED_MASK;  // enable LEDs and pushbuttons

}

void red_on_off(bool onoff){
    RED_LED = onoff;
}

void red_toggle(){
    if(RED_LED != 0){
        red_on_off(false);
    }else{
        red_on_off(true);
    }
}

void blue_on_off(bool onoff){
    BLUE_LED = onoff;
}

void blue_toggle(){
    if(BLUE_LED != 0){
        blue_on_off(false);
    }else{
        blue_on_off(true);
    }
}


void green_on_off(bool onoff){
    GREEN_LED = onoff;
}


void green_toggle(){
    if(GREEN_LED != 0){
        green_on_off(false);
    }else{
        green_on_off(true);
    }
}





