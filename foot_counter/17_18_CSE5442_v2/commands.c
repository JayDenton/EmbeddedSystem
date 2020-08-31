// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "i2c0.h"
#include "hw_funcs.h"
#include "mpu9250.h"
#include "shell.h"
#include "interrupts.h"
#include "led.h"
#include "flash.h"

// Global variables for time and date setting
uint8_t hh = 0;
uint8_t mi = 0;
uint8_t ss = 0;
uint8_t mo = 1;
uint8_t dd = 1;
uint16_t yy = 2019;

// Global variable for gating values
float temperature_gt = 40.1;
float temperature_lt = 19.9;
float acceleration_gt = 1.2;
float acceleration_lt = 0.8;

// Global step count variable
uint32_t stepC = 0;

bool sleep_mode = false;

int hysterisis = 0;

/*
 * Subroutines
 */
// Return if year is leap year or not.
// Ref:geeksforgeeks.org
bool isLeap(int y)
{
    if (y % 100 != 0 && y % 4 == 0 || y % 400 == 0)
        return true;

    return false;
}

// Given a date, returns number of days elapsed
// from the  beginning of the current year (1st
// jan). Ref:geeksforgeeks.org
int offsetDays(int d, int m, int y)
{
    int offset = d;

    switch (m - 1)
    {
    case 11:
        offset += 30;
    case 10:
        offset += 31;
    case 9:
        offset += 30;
    case 8:
        offset += 31;
    case 7:
        offset += 31;
    case 6:
        offset += 30;
    case 5:
        offset += 31;
    case 4:
        offset += 30;
    case 3:
        offset += 31;
    case 2:
        offset += 28;
    case 1:
        offset += 31;
    }

    if (isLeap(y) && m > 2)
        offset += 1;

    return offset;
}

// Given a year and days elapsed in it, finds
// date by storing results in d and m.
// Ref:geeksforgeeks.org
void revoffsetDays(int offset, int y, int *d, int *m)
{
    int month[13] = { 0, 31, 28, 31, 30, 31, 30,
                      31, 31, 30, 31, 30, 31 };

    if (isLeap(y))
        month[2] = 29;

    int i;
    for (i = 1; i <= 12; i++)
    {
        if (offset <= month[i])
            break;
        offset = offset - month[i];
    }

    *d = offset;
    *m = i;
}

// Add x days to the given date.
// Ref:geeksforgeeks.org
void addDays(int x, char mode)
{
    char date[10];
    int offset1 = offsetDays(dd, mo, yy);
    int remDays = isLeap(yy)?(366-offset1):(365-offset1);

    // y2 is going to store result year and
    // offset2 is going to store offset days
    // in result year.
    int y2, offset2;
    if (x <= remDays)
    {
        y2 = yy;
        offset2 = offset1 + x;
    }

    else
    {
        // x may store thousands of days.
        // We find correct year and offset
        // in the year.
        x -= remDays;
        y2 = yy + 1;
        int y2days = isLeap(y2)?366:365;
        while (x > y2days)
        {
            x -= y2days;
            y2++;
            y2days = isLeap(y2)?366:365;
        }
        offset2 = x;
    }

    // Find values of day and month from
    // offset of result year.
    int m2, d2;
    revoffsetDays(offset2, y2, &d2, &m2);

    // If it is the time changing mode, update date values
    if(mode == 't')
    {
        yy = y2;
        mo = m2;
        dd = d2;
    }
    // If it is the date printing only, just print dates by offset, no date values update
    else if(mode == 'd')
    {
        // Convert integers to string
        my_itoa(m2, date, 10, 2);
        date[2] = '/';
        my_itoa(d2, date+3, 10, 2);
        date[5] = '/';
        my_itoa(y2, date+6, 10, 4);

        putsUart0(date);
    }
}

// Returns true if given
// year is valid or not.
// Ref:geeksforgeeks.org
bool validDate(int d, int m, int y)
{
    // If year, month and day
    // are not in given range
//    if (y > MAX_VALID_YR || y < MIN_VALID_YR)
//    return false;
    if (m < 1 || m > 12)
    return false;
    if (d < 1 || d > 31)
    return false;

    // Handle February month
    // with leap year
    if (m == 2)
    {
        if (isLeap(y))
        return (d <= 29);
        else
        return (d <= 28);
    }

    // Months of April, June,
    // Sept and Nov must have
    // number of days less than
    // or equal to 30.
    if (m == 4 || m == 6 ||
        m == 9 || m == 11)
        return (d <= 30);

    return true;
}


