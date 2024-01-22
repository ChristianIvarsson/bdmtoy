#include <stdio.h>
#include <stdint.h>
#include "str_tools.h"

uint32_t countChars(const char *in) {
    uint32_t count = 0;
    while (*in++) ;
    return count;
}

// strstr sucks donkey's ballz...
char *isMatch(char *unknown, const char *template)
{
    char *tmp = unknown;
    uint32_t templen = 0;

    while ( template[templen] )
        templen++;

    while ( (*unknown) && (*template) && (*unknown++ == *template++) )
        templen--;

    if ( ((*unknown) && (*unknown != 0x20)) || (templen) )
        return 0;

    return tmp;
}

void toLower(char *ptr)
{
    while ( *ptr )
    {
        if ( (*ptr > 0x40) && (*ptr < 0x5B) )
            *ptr += 0x20;
        ptr++;
    }
}

void toUpper(char *ptr)
{
    while ( *ptr )
    {
        if ( (*ptr > 0x60) && (*ptr < 0x7B) )
            *ptr -= 0x20;
        ptr++;
    }
}

uint32_t stripLeadingSpaces(char *array, uint32_t origLen)
{
    uint32_t i;

    // Space! Strip it
    while ( array[0] == 0x20 )
    {
        for (i = 0; i < origLen; i++)
            array[i] = array[i+1];

        origLen--;
    }

    return origLen;
}

uint32_t stripForbidden(char *array, uint32_t origLen)
{
    uint32_t i, currPos = 0;

    while (currPos < origLen)
    {
        // Turn TABs into spaces (I f*cking hate them..)
        if ( array[currPos] == 0x09 )
            array[currPos] = 0x20;

        // Strip illegal characters, comments etc..
        else if ( (array[currPos] < 0x20) || (array[currPos] > 0x7E) ||
                ((array[currPos] == 0x2F) && (array[currPos+1] == 0x2F)) )
            break;

        currPos++;
    }

    // Clear junk
    for (i = currPos; i < origLen; i++)
        array[i] = 0x00;

    return currPos;
}

////////////////////////////////////////////////////////////////////////////
// Pretty much only used when reporting errors since it destroys the string
char *truncateString(char *ptr)
{
    char    *ret       = ptr;
    uint32_t triggered = 0;

    while ( *ptr )
    {
        if ( *ptr == 0x20 )
            triggered = 1;
        if ( triggered )
            *ptr = 0x00;
        ptr++;
    }
    return ret;
}

// Fix this mess..
char *findNext(char *ptr)
{
    ptr++;
    while ( (*ptr) && (ptr[-1] != 0x20) )
        ptr++;

    return (*ptr) ? ptr : 0;
}

////////////////////////////////////////////////////
// Strip leading crap, f*cking tabs, comments etc..
uint32_t formatString(char *array, uint32_t maxLen)
{
    uint32_t stripped, charsleft, i;
    uint32_t origLen = 0;
    uint32_t currPos = 0;

    // Figure out length
    while ( (array[origLen++]) && (origLen < (maxLen-1)) )  ;

    // Precaution, make sure string really is terminated
    array[maxLen-1] = 0x00;

    // Strip leading spaces / forbidden characters
    origLen = stripLeadingSpaces(array, stripForbidden(array, origLen));

    // We've looked for leading spaces and regular comments but there's also pound comments..
    if ( *array == 0x23 )
    {
        for (i = 0; i < origLen; i++)
            array[i] = 0x00;
        origLen = 0;
    }

    while ( array[currPos] && currPos < origLen )
    {
        if ( array[currPos] == 0x20 )
        {
            charsleft = origLen - (currPos + 1);
            stripped  = stripLeadingSpaces(&array[currPos + 1], charsleft);
            origLen  -= (charsleft - stripped);
        }

        currPos++;
    }

    // Strip trailing space if present
    if ( origLen && array[origLen - 1] == 0x20 )
        array[--origLen] = 0x00;

    return origLen;
}

//////////////////////////////////////////////
// Tools for retrieving numbers from a string
char *isHexNumber(char *in)
{
    char *tmp;

    if ( !*in || !in )
        return 0;

    while ( (*in == 0x23) || (*in == 0x24) ) // # or $
        in++;

    // Only checking for x is dangerous when dealing with registers..
    /* if ( (*in == 0x78) || (*in == 0x58) ) // x X
        in++;
    else*/ if ( (*in == 0x30) && ((in[1] == 0x78) || (in[1] == 0x58)) ) // 0x 0X
        in+=2;

    tmp = in;

    while ( (*in) && (*in != 0x20) ) // Look for terminating zero and space
    {
        if ( ((*in > 0x2F) && (*in < 0x3A)) || // 0 - 9
             ((*in > 0x60) && (*in < 0x67)) || // a - f
             ((*in > 0x40) && (*in < 0x47))  ) // A - F
            in++;
        else if (*in != 0x20)
            return 0;
    }

    return tmp;
}

char *isDecNumber(char *in)
{
    char *tmp = in;
    
    if ( !*in || !in )
        return 0;

    while ( (*in) && (*in != 0x20) ) // Look for terminating zero and space
    {
        if ( ((*in > 0x2F) && (*in < 0x3A)) )// 0 - 9
            in++;
        else if (*in != 0x20)
            return 0;
    }

    return tmp;
}

uint32_t toHexNumber(char *in)
{
    uint32_t tmp = 0;

    while ( (*in) && (*in != 0x20) )
    {
        tmp <<= 4;

        if ( ((*in > 0x2F) && (*in < 0x3A)) )     // 0 - 9
            tmp |= ((*in - 0x30)&0xF);
        else if ( (*in > 0x60) && (*in < 0x67) )  // a - f
            tmp |= ((*in - 0x57)&0xF);
        else if ( (*in > 0x40) && (*in < 0x47) )  // A - F
            tmp |= ((*in - 0x37)&0xF);

        in++;
    }

    return tmp;
}

uint32_t toDecNumber(char *in)
{
    uint32_t tmp = 0;

    while ( (*in) && (*in != 0x20) )
    {
        tmp *= 10;

        if ( ((*in > 0x2F) && (*in < 0x3A)) )     // 0 - 9
            tmp += ((*in - 0x30)&0xF);

        in++;
    }

    return tmp;
}