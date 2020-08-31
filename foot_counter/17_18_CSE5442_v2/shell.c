// 17 Youngtak Cho
// 18 Jay Denton
// CSE 5442 Embedded Systems II

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "uart0.h"
#include "i2c0.h"
#include "shell.h"
#include "commands.h"
#include "flash.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
#define MAX_CHARS 50
#define MAX_ARGS 5

// Structure of variable
struct var
{
    char str[MAX_CHARS+1];
    int pos[MAX_ARGS];
    int argCount;
}vars;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
// String length function. Ref:geeksforgeeks.org
int stringlen(char* str)
{
    int len = 0;
    int i;
    for (i=0; str[i] != 0; i++)
        len++;
    return(len);
}

// Sub String length, return length of the first string until the space or the end
int substrlen(char* str)
{
    int len = 0;
    int i;
    for (i=0; str[i] != 0 && str[i] != 32; i++)
        len++;
    return(len);
}

// Check if the character is a digit number. Ref:geeksforgeeks.org
bool isDigit(char n)
{
    if(n >= 48 && n <= 57)
        return true;
    return false;
}

// Check if the string is a decimal number. Ref:geeksforgeeks.org
bool isNumber(int pos)
{
    int i;
    for (i = 0; i < substrlen(vars.str+pos); i++)
    {
        if (!isDigit(vars.str[pos+i]))
        {
//            if (vars.str[pos+i]=='-' && i ==0)
//                continue;
//            else
                return false;
        }
    }

    return true;
}

// Check if the string is a decimal number. Ref:geeksforgeeks.org
bool isFloat(int pos)
{
    int i;
    bool point = false;
    for (i = 0; i < substrlen(vars.str+pos); i++)
    {
        if (isDigit(vars.str[pos+i]) == false)
        {
            if (vars.str[pos+i]=='-' && i ==0)
                continue;
            else if(vars.str[pos+i]=='.' && !point)
                point = true;
            else
                return false;
        }
    }

    return true;
}


// Check if the character is a heximal number.
bool isHeximal(char n)
{
    if(n >= 48 && n <= 57)
        return true;
    else if(n >=65 && n <= 70)
        return true;
    else if(n >= 97 && n <=102)
        return true;
    return false;
}

// Check if the substring at vars.str[pos] is valid hex
bool isHex(int pos)
{
    int i;

    // The hex value has to be at least 3 chars long
    if(substrlen(vars.str+pos)<3)
        return false;

    // Allow '0x' or '0X' at the beginning of the hex value
    if(vars.str[pos]!='0')
        return false;
    if(vars.str[pos+1]!='x')
        if(vars.str[pos+1]!='X')
            return false;
    if(vars.str[pos+1]!='X')
        if(vars.str[pos+1]!='x')
            return false;

    for (i = 0; i < substrlen(vars.str+pos+2); i++)
        if (isHeximal(vars.str[pos+i+2]) == false)
            return false;

    return true;
}

// A simple atoi() function. Only intgers greater than 0. Ref: www.tutorialspoint.com
int my_atoi(int pos)
{
    int i;
    int res = 0; // Initialize result

    // Iterate through characters and update result
    for (i = 0; i<substrlen(vars.str+pos); ++i)
        res = res * 10 + vars.str[pos+i] - 48;

    // return result.
    return res;
}

// Check for first substring after command in vars position. Ref: www.tutorialspoint.com
bool isSubstring(char* str, int pos)
{
        int len = stringlen(str);
        int i;

        // Correct command
        for(i = 0; i < stringlen(str) && i < stringlen(vars.str); i++)
        {
            if(str[i] != vars.str[i+pos])
                return false;
        }
        // Shorter than command
        if(i < stringlen(str))
            return false;

        // Longer than command
        if(stringlen(vars.str) > i+pos && vars.str[i+pos] != 32 )
            return false;

        return true;
}

// Count number of digits in the decimal number
int digitCount(int num)
{
    int digitC = 0;
    if(num == 0)
        return 1;
    while(num!=0)
    {
        digitC++;
        num /= 10;
    }
    return digitC;
}

// Function to reverse a string. Ref: www.tutorialspoint.com
void my_reverse(char str[], int len)
{
    int start, end;
    char temp;
    for(start=0, end=len-1; start < end; start++, end--) {
        temp = *(str+start);
        *(str+start) = *(str+end);
        *(str+end) = temp;
    }
}

// Function to convert integer to string.  Ref: www.tutorialspoint.com
void my_itoa(int num, char* str, int base, int digitNum)
{
    int i = 0;
    int j;
    int offset = digitNum - digitCount(num);
    bool isNegative = false;

    /* A zero is same "0" string in all base */
    if (num == 0) {
        str[i++] = '0';
        if(offset==0)
        {
            str[i] = '\0';
            return;
        }
    }

    /* negative numbers are only handled if base is 10
       otherwise considered unsigned number */
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'A' : rem + '0';
        num = num/base;
    }

    // Add '0' for digit number offset
    for(j = 0; j < offset; j++)
        str[i++] = '0';

    /* Append negative sign for negative numbers */
    if (isNegative){
        str[i++] = '-';
    }

    str[i] = '\0';

    my_reverse(str, i);
}
// Function calculates the power in math. Ref: edureka.co
int my_power(int base, int exponent)
{
    int i;
    int result=1;
    for (i = exponent; i>0; i--)
    {
        result = result * base;
    }
    return result;
}
// Function calculated the power of floats in math
float my_fpower(float base, int exponent)
{
    int i;
    float result=1.0;
    for (i = exponent; i>0; i--)
    {
        result = result * base;
    }
    return result;
}

// Function to convert a floating-point/double number to a string. Ref:geeksforgeeks.org
void my_ftoa(float n, char* res, int afterpoint)
{
    int i;

    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;
    if(fpart < 0)
        fpart = fpart*(-1.0);

    // convert integer part to string
    my_itoa(ipart, res, 10, digitCount(ipart));
    i = stringlen(res);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.'; // add dot

        // Get the value of fraction part upto given number of points after dot.
        fpart = fpart * my_power(10, afterpoint);
        my_itoa((int)fpart, res + i + 1, 10, afterpoint);
    }
}

// Convert heximal string to integer. Ref:geeksforgeeks.org
int my_asciiToUint(int pos)
{
    // Initializing base value to 1, i.e 16^0
    int base = 1;
    int len = substrlen(vars.str+pos);
    int uint = 0;
    int i ;

    // Extracting characters as digits from last character
    // Skip '0x' or '0X' at the beginning
    for (i=len-1; i>=2; i--)
    {
        // if character lies in '0'-'9', converting
        // it to integral 0-9 by subtracting 48 from
        // ASCII value.
        if (vars.str[pos+i]>='0' && vars.str[pos+i]<='9')
        {
            uint += (vars.str[pos+i] - 48)*base;

            // incrementing base by power
            base = base * 16;
        }

        // if character lies in 'A'-'F' , converting
        // it to integral 10 - 15 by subtracting 55
        // from ASCII value
        else if (vars.str[pos+i]>='A' && vars.str[pos+i]<='F')
        {
            uint += (vars.str[pos+i] - 55)*base;

            // incrementing base by power
            base = base*16;
        }

        // if character lies in 'a'-'f' , converting
        // it to integral 10 - 15 by subtracting 55
        // from ASCII value
        else if (vars.str[pos+i]>='a' && vars.str[pos+i]<='f')
        {
            uint += (vars.str[pos+i] - 87)*base;

            // incrementing base by power
            base = base*16;
        }
    }

    return uint;
}

// Convert uint8_t value to heximal string
// Ref: geeksforgeeks.org
void my_uint8ToHex(uint8_t u8, char* hex)
{
    // counter for hexadecimal number array
    int i = 0;
    int temp;

    while(u8!=0)
    {
        // temporary variable to store remainder
        temp = 0;

        // storing remainder in temp variable.
        temp = u8 % 16;

        // check if temp < 10
        if(temp < 10)
        {
            hex[i] = temp + 48;
            i++;
        }
        else
        {
            hex[i] = temp + 55;
            i++;
        }

        u8 = u8/16;
    }

    // '00' for zero value
    if(i==0)
    {
        hex[i++] = '0';
        hex[i++] = '0';
    }
    // Add extra '0' for 4 bit numbers
    else if(i==1)
        hex[i++]='0';

    // Add '0x' at beginning
    hex[i++] = 'x';
    hex[i++] = '0';
    hex[i] = '\0';

    my_reverse(hex, 4);
}

// Blocking function that gets a string when the MAX_CHARS and MAX_ARGS is not reached or ENTER key pressed
void getsUart0()
{
    int i;
    char c;
    vars.argCount = 0;

    for(i = 0; i < MAX_CHARS; i++)
    {
        // Get the entered character
        c = getcUart0();

        // Backspace/Delete key
        if(c == 8 || c == 127)
        {
            // Not at the first index, left shift two positions
            if(i > 0)
            {
                // If delete an element rather than space
                if(vars.str[i-1] != 32)
                {
                    // If the element is at the beginning of an argument
                    if(i >= 2 && vars.str[i-2] == 32)
                        vars.argCount--;
                    // If the element is at first position of string
                    else if(i == 1)
                        vars.argCount--;
                }
                //putsUart0("[BS]");
                putcUart0(c);
                i=i-2;
            }
            else if(i == 0)
                i--;
        }
        // Enter key/New line
        else if(c==13 || c==10)
        {
            putsUart0("\r\n");
            break;
        }
        // Space or above
        else if(c >= 32)
        {
            putcUart0(c);
            vars.str[i] = c;

            // Check for arguments at first index
            if(i == 0 && c != 32)
            {
                vars.pos[vars.argCount] = 0;
                vars.argCount++;
            }
            // Check for previous position is space
            else if(c != 32 && vars.str[i-1] == 32)
            {
                vars.pos[vars.argCount] = i;
                vars.argCount++;
            }
            // Check for MAX_ARGS reached
            else if(c == 32 && vars.argCount == MAX_ARGS)
            {
                putsUart0("[MAX_ARGS]\r\n");
                break;
            }
        }
        // Other circumstances
        else
        {
            putsUart0("[N/A]");
            i--;
        }
    }

    // If maxed out by characters
    if(i == MAX_CHARS)
        putsUart0("[MAX_CHARS]\r\n");

    // Close the string input
    vars.str[i] = 0;
}