/*
 * Command functions
 */
void hello()
{
    putsUart0("Wassup-_-\r\n");
}

void reset()
{
    putsUart0("Sure to RESET the board?[Y/N]  ");
    getsUart0();
    if(isCommand("Y", 0) || isCommand("y", 0) || isCommand("YES", 0) || isCommand("yes", 0))
    {
        waitMicrosecond(1000);
        reset_hw();
    }
    else if(isCommand("N", 0) || isCommand("n", 0) || isCommand("NO", 0) || isCommand("no", 0))
    {
    }
    else
    {
        putsUart0("Invalid Command\r\n");
    }
}

void getTemperature()
{
    int16_t raw;
    char temperatureStr[10];
    float adc_read;
    raw = readADC0SS3();
    // TEMP = 147.5 - ((75 * (VREFP - VREFN) * ADC code) / 4096)
    adc_read = 147.5-(75.0*(3.3-0.0)*raw)/4096.0;

    if(adc_read > temperature_lt && adc_read < temperature_gt)
    {
        my_ftoa(adc_read, temperatureStr, 4);
        putsUart0("CPU temperature: ");
        putsUart0(temperatureStr);

        putsUart0(" C\r\n");
    }
}

void printTime()
{
    char time[10];
    uint32_t rtcc= HIB_RTCC_R;
    uint8_t hour = hh;
    uint8_t minute = mi;
    uint8_t second = ss;
//    uint8_t temp;

    // Get hours
    rtcc = rtcc % (24 * 3600);
    hour = rtcc / 3600 + hh;
    // Get minutes
    rtcc %= 3600;
    minute = rtcc / 60 + mi;
    // Get seconds
    rtcc %= 60;
    second = rtcc + ss;

    // Second offset
    if(second >= 60)
    {
        second -= 60;
        minute++;
    }
    // Minute offset
    if(minute >= 60)
    {
        minute -= 60;
        hour++;
    }
    // Hour offset
    if(hour >= 24)
    {
        hour -= 24;
        addDays(1, 't');
    }


    // Convert integers to string
    my_itoa(hour, time, 10, 2);
    time[2] = ':';
    my_itoa(minute, time+3, 10, 2);
    time[5] = ':';
    my_itoa(second, time+6, 10, 2);

    putsUart0(time);
}

void changeTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    uint32_t rtcc= HIB_RTCC_R;
    // Update mo/dd/yy
    rtcc /= 24*60*60;
    addDays(rtcc, 't');

    // Update time values
    hh = hour;
    mi = minute;
    ss = second;

    // Reload RTC with 0
    reloadRTC(0);

    putsUart0("Time updated successfully\r\n");
}

void printDate()
{
    uint32_t rtcc= HIB_RTCC_R;

    // Get dates elapse and print out the date
    rtcc /= (24 * 3600);
    addDays(rtcc, 'd');
}

void changeDate(uint8_t month, uint8_t day, uint16_t year)
{
    uint32_t rtcc= HIB_RTCC_R;
    // Ignore days
    rtcc = rtcc % (24*60*60);
    // Update seconds
    ss += rtcc;
    if(ss >= 60)
        mi += 1;
    ss %= 60;
    // Update minutes
    mi += rtcc/60;
    if(mi >= 60)
        hh +=1 ;
    mi %= 60;
    // Update hours
    hh += rtcc/3600;
    hh %= 24;

    // Update date values
    mo = month;
    dd = day;
    yy = year;

    // Reload RTC with 0
    reloadRTC(0);

    putsUart0("Date updated successfully\r\n");

}

void pollI2C()
{
    bool found = false;
    char device[5];
    int i;
    for (i = 4; i < 119; i++)
    {
        if (pollI2c0Address(i))
        {
            // First device found
            if(!found)
            {
                putsUart0("Device(s) found: ");
                found = true;
            }
            my_uint8ToHex(i, device);
            putsUart0(device);
            putsUart0(" ");
        }
    }

    // If no device found
    if(!found)
        putsUart0("No Device found\r\n");
    else
        putsUart0("\r\n");
}

