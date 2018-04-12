#include <msp430.h>
#include <stdio.h>
#include "driverlib.h"

long lr(long *addr)
{
    return __data20_read_long((unsigned long int)addr);
}

void lw(long *addr, long i)
{
    __data20_write_short((unsigned long int)addr, i);
}

int ir(int *addr)
{
    int temp = __data20_read_short((unsigned long int)addr);
    int *i = (int *)&temp;
    return *i;
}

void iw(int *addr, int f)
{
    int *i = (int *)&f;
    __data20_write_short((unsigned long int)addr, *i);
}

float fr(float *addr)
{
    unsigned long temp = __data20_read_long((unsigned long int)addr);
    float *f = (float *)&temp;
    return *f;
}

void fw(float *addr, float f)
{
    uint32_t *i = (uint32_t *)&f;
    __data20_write_long((unsigned long int)addr, *i);
}
