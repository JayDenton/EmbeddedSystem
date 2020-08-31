// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "hw_funcs.h"
#include "uart0.h"

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------
#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define RED_LED_MASK 2

void initHW()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;
}

void initInterrupts()
{
    NVIC_EN1_R = NVIC_EN1_INT_M;
}

// The Hibernation module is clocked by a 32.768-kHz oscillator connected to the XOSC0 pin.
void initRTC()
{
//    // clear the RTC Match interrupt if any
//    HIB_IC_R = HIB_IM_RTCALT0;
//    waitMicrosecond(1000);
//    while(!(HIB_MIS_R & HIB_MIS_WC)){}



    // Set interrupt masks for External Write Complete/Capable. This is in system clock domain, so it would be immediate
    HIB_IM_R = HIB_IM_WC;
    // Clocking enable
    HIB_CTL_R = HIB_CTL_CLK32EN;
    // Hibernation module has an independent clocking domain, hibernation registers must be written only with a timing gap between accesses.
    // Wait until the WC interrupt in the HIBMIS register has been triggered before performing any other operations with the Hibernation module.
    waitMicrosecond(1000);
    while(!(HIB_MIS_R & HIB_MIS_WC)){}
    // Write match value to maximum to prevent early match interrupt
    HIB_RTCM0_R = HIB_RTCM0_M;
    waitMicrosecond(1000);
    while(!(HIB_MIS_R & HIB_MIS_WC)){}

    // Set interrupt mask for RTC Alert 0
    HIB_IM_R |= HIB_IM_RTCALT0;
    waitMicrosecond(1000);
    while(!(HIB_MIS_R & HIB_MIS_WC)){}

    // Enable RTC timer
    HIB_CTL_R |= HIB_CTL_RTCEN;
    waitMicrosecond(1000);
    while(!(HIB_MIS_R & HIB_MIS_WC)){}
}

void initADC0Tsens()
{
    // PLL should be enabled and programmed to a supported crystal frequency in the RCC register
    // Enable ADC module 0 clock
    SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R0;
    // AIN0 - PE3
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
    // Configure GPIO for ADC input
    GPIO_PORTE_AFSEL_R |= 0x08;
    // Analog input
    GPIO_PORTE_DEN_R &= ~ 0x08;
    // Disable the analog isolation
    GPIO_PORTE_AMSEL_R |= 0x08;
    // Configure sample sequencer priorities in ADCSSPRI if needed
    // According to FIFO, SS0 is highest, SS3 is lowest.
    // There must be 3 system clocks after the ADC module clock is enabled before any ADC module registers are accessed.
    __asm("         NOP");
    __asm("         NOP");
    __asm("         NOP");
    // Sample Sequencer config, firstly disable it
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN3;
    // Config trigger event for SS3, Processor is default
    ADC0_EMUX_R = ADC_EMUX_EM3_PROCESSOR;
    // For each sample in the sample sequence, configure the corresponding input source. SS3 only have MUX0 sample field
    ADC0_SSMUX3_R = 0;  // Not really needed, since only the internal temperature is read.
    // Configure the sample control bits. Select temperature read, interrupt, and set end bit.
    ADC0_SSCTL3_R = ADC_SSCTL3_TS0 | ADC_SSCTL3_IE0 | ADC_SSCTL3_END0;
    // If interrupts are to be used, set the corresponding MASK bit in the ADCIM register.
    ADC0_IM_R = ADC_IM_MASK3;
    // Enable sample sequencer logic
    ADC0_ACTSS_R = ADC_ACTSS_ASEN3;
    // Clear SS3 interrupt status
    ADC0_ISC_R = ADC_ISC_IN3;
}

// Approximate busy waiting (in units of microseconds), given a 40 MHz system clock
void waitMicrosecond(uint32_t us)
{
    __asm("WMS_LOOP0:   MOV  R1, #6");          // 1
    __asm("WMS_LOOP1:   SUB  R1, #1");          // 6
    __asm("             CBZ  R1, WMS_DONE1");   // 5+1*3
    __asm("             NOP");                  // 5
    __asm("             NOP");                  // 5
    __asm("             B    WMS_LOOP1");       // 5*2 (speculative, so P=1)
    __asm("WMS_DONE1:   SUB  R0, #1");          // 1
    __asm("             CBZ  R0, WMS_DONE0");   // 1
    __asm("             NOP");                  // 1
    __asm("             B    WMS_LOOP0");       // 1*2 (speculative, so P=1)
    __asm("WMS_DONE0:");                        // ---
                                                // 40 clocks/us + error
}


void reloadRTC(uint32_t load)
{
    // Write load value to HIBRTCLD register;
    HIB_RTCLD_R = load;
    waitMicrosecond(1000);
    while(!(HIB_MIS_R & HIB_MIS_WC)){}
}

void RTCMatch()
{
    // write the match value
    HIB_RTCM0_R = 5; // say 5 seconds;
    // sub-seconds write to HIB_RTCSS_R = sub-second<< HIB_RTCSS_RTCSSM_S;
    // write the required RTC load value
    HIB_RTCLD_R = 0; // for instance 0 seconds
    // clear interrupts
    HIB_IC_R |= HIB_IC_RTCALT0;
    // write 0x0000.0041 to the HIBCTL register at offset 0x010
    HIB_CTL_R |= HIB_CTL_CLK32EN | HIB_CTL_RTCEN;
}

void toggleRedLED()
{
    if(RED_LED == 0)
        RED_LED = 1;
    else
        RED_LED = 0;
    return;
}

// Reboot function
void reset_hw(void)
{
    // Set the bit SYSRESREQ on APINT VECTKEY
    NVIC_APINT_R = 0x05FA0004;
}

// Read ADC0 Temperature sensor value
int16_t readADC0SS3()
{
    // Initiate Processor sampling in Sample Sequencer 3
    ADC0_PSSI_R = ADC_PSSI_SS3;
    // Wait until SS3 interrupt is active
    while(!(ADC0_ISC_R & ADC_ISC_IN3)){}
    // Clear the interrupt flag for next use
    ADC0_ISC_R = ADC_ISC_IN3;
    // Some delay
    waitMicrosecond(1000);
    // Get result from FIFO3
    return ADC0_SSFIFO3_R;
}