// Write address register data to I2C
void writeI2C(int add, int reg, int data)
{
    // Check if integer is 8 bits, decimal 0-255
    if(add>255 || reg>255 || data>255)
    {
        putsUart0("Invalid uint8_t value\r\n");
        return;
    }

    writeI2c0Register(add, reg, data);
    putsUart0("Write successfully\r\n");
}

// Read address register for data from I2C
void readI2C(int add, int reg)
{
    // Check if integer is 8 bits, decimal 0-255
    if(add>255 || reg>255)
    {
        putsUart0("Invalid uint8_t value\r\n");
        return;
    }

    uint8_t data = readI2c0Register(add, reg);
    char dataStr[5];

    my_uint8ToHex(data, dataStr);

    putsUart0("Read value: ");
    putsUart0(dataStr);
    putsUart0("\r\n");
}

// Get Acceleration from MPU9250
void getAcc()
{
    char acc_x[10], acc_y[10], acc_z[10], sum[10];
    float acc[3];
    get_acc(acc);

    putsUart0("Acceleration: X=");
    my_ftoa(acc[0], acc_x, 2);
    putsUart0(acc_x);
    putsUart0(", Y=");
    my_ftoa(acc[1], acc_y, 2);
    putsUart0(acc_y);
    putsUart0(", Z=");
    my_ftoa(acc[2], acc_z, 2);
    putsUart0(acc_z);
    putsUart0(" Sum: ");
    my_ftoa(sqrtf(my_fpower(acc[0], 2)+my_fpower(acc[1], 2)+my_fpower(acc[2], 2)), sum, 4);
    putsUart0(sum);
    putsUart0("\r\n");
}

// Get Gyroscope values from MPU9250
void getGyro()
{
    char gyro_x[10], gyro_y[10], gyro_z[10];
    float gyro[3];
    get_gyro(gyro);

    putsUart0("Gyroscope: X=");
    my_ftoa(gyro[0], gyro_x, 2);
    putsUart0(gyro_x);
    putsUart0(", Y=");
    my_ftoa(gyro[1], gyro_y, 2);
    putsUart0(gyro_y);
    putsUart0(", Z=");
    my_ftoa(gyro[2], gyro_z, 2);
    putsUart0(gyro_z);
    putsUart0("\r\n");
}

// Get Magnetometer values from MPU9250
void getMag()
{
    char mag_x[10], mag_y[10], mag_z[10];
    float mag[3];
    get_mag(mag);

    putsUart0("Magnetometer: X=");
    my_ftoa(mag[0], mag_x, 2);
    putsUart0(mag_x);
    putsUart0(", Y=");
    my_ftoa(mag[1], mag_y, 2);
    putsUart0(mag_y);
    putsUart0(", Z=");
    my_ftoa(mag[2], mag_z, 2);
    putsUart0(mag_z);
    putsUart0("\r\n");
}

void printGating()
{
    char tem_gt[10], tem_lt[10], acc_gt[10], acc_lt[10];
    my_ftoa(temperature_gt, tem_gt, 2);
    my_ftoa(temperature_lt, tem_lt, 2);
    my_ftoa(acceleration_gt, acc_gt, 2);
    my_ftoa(acceleration_lt, acc_lt, 2);
    putsUart0("Gating Parameters   GT   |   LT\r\n");
    putsUart0("      Temperature   ");
    putsUart0(tem_gt);
    putsUart0(" | ");
    putsUart0(tem_lt);
    putsUart0("\r\n");
    putsUart0("     Acceleration   ");
    putsUart0(acc_gt);
    putsUart0(" | ");
    putsUart0(acc_lt);
    putsUart0("\r\n");
}

void changeGatingTem(float gt, float lt)
{
    temperature_gt = gt;
    temperature_lt = lt;
    putsUart0("Temperature GT|LT values changed.\r\n");
}

void changeGatingAcc(float gt, float lt)
{
    acceleration_gt = gt;
    acceleration_lt = lt;
    putsUart0("Acceleration GT|LT values changed.\r\n");
}