// Command check function (space character auto-ignore)
bool isCommand(char* str, int argument)
{
    int len = stringlen(str);
    int i;
    int startPos;

    // check amount of argument
    if(argument != vars.argCount - 1)
    {
        //putsUart0("Wrong argument amount.\r\n");
        return false;
    }

    // Valid command
    startPos = vars.pos[0];
    for(i = 0; i < stringlen(str) && i < stringlen(vars.str); i++)
    {
        if(str[i] != vars.str[i+startPos])
            return false;
    }

    // Shorter than command
    if(i < stringlen(str))
        return false;

    // Longer than command
    if(stringlen(vars.str) > i+startPos && vars.str[i+startPos] != 32 )
        return false;



    return true;
}

// Shell function
void shell(void)
{
    while(1)
    {
        getsUart0();

        if(isCommand("hello", 1))
        {
            if(isSubstring("world", vars.pos[1]))
                hello();
            else
                putsUart0("Invalid hello\r\n");
        }
        else if(isCommand("reset", 0))
        {
            reset();
        }
        else if(isCommand("temperature", 0))
        {
            getTemperature();
        }
        else if(isCommand("time", 0))
        {
            printTime();
            putsUart0("\r\n");
        }
        else if(isCommand("time", 3))
        {
            int h,m,s;
            if(isNumber(vars.pos[1]))
                h = my_atoi(vars.pos[1]);
            else
            {
                putsUart0("Invalid time value\r\n>");
                continue;
            }
            if(isNumber(vars.pos[2]))
                m = my_atoi(vars.pos[2]);
            else
            {
                putsUart0("Invalid time value\r\n>");
                continue;
            }
            if(isNumber(vars.pos[3]))
                s = my_atoi(vars.pos[3]);
            else
            {
                putsUart0("Invalid time value\r\n>");
                continue;
            }
            if(h >= 0 && h < 24 && m >= 0 && m < 60 && s >= 0 && s < 60)
                changeTime(h,m,s);
            else
                putsUart0("Invalid time value\r\n");
        }
        else if(isCommand("date", 0))
        {
            printDate();
            putsUart0("\r\n");
        }
        else if(isCommand("date", 3))
        {
            int m,d,y;
            if(isNumber(vars.pos[1]))
                m = my_atoi(vars.pos[1]);
            else
            {
                putsUart0("Invalid date value\r\n>");
                continue;
            }
            if(isNumber(vars.pos[2]))
                d = my_atoi(vars.pos[2]);
            else
            {
                putsUart0("Invalid date value\r\n>");
                continue;
            }
            if(isNumber(vars.pos[3]))
                y = my_atoi(vars.pos[3]);
            else
            {
                putsUart0("Invalid date value\r\n>");
                continue;
            }
            if(m > 0 && m <= 12 && d > 0 && m <=31  && y >= 0)
            {
                if(validDate(d, m, y))
                    changeDate(m, d, y);
                else
                {
                    putsUart0("Invalid date value\r\n>");
                    continue;
                }
            }
            else
                putsUart0("Invalid date value\r\n");
        }
        else if(isCommand("timedate", 0))
        {
            printTime();
            putsUart0("  ");
            printDate();
            putsUart0("\r\n");
        }
        else if(isCommand("poll", 0))
        {
            pollI2C();
        }
        else if(isCommand("write", 3))
        {
            int add,reg,data;
            if(isHex(vars.pos[1]))
                add = my_asciiToUint(vars.pos[1]);
            else
            {
                putsUart0("Invalid HEX value\r\n>");
                continue;
            }
            if(isHex(vars.pos[2]))
                reg = my_asciiToUint(vars.pos[2]);
            else
            {
                putsUart0("Invalid HEX value\r\n>");
                continue;
            }
            if(isHex(vars.pos[3]))
                data = my_asciiToUint(vars.pos[3]);
            else
            {
                putsUart0("Invalid HEX value\r\n>");
                continue;
            }
            writeI2C(add, reg, data);
        }
        else if(isCommand("read", 2))
        {
            int add,reg;
            if(isHex(vars.pos[1]))
                add = my_asciiToUint(vars.pos[1]);
            else
            {
                putsUart0("Invalid HEX value\r\n>");
                continue;
            }
            if(isHex(vars.pos[2]))
                reg = my_asciiToUint(vars.pos[2]);
            else
            {
                putsUart0("Invalid HEX value\r\n>");
                continue;
            }
            readI2C(add, reg);
        }
        else if(isCommand("getAcc",0))
        {
            getAcc();
        }
        else if(isCommand("getGyro", 0))
        {
            getGyro();
        }
        else if(isCommand("getMag", 0))
        {
            getMag();
        }
        else if(isCommand("gating",0))
        {
            printGating();
        }
        else if(isCommand("gating", 3))
        {
            float gt, lt;
            char gt_str[MAX_CHARS], lt_str[MAX_CHARS];
            if(isSubstring("temperature", vars.pos[1]) || isSubstring("acceleration", vars.pos[1]))
            {
                if(isNumber(vars.pos[2]) || isFloat(vars.pos[2]))
                {
                    strncpy(gt_str, vars.str+vars.pos[2], substrlen(vars.str+vars.pos[2]));
                    gt = atof(gt_str);
                }
                else
                {
                    putsUart0("Invalid values\r\n>");
                    continue;
                }
                if(isNumber(vars.pos[3]) || isFloat(vars.pos[3]))
                {
                    strncpy(lt_str, vars.str+vars.pos[3], substrlen(vars.str+vars.pos[3]));
                    lt = atof(lt_str);
                }
                else
                {
                    putsUart0("Invalid values\r\n>");
                    continue;
                }
                if(gt <= lt)
                {
                    putsUart0("Invalid GT|LS values\r\n>");
                    continue;
                }
                else
                {
                    if(isSubstring("temperature", vars.pos[1]))
                        changeGatingTem(gt, lt);
                    else
                        changeGatingAcc(gt, lt);
                }
            }
            else
                putsUart0("Invalid gating\r\n");
        }
        else if(isCommand("gatingDemo",0))
        {
            gatingDemo();
        }
        else if(isCommand("periodic", 1))
        {
            int t;
            if(isNumber(vars.pos[1]))
            {
                t = my_atoi(vars.pos[1]);
                periodicTemp(t);
            }
            else
                putsUart0("Invalid periodic time\r\n");
        }
        else if(isCommand("periodic", 2))
        {
            int time;
            int count;
            if(isNumber(vars.pos[1]))
            {
                time = my_atoi(vars.pos[1]);
                if(isNumber(vars.pos[2]))
                {
                    count = my_atoi(vars.pos[2]);
                    periodicCL(time, count);
                }
            }
            else
                putsUart0("Invalid periodic setting\r\n");
        }
        else if(isCommand("hysterisis", 0))
        {
            printHys();
        }
        else if(isCommand("hysterisis", 1))
        {
            int temp;
            if(isFloat(vars.pos[1]))
            {
                temp = my_atoi(vars.pos[1]);
                setHys(temp);
            }
        }
        else if(isCommand("trigger", 0))
        {
            stepTriggerTemp();
        }
        else if(isCommand("steps",0))
        {
            stepCounter();
        }
        else if(isCommand("steps",1))
        {
            int sample;
            if(isNumber(vars.pos[1]))
            {
                sample = my_atoi(vars.pos[1]);
                stepCounter2(sample);
            }
            else
                putsUart0("Invalid step count\r\n");

        }
        else if(isCommand("sleep", 1))
        {
            if(isSubstring("on", vars.pos[1]))
                sleepCmd(1);
            else if(isSubstring("off", vars.pos[1]))
                sleepCmd(0);
            else
                putsUart0("Invalid sleep mode\r\n");
        }
        else if(isCommand("logTemp", 2))
        {
            unsigned int time;
            unsigned int sample;
            if(isNumber(vars.pos[1]))
            {
                time = my_atoi(vars.pos[1]);
            }
            else
                putsUart0("Invalid log\r\n");
            if(isNumber(vars.pos[2]))
            {
                sample = my_atoi(vars.pos[2]);
                writeTimeData(time, sample);
            }
            else
                putsUart0("Invalid log\r\n");
        }
        else if(isCommand("sample", 1))
        {
           unsigned int sample;
           if(isNumber(vars.pos[1]))
           {
               sample = my_atoi(vars.pos[1]);
               sample = sample * 2;
               readData(sample);
           }
           else
               putsUart0("Invalid read of samples\r\n");
        }
        // Enter is pressed, do nothing
        else if(stringlen(vars.str) == 0)
        {
        }
        else
        {
            putsUart0("Invalid Command\r\n");
        }

        putcUart0('>');
    }
}
