// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

#include<stdint.h>
#include "interrupts.h"
#include "uart0.h"
#include "hw_funcs.h"
#include "tm4c123gh6pm.h"
#include "shell.h"

void sleep()
{
    putsUart0("board is going to sleep.\r\n");
    waitMicrosecond(1000);
    __asm(" WFI");
}

void setGPIOFITR()
{
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
    GPIO_PORTF_LOCK_R =  0x4C4F434B;
    GPIO_PORTF_CR_R = 0xFF;
    GPIO_PORTF_DEN_R |= 0x01;
    GPIO_PORTF_DR2R_R |= 0x01;
    GPIO_PORTF_DIR_R |= 1;
    GPIO_PORTF_PUR_R |= 0x01;
    // Configure falling edge interrupts on row inputs
    // (edge mode, single edge, falling edge, clear any interrupts, turn on)
    GPIO_PORTF_IS_R = 1;        // level-sensitive
    GPIO_PORTF_IBE_R &= ~1;
    GPIO_PORTF_IEV_R &= ~1;
    GPIO_PORTF_IM_R = 1;
    GPIO_PORTF_LOCK_R = 0;
    GPIO_PORTF_ICR_R = 0xFF;

    // Interrupt enable GPIO port F - 30
//    NVIC_EN0_R |= 1 << (INT_GPIOF-16); // turn-on interrupt (GPIOF)
}

void setHibernationITR()
{
    // Set interrupt mask for RTC Alert 0
    HIB_IM_R |= HIB_IM_RTCALT0;
    waitMicrosecond(1000);
    while(!(HIB_MIS_R & HIB_MIS_WC)){}

    // Interrupt enable Hibernate Module - 43
    NVIC_EN1_R |= 1 << (INT_HIBERNATE-48); // turn-on interrupt Hibernation
}

void GPIOFITR()
{
    // Clear GPIO_F0
    GPIO_PORTF_ICR_R |= 1;
    waitMicrosecond(100000);
    putsUart0("wake up board..\r\n");
    // Interrupt enable GPIO port F - 30

}

void HibernationITR()
{
    putsUart0("Hibernation Interrupt\r\n");

    // clear the RTC Match interrupt
    HIB_IC_R = HIB_IM_RTCALT0;
    waitMicrosecond(1000);
    while(!(HIB_MIS_R & HIB_MIS_WC)){}

    // When the hibernation triggered, go to sleep
    sleep();

    shell();
}