void periodicTemp(uint8_t t)
{
    uint32_t rtcc;
    while(1)
    {
        green_toggle();
        // Read RTCC value
        rtcc= HIB_RTCC_R;
        // Delay
        waitMicrosecond(1000);
        while(!(HIB_MIS_R & HIB_MIS_WC)){}
        rtcc += t;
        // Write time values to RTCM0
        HIB_RTCM0_R = rtcc;
        // Delay
        waitMicrosecond(1000);
        while(!(HIB_MIS_R & HIB_MIS_WC)){}
        // Wait until the RTC is matched
        while(!(HIB_MIS_R & HIB_MIS_RTCALT0)){}
        // clear interrupts
        HIB_IC_R = HIB_IC_RTCALT0;
        // Delay
        waitMicrosecond(1000);
        while(!(HIB_MIS_R & HIB_MIS_WC)){}
        getTemperature();
        if(!isUART0Empty())
        {
            getsUart0();
            if(isCommand("stop", 0) || isCommand("STOP", 0))
                break;
            else
                putsUart0("Invalid Command\r\n");
        }
    }
}

// Set/Reset the match value for periodic mode
void periodic(uint8_t t)
{
    // Read RTCC value
    uint32_t rtcc= HIB_RTCC_R;
    // Delay
    waitMicrosecond(1000);
    while(!(HIB_MIS_R & HIB_MIS_WC)){}
    rtcc += t;
    // Write time values to RTCM0
    HIB_RTCM0_R = rtcc;
    // Delay
    waitMicrosecond(1000);
    while(!(HIB_MIS_R & HIB_MIS_WC)){}
}



void printHys()
{
    char str[10];
    my_itoa(hysterisis, str, 10, 0);
    putsUart0("Hysterisis value: ");
    putsUart0(str);
    putsUart0("\r\n");
}

void setHys(int temp)
{
    hysterisis = temp;
}

void stepTriggerTemp()
{

    float acc[3];
    float sum=0;
    char step_str[10];
    while(1)
    {
        // Some time delay for hysteresis check later
        waitMicrosecond(1000);
        while(sum < acceleration_gt){
            getAcc_mpu9250(acc);
            sum = sqrtf(my_fpower(acc[0], 2)+my_fpower(acc[1], 2)+my_fpower(acc[2], 2));
            if(!isUART0Empty())
                break;
        }

        while(sum > acceleration_gt){
            getAcc_mpu9250(acc);
            sum = sqrtf(my_fpower(acc[0], 2)+my_fpower(acc[1], 2)+my_fpower(acc[2], 2));
            if(!isUART0Empty())
                break;
        }

        if(!isUART0Empty())
        {
            getsUart0();
            if(isCommand("stop", 0) || isCommand("STOP", 0))
                break;
            else
                putsUart0("Invalid Command\r\n");
        }

        stepC++;
        my_itoa(stepC, step_str, 10, 0);
        putsUart0(step_str);
        putsUart0("\r\n");
    }
}

void stepCounter()
{
    float acc[3];
    float sum=0;
    char step_str[10];
    while(1)
    {
        // Some time delay for hysteresis check later
        waitMicrosecond(1000);
        // Setup time for to enable sleep when RTC match
        if(sleep_mode)
            periodic(10);

        while(sum < acceleration_gt && !(HIB_MIS_R & HIB_MIS_RTCALT0)){
            getAcc_mpu9250(acc);
            sum = sqrtf(my_fpower(acc[0], 2)+my_fpower(acc[1], 2)+my_fpower(acc[2], 2));
            if(!isUART0Empty())
                break;
        }

        while(sum > acceleration_gt && !(HIB_MIS_R & HIB_MIS_RTCALT0)){
            getAcc_mpu9250(acc);
            sum = sqrtf(my_fpower(acc[0], 2)+my_fpower(acc[1], 2)+my_fpower(acc[2], 2));
            if(!isUART0Empty())
                break;
        }

        // RTC match the sleep time
        if(HIB_MIS_R & HIB_MIS_RTCALT0)
        {
            // clear the RTC Match interrupt
            HIB_IC_R = HIB_IM_RTCALT0;
            waitMicrosecond(1000);
            while(!(HIB_MIS_R & HIB_MIS_WC)){}
            // Interrupt enable GPIO port F - 30
            setMPU9250WOM();
            waitMicrosecond(1000);
            HIB_IC_R = HIB_IM_RTCALT0;
            NVIC_EN0_R |= 1 << (INT_GPIOF-16); // turn-on interrupt (GPIOF)
            sleep();
            NVIC_DIS0_R = (1 << (INT_GPIOF-16)); // turn-off interrupt (GPIOF)
            init_mpu9250();

        }

        // Check to stop
        if(!isUART0Empty())
        {
            getsUart0();
            if(isCommand("stop", 0) || isCommand("STOP", 0))
                break;
            else
                putsUart0("Invalid Command\r\n");
        }

        stepC++;
        my_itoa(stepC, step_str, 10, 0);
        putsUart0(step_str);
        putsUart0("\r\n");
    }
}

