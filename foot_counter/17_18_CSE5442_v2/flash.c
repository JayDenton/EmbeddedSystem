// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

// 1 word  = 4 bytes
// 1 block = 32 words (128 bytes)
// 1 page  = 8 blocks (1024 bytes)


#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "flash.h"
#include "uart0.h"
#include "shell.h"


#define PAGE_START  0x00008000
#define PAGE_SIZE   0x400       // 1-KB
#define PAGES       64          // 64-KB, addresses 0x0000.8000 - 0x0001.7FFF
#define BLOCK_SIZE  0x80        // 128-B
#define BLOCK_START 0x00008000
#define BLOCK_END   0x00017F80

//uint32_t PREV_BLOCK = BLOCK_START;
uint32_t CURR_BLOCK = BLOCK_START;
uint32_t NEXT_BLOCK = BLOCK_START+BLOCK_SIZE;

// Write a 32-bit word
void writeWord(uint32_t data, uint32_t address)
{
    // Write source data to the FMD register
    FLASH_FMD_R = data;
    // Write the target address to the FMA register
    FLASH_FMA_R = address;
    // Write the Flash memory write key and the WRITE bit to the FMC register
    // Write into the WRKEY field for a Flash memory write to occur
    // Depending on the value of the KEY bit in the BOOTCFG register, if 0->0x71D5, if 1->0xA442
    // FLASH_BOOTCFG_KEY
    FLASH_FMC_R = FLASH_FMC_WRKEY | FLASH_FMC_WRITE;
    // Poll the FMC register until the WRITE bit is cleared
    while(FLASH_FMC_R & FLASH_FMC_WRITE){};
}

// Erase a 1-KB page
void erasePage(uint32_t address)
{
    // Write the page address to the FMA register
    FLASH_FMA_R = address;
    // Write the Flash memory write key and the ERASE bit to the FMC register
    FLASH_FMC_R = FLASH_FMC_WRKEY | FLASH_FMC_ERASE;
    // Poll the FMC register until the ERASE bit is cleared
    while(FLASH_FMC_R & FLASH_FMC_ERASE){};
    // Clear the Programming Raw Interrupt Status
    FLASH_FCMISC_R = FLASH_FCMISC_PMISC;
}

// Erase 64 KB for data logging. Which means 512 Blocks of data.
void initFlash()
{
    // Set the KEY bit in the BOOTCFG register up for later use. Default is 1.
    FLASH_BOOTCFG_R |= FLASH_BOOTCFG_KEY;
    uint8_t i;
    for(i = 0; i < PAGES; i++)
        erasePage(PAGE_START+PAGE_SIZE*i);
}

void read()
{
    uint32_t value = (*((volatile uint32_t*)(PAGE_START+(PAGE_SIZE)*30+(0x3FA))));
}

void writeData(uint32_t data)
{
    if(NEXT_BLOCK!=BLOCK_END)
    {
        writeWord(data, CURR_BLOCK);
        // Update block pointer
        CURR_BLOCK = NEXT_BLOCK;
        NEXT_BLOCK += BLOCK_SIZE;
    }
}

unsigned createMask(unsigned a, unsigned b)
{
   unsigned r = 0;
   unsigned i;
   for (i=a; i<=b; i++)
       r |= 1 << i;

   return r;
}

void readData(unsigned int sample)
{
    unsigned int i;
    uint32_t tempData;
    uint8_t hh;
    uint8_t mi;
    uint8_t ss;
    char hour[10], minute[10], second[10];

    uint16_t int_part;
    uint16_t f_part;

    char int_str[10], f_str[10];


    for(i=0; i<sample; i++)
    {
        // Get the data value from the address
        tempData = (*((volatile uint32_t*)(PAGE_START+(BLOCK_SIZE)*i)));

        // Print the temperature data
        if(i%2 == 0 && !(tempData & 0x70000000))
        {
            // Print the time
            tempData = (*((volatile uint32_t*)(PAGE_START+(BLOCK_SIZE)*i)));
            ss = tempData & createMask(0,5);
            mi = (tempData & createMask(6,11))>>6;
            hh = (tempData & createMask(12,16))>>12;
            my_itoa(ss, second, 10, 2);
            my_itoa(mi, minute, 10, 2);
            my_itoa(hh, hour, 10, 2);
            putsUart0(hour);
            putsUart0(":");
            putsUart0(minute);
            putsUart0(":");
            putsUart0(second);
            putsUart0(" ");

            i++;

            // Print the temperature
            tempData = (*((volatile uint32_t*)(PAGE_START+(BLOCK_SIZE)*i)));
            f_part = (tempData & createMask(0, 13));
            int_part = (tempData & createMask(14, 29))>>14;
            if(tempData & 0x80000000)
                putsUart0("-");
            my_itoa(int_part, int_str, 10, 2);
            my_itoa(f_part, f_str, 10, 4);
            putsUart0(int_str);
            putsUart0(".");
            putsUart0(f_str);
            putsUart0("\r\n");

        }
        // Print the acceleration data
        if(i%2 == 0 && (tempData & 0x70000000))
        {
            // Print the time
            tempData = (*((volatile uint32_t*)(PAGE_START+(BLOCK_SIZE)*i)));
            ss = tempData & createMask(0,5);
            mi = (tempData & createMask(6,11))>>6;
            hh = (tempData & createMask(12,16))>>12;
            my_itoa(ss, second, 10, 2);
            my_itoa(mi, minute, 10, 2);
            my_itoa(hh, hour, 10, 2);
            putsUart0(hour);
            putsUart0(":");
            putsUart0(minute);
            putsUart0(":");
            putsUart0(second);
            putsUart0(" ");

            i++;

            // Print the X value
            tempData = (*((volatile uint32_t*)(PAGE_START+(BLOCK_SIZE)*i)));
            f_part = (tempData & createMask(0, 13));
            int_part = (tempData & createMask(14, 29))>>14;
            putsUart0(" X=");
            if(tempData & 0x80000000)
                putsUart0("-");
            my_itoa(int_part, int_str, 10, 2);
            my_itoa(f_part, f_str, 10, 4);
            putsUart0(int_str);
            putsUart0(".");
            putsUart0(f_str);
            putsUart0(" ");

            i++;

            // Print the Y value
            tempData = (*((volatile uint32_t*)(PAGE_START+(BLOCK_SIZE)*i)));
            f_part = (tempData & createMask(0, 13));
            int_part = (tempData & createMask(14, 29))>>14;
            putsUart0(" Y=");
            if(tempData & 0x80000000)
                putsUart0("-");
            my_itoa(int_part, int_str, 10, 2);
            my_itoa(f_part, f_str, 10, 4);
            putsUart0(int_str);
            putsUart0(".");
            putsUart0(f_str);
            putsUart0(" ");

            i++;

            // Print the Z value
            tempData = (*((volatile uint32_t*)(PAGE_START+(BLOCK_SIZE)*i)));
            f_part = (tempData & createMask(0, 13));
            int_part = (tempData & createMask(14, 29))>>14;
            putsUart0(" Z=");
            if(tempData & 0x80000000)
                putsUart0("-");
            my_itoa(int_part, int_str, 10, 2);
            my_itoa(f_part, f_str, 10, 4);
            putsUart0(int_str);
            putsUart0(".");
            putsUart0(f_str);
            putsUart0("\r\n");

        }
    }
}