void stepCounter2(int sample)
{
    float acc[3];
    float sum=0;
    char step_str[10];
    while(sample)
    {
        // Some time delay for hysteresis check later
        waitMicrosecond(1000);
        // Setup time for to enable sleep when RTC match
        if(sleep_mode)
            periodic(10);

        while(sum < acceleration_gt && !(HIB_MIS_R & HIB_MIS_RTCALT0)){
            getAcc_mpu9250(acc);
            sum = sqrtf(my_fpower(acc[0], 2)+my_fpower(acc[1], 2)+my_fpower(acc[2], 2));
            if(!isUART0Empty())
                break;
        }

        while(sum > acceleration_gt && !(HIB_MIS_R & HIB_MIS_RTCALT0)){
            getAcc_mpu9250(acc);
            sum = sqrtf(my_fpower(acc[0], 2)+my_fpower(acc[1], 2)+my_fpower(acc[2], 2));
            if(!isUART0Empty())
                break;
        }

        // RTC match the sleep time
        if(HIB_MIS_R & HIB_MIS_RTCALT0)
        {
            // clear the RTC Match interrupt
            HIB_IC_R = HIB_IM_RTCALT0;
            waitMicrosecond(1000);
            while(!(HIB_MIS_R & HIB_MIS_WC)){}
            // Interrupt enable GPIO port F - 30
            setMPU9250WOM();
            waitMicrosecond(1000);
            HIB_IC_R = HIB_IM_RTCALT0;
            NVIC_EN0_R |= 1 << (INT_GPIOF-16); // turn-on interrupt (GPIOF)
            sleep();
            NVIC_DIS0_R = (1 << (INT_GPIOF-16)); // turn-off interrupt (GPIOF)
            init_mpu9250();

        }

        // Check to stop
        if(!isUART0Empty())
        {
            getsUart0();
            if(isCommand("stop", 0) || isCommand("STOP", 0))
                break;
            else
                putsUart0("Invalid Command\r\n");
        }

        stepC++;
        sample--;
        my_itoa(stepC, step_str, 10, 0);
        putsUart0(step_str);
        putsUart0("\r\n");
    }
}


void sleepCmd(bool on)
{
    if(on)
    {
        sleep_mode = true;
        putsUart0("Sleep mode is turned on\r\n");
    }
    else
    {
        sleep_mode = false;
        putsUart0("Sleep mode is turned off\r\n");
    }

}



uint32_t time_values()
{
    uint32_t rtcc= HIB_RTCC_R;
    uint8_t hour = hh;          //
    uint8_t minute = mi;
    uint8_t second = ss;
    uint32_t timeVal;
//    uint8_t temp;

    // Get hours
    rtcc = rtcc % (24 * 3600);
    hour = rtcc / 3600 + hh;
    // Get minutes
    rtcc %= 3600;
    minute = rtcc / 60 + mi;
    // Get seconds
    rtcc %= 60;
    second = rtcc + ss;

    // Second offset
    if(second >= 60)
    {
        second -= 60;
        minute++;
    }
    // Minute offset
    if(minute >= 60)
    {
        minute -= 60;
        hour++;
    }
    // Hour offset
    if(hour >= 24)
    {
        hour -= 24;
        addDays(1, 't');
    }


    // Convert time values to a uint32 value
    timeVal = hour<<12 | minute<<6 | second;
    return timeVal;
}

uint32_t temperature_v()
{
    float adc_read;
    int int_part;
    int float_part;
    int16_t raw;
    uint32_t tempVal = 0x00000000;

    raw = readADC0SS3();
    // TEMP = 147.5 - ((75 * (VREFP - VREFN) * ADC code) / 4096)
    adc_read = 147.5-(75.0*(3.3-0.0)*raw)/4096.0;

    int_part = (int)adc_read;
    float_part = (int) ((adc_read-int_part)*10000); // four digits of float number after '.'

    // If below 0
    if(int_part<0)
    {
        int_part = int_part* (-1);
        tempVal = 0x80000000; // Write 1 bit at the very beginning
    }

    tempVal |= int_part<<14 | float_part;
    return tempVal;
}



void writeTimeData(unsigned int time, unsigned int sample)
{
    uint32_t rtcc;
    uint32_t time_val;
    uint32_t temp_val;
    unsigned int s_count = sample;



    while(s_count)
   {
       // Read RTCC value
       rtcc= HIB_RTCC_R;
       // Delay
       waitMicrosecond(1000);
       while(!(HIB_MIS_R & HIB_MIS_WC)){}
       rtcc += time;
       // Write time values to RTCM0
       HIB_RTCM0_R = rtcc;
       // Delay
       waitMicrosecond(1000);
       while(!(HIB_MIS_R & HIB_MIS_WC)){}
       // Wait until the RTC is matched
       while(!(HIB_MIS_R & HIB_MIS_RTCALT0)){}
       // clear interrupts
       HIB_IC_R = HIB_IC_RTCALT0;
       // Delay
       waitMicrosecond(1000);
       while(!(HIB_MIS_R & HIB_MIS_WC)){}

       time_val = time_values();
       writeData(time_val);
       temp_val = temperature_v();
       writeData(temp_val);

       // Update sample count
       s_count--;
   }

    putsUart0("Logging completed\r\n");
}


// Periodic time with count and log and hysterisis
void periodicCL(int time, int count)
{
    int int_part;
    int float_part;
    int16_t raw;


    char temperatureStr[10];
    float adc_read;

    uint32_t rtcc;
    bool triger = 0;
    float adc_read_origin;
    uint32_t time_val;
    uint32_t tempVal;
    putsUart0("CPU temperature: \r\n");
    while(count)
    {
        tempVal = 0x00000000;
        green_toggle();
        // Read RTCC value
        rtcc= HIB_RTCC_R;
        // Delay
        waitMicrosecond(1000);
        while(!(HIB_MIS_R & HIB_MIS_WC)){}
        rtcc += time;
        // Write time values to RTCM0
        HIB_RTCM0_R = rtcc;
        // Delay
        waitMicrosecond(1000);
        while(!(HIB_MIS_R & HIB_MIS_WC)){}
        // Wait until the RTC is matched
        while(!(HIB_MIS_R & HIB_MIS_RTCALT0)){}
        // clear interrupts
        HIB_IC_R = HIB_IC_RTCALT0;
        // Delay
        waitMicrosecond(1000);
        while(!(HIB_MIS_R & HIB_MIS_WC)){}

         raw = readADC0SS3();
         // TEMP = 147.5 - ((75 * (VREFP - VREFN) * ADC code) / 4096)
         adc_read = 147.5-(75.0*(3.3-0.0)*raw)/4096.0;
         if(triger==0)
         {
            triger = 1;
            adc_read_origin = adc_read;
            my_ftoa(adc_read, temperatureStr, 4);

            printTime();
            putsUart0(" ");
            putsUart0(temperatureStr);
            putsUart0(" C\r\n");


            count--;


            time_val = time_values();
            writeData(time_val);

            int_part = (int)adc_read;
            float_part = (int) ((adc_read-int_part)*10000); // four digits of float number after '.'

            // If below 0
            if(int_part<0)
            {
                int_part = int_part* (-1);
                tempVal = 0x80000000; // Write 1 bit at the very beginning
            }

            tempVal |= int_part<<14 | float_part;


            writeData(tempVal);
         }
         else
         {
            if((adc_read - adc_read_origin)*(adc_read - adc_read_origin) >= hysterisis*hysterisis)
            {
                my_ftoa(adc_read, temperatureStr, 4);

                printTime();
                putsUart0(" ");
                putsUart0(temperatureStr);
                adc_read_origin = adc_read;
                putsUart0(" C\r\n");
                count--;

                int_part = (int)adc_read;
                float_part = (int) ((adc_read-int_part)*10000); // four digits of float number after '.'

                // If below 0
                if(int_part<0)
                {
                    int_part = int_part* (-1);
                    tempVal = 0x80000000; // Write 1 bit at the very beginning
                }

                tempVal |= int_part<<14 | float_part;

                time_val = time_values();
                writeData(time_val);
                writeData(tempVal);
             }
         }

        if(!isUART0Empty())
        {
            getsUart0();
            if(isCommand("stop", 0) || isCommand("STOP", 0))
                break;
            else
                putsUart0("Invalid Command\r\n");
        }
    }

}


uint32_t acc_time_values()
{
    uint32_t acc_time;
    uint32_t acc_watermark = 0x70000000; // Add 0111 bit at very first of the time
    acc_time = time_values();
    acc_time |= acc_watermark;

    return acc_time;
}


void gatingDemo()
{
    // Initial values
    char acc_x[10], acc_y[10], acc_z[10], sum_str[10];
    float acc[3];
    float sum = 0.0;
    uint32_t timeVal;
    int int_part;
    int float_part;
    int i;
    uint32_t logAcc = 0x00000000;

    // Get acceleration values and calculate the sum
    get_acc(acc);
    sum = sqrtf(my_fpower(acc[0], 2)+my_fpower(acc[1], 2)+my_fpower(acc[2], 2));

    // Loop to get values
    while(1)
    {
        // Gating values checking
        while(sum < acceleration_gt)
        {
            // Get acceleration values
            get_acc(acc);
            sum = sqrtf(my_fpower(acc[0], 2)+my_fpower(acc[1], 2)+my_fpower(acc[2], 2));


            if(!isUART0Empty())
                break;
        }

        // One second delay to filter out unnecessary  values
        waitMicrosecond(1000000);

        // Check to stop
        if(!isUART0Empty())
        {
            getsUart0();
            if(isCommand("stop", 0) || isCommand("STOP", 0))
                break;
            else
            {
                putsUart0("Invalid Command\r\n");
                continue;
            }
        }

        // Print out the acceleration values
        putsUart0("Acceleration: X=");
        my_ftoa(acc[0], acc_x, 2);
        putsUart0(acc_x);
        putsUart0(", Y=");
        my_ftoa(acc[1], acc_y, 2);
        putsUart0(acc_y);
        putsUart0(", Z=");
        my_ftoa(acc[2], acc_z, 2);
        putsUart0(acc_z);
        putsUart0(" Sum: ");
        my_ftoa(sum, sum_str, 4);
        putsUart0(sum_str);
        putsUart0("\r\n");

        // Log the time and accelerations
        timeVal = acc_time_values();
        writeData(timeVal);

        for(i = 0; i < 3; i++)
        {
            // If below 0
            if(acc[i]<0)
            {
                int_part = int_part* (-1);
                logAcc = 0x80000000; // Write 1 bit at the very beginning
            }

            int_part = (int)acc[i];
            if(int_part==0)
            {
                float_part = (int) (acc[i]*10000);// four digits of float number after '.'
            }
            else
                float_part = (int) ((acc[i]-int_part)*10000); // four digits of float number after '.'

            if(int_part == 0)
                logAcc |= float_part;
            else
                logAcc |= int_part<<14 | float_part;
            writeData(logAcc);
            // Somedelay for data writing
            waitMicrosecond(10000);
            // Clean logAcc
            logAcc = 0x00000000;

        }
        // Clean logAcc
        logAcc = 0x00000000;


        // Re-read the acceleration
        get_acc(acc);
        sum = sqrtf(my_fpower(acc[0], 2)+my_fpower(acc[1], 2)+my_fpower(acc[2], 2));



    }

}
