#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "driverlib.h"
#include "memory_util.h"

#define HZ                  5
#define INTERVAL            2 // 1 seconds
#define SAMPLES_PER_INTERVAL    HZ*INTERVAL // 10
#define INTERVAL_SET        20
#define MAX_NUM_INTERVAL    1024 // 1024 * 2 sec = 2048 sec
#define MAX_SAMPLES         HZ * INTERVAL_SET * INTERVAL // 62 * 15 * 2 = 30 seconds , it learns every time when 60 seconds samples are collected.

#define NUM_AUTOCORR_LAGS   50
#define LPF_FILT_LEN        9
#define DERIV_FILT_LEN      5

#define NUM_FEATURES        8
#define K                   3
#define PERCENT             0.9


int acc_x;
int acc_y;
int acc_z;
float p2p, zcr;

#pragma PERSISTENT(acc)
int acc[MAX_SAMPLES * 2][3] = { 0, };

#pragma PERSISTENT(num_samples)
int num_samples = 0;

#pragma PERSISTENT(features)
float features[MAX_NUM_INTERVAL][NUM_FEATURES] = { 0, };

#pragma PERSISTENT(normalized_features)
float normalized_features[MAX_NUM_INTERVAL][NUM_FEATURES] = { 0, };

#pragma PERSISTENT(features_min)
float features_min[NUM_FEATURES] = { 0, };

#pragma PERSISTENT(features_max)
float features_max[NUM_FEATURES] = { 0, };

#pragma PERSISTENT(ass)
float ass[MAX_NUM_INTERVAL] = { 0, };

#pragma PERSISTENT(num_intervals)
int num_intervals = 0;

#pragma PERSISTENT(mag_sqrt)
long mag_sqrt[SAMPLES_PER_INTERVAL] = { 0, };

#pragma PERSISTENT(lpf)
int32_t lpf[SAMPLES_PER_INTERVAL] = {0};

#pragma PERSISTENT(criterion_as)
float criterion_as = 0;


int sense()
{
    //GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    //printf("sense\n");

    static int sensor_init = 0;

    if (sensor_init == 0) {
        P7SEL0 |= BIT0 | BIT1;
        P7SEL1 &= ~(BIT0 | BIT1);

        PM5CTL0 &= ~LOCKLPM5;

        // Configure USCI_B2 for I2C mode
        UCB2CTLW0 = UCSWRST;                    // put eUSCI_B in reset state
        UCB2CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK; // I2C master mode, SMCLK
        UCB2BRW = 0x8;                          // baudrate = SMCLK / 8
        UCB2CTLW0 &= ~UCSWRST;                  // clear reset register
        //UCB2IE |= UCTXIE0 | UCNACKIE;           // transmit and NACK interrupt enable
        //UCB2CTLW1 |= UCASTP_2;

        EUSCI_B_I2C_setSlaveAddress(EUSCI_B2_BASE, 0x18);

        //EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
        //EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x23);
        //while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;
        //EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, (0x01 << 4));
        //while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;


        EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
        EUSCI_B_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, 0x23);
        EUSCI_B_I2C_masterSendMultiByteFinish(EUSCI_B2_BASE, (0x01 << 4));
        while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;
        //__delay_cycles(50);

        sensor_init = 1;
    }

    int ns = ir(&num_samples);
    int ni = ir(&num_intervals);
    int i;

    //GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    //GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);

    //EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    //EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x20);
    //while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;

    //EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    //EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x07 | (0x05 << 4));
    //while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;

    EUSCI_B_I2C_disable(EUSCI_B1_BASE);
    EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    EUSCI_B_I2C_enable(EUSCI_B1_BASE);
    EUSCI_B_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, 0x20);
    EUSCI_B_I2C_masterSendMultiByteFinish(EUSCI_B2_BASE, 0x07 | (0x05 << 4));
    while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;
    //__delay_cycles(50);

    for (i = 0; i < SAMPLES_PER_INTERVAL; i++) {

        int16_t x, y, z;

        EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
        EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x28);
        while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;
        x = EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B2_BASE);

        EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
        EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x29);
        while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;
        x |= ((uint16_t)EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B2_BASE)) << 8;

        EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
        EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x2A);
        while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;
        y = EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B2_BASE);

        EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
        EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x2B);
        while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;
        y |= ((uint16_t)EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B2_BASE)) << 8;

        EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
        EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x2C);
        while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;
        z = EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B2_BASE);

        EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
        EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x2D);
        while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;
        z |= ((uint16_t)EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B2_BASE)) << 8;

 //       if (ns == 0) {

            //printf("%d: %d %d %d\n", ns+i, x, y, z);

            iw(&acc[ns+i][0], x);
            iw(&acc[ns+i][1], y);
            iw(&acc[ns+i][2], z);

            acc_x = x;
            acc_y = y;
            acc_z = z;
#if 0
    } else {
            int last_x = ir(&acc[ns+i-1][0]);
            int last_y = ir(&acc[ns+i-1][1]);
            int last_z = ir(&acc[ns+i-1][2]);

            if (abs(last_x - x) >= 1000 || abs(last_y - y) >= 1000 || abs(last_z - z) >= 1000) {
                //printf("%d: %d %d %d\n", ns+i, x, y, z);

                iw(&acc[ns+i][0], x);
                iw(&acc[ns+i][1], y);
                iw(&acc[ns+i][2], z);

                acc_x = x;
                acc_y = y;
                acc_z = z;
            } else {
                i--;
            }
        }
#endif
    }

    //EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    //EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x20);
    //while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;

    //EUSCI_B_I2C_masterSendSingleByte(EUSCI_B2_BASE, 0x00);
    //while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;

    //EUSCI_B_I2C_setMode(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    EUSCI_B_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, 0x20);
    EUSCI_B_I2C_masterSendMultiByteFinish(EUSCI_B2_BASE, 0x00);
    while (EUSCI_B_I2C_isBusBusy(EUSCI_B2_BASE)) ;
    //__delay_cycles(50);

    //GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    //__delay_cycles(6000);
    //GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);


    iw(&num_samples, ns+SAMPLES_PER_INTERVAL);
    iw(&num_intervals, ni+1);

    //printf("ns = %d\n", ns);

    //GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);

    return 1;
}

int preprocess()
{
    //printf("preprocess\n");
    /* needs to be programmed by a user */
    return 1;
}

int select()
{
    //printf("select\n");
    /* needs to be programmed by a user */
    return 1;
}

int store()
{
    //printf("store\n");
    /* needs to be programmed by a user */
    return 1;
}

int pick()
{
    //printf("pick\n");
    /* needs to be programmed by a user */
    return 1;
}

int learnable()
{
    //printf("learnable\n");

    int ns = ir(&num_samples);

    if (ns >= MAX_SAMPLES) {
        //printf("ns = %d\n", ns);
        return 1;
    }

    return 0;
}

uint32_t SquareRoot(uint32_t a_nInput) {
    uint32_t op  = a_nInput;
    uint32_t res = 0;
    uint32_t one = 1uL << 30;
    while (one > op) {
        one >>= 2;
    }

    while (one != 0) {
        if (op >= res + one) {
            op  = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}

uint8_t get_precise_peakind(int64_t *autocorr_buff, uint8_t peak_ind) {
    uint8_t loop_limit = 0;
    if ((autocorr_buff[peak_ind] > autocorr_buff[peak_ind-1]) && (autocorr_buff[peak_ind] > autocorr_buff[peak_ind+1])) {
        //peak_ind is perfectly set at the peak. nothing to do
    }
    else if ((autocorr_buff[peak_ind] > autocorr_buff[peak_ind+1]) && (autocorr_buff[peak_ind] < autocorr_buff[peak_ind-1])) {
        //peak is to the left. keep moving in that direction
        loop_limit = 0;
        while ((autocorr_buff[peak_ind] > autocorr_buff[peak_ind+1]) && (autocorr_buff[peak_ind] < autocorr_buff[peak_ind-1]) && (loop_limit < 10)) {
            peak_ind = peak_ind - 1;
            loop_limit++;
        }
    }
    else {
        //peak is to the right. keep moving in that direction
        loop_limit = 0;
        while ((autocorr_buff[peak_ind] > autocorr_buff[peak_ind-1]) && (autocorr_buff[peak_ind] < autocorr_buff[peak_ind+1]) && (loop_limit < 10)) {
            peak_ind = peak_ind + 1;
            loop_limit++;
        }
    }
    return peak_ind;
}

//calculate deriviative via FIR filter
void derivative(int64_t *autocorr_buff, int64_t *deriv)
{
    int8_t deriv_coeffs[DERIV_FILT_LEN] = {-6,31,0,-31,6};
    uint8_t n          = 0;
    uint8_t i          = 0;
    int64_t temp_deriv = 0;
    for (n = 0; n < NUM_AUTOCORR_LAGS; n++) {
        temp_deriv = 0;
        for (i = 0; i < DERIV_FILT_LEN; i++) {
            if (n-i >= 0) {     //handle the case when n < filter length
                temp_deriv += deriv_coeffs[i]*autocorr_buff[n-i];
            }
        }
        deriv[n] = temp_deriv;
    }
}

//autocorrelation function
//this takes a lot of computation. there are efficient implementations, but this is more intuitive
void autocorr(int32_t *lpf, int64_t *autocorr_buff)
{
    uint8_t lag;
    uint16_t i;
    int64_t temp_ac;
    for (lag = 0; lag < NUM_AUTOCORR_LAGS; lag++) {
        temp_ac = 0;
        for (i = 0; i < SAMPLES_PER_INTERVAL-lag; i++) {
            temp_ac += (int64_t)lpf[i]*(int64_t)lpf[i+lag];
        }
        autocorr_buff[lag] = temp_ac;
    }
}

//calculate and remove the mean
void remove_mean(int32_t *lpf)
{
    int32_t sum = 0;
    uint16_t i;
    for (i = 0; i < SAMPLES_PER_INTERVAL; i++) {
        sum += lpf[i];
    }
    sum = sum/(SAMPLES_PER_INTERVAL);

    for (i = 0; i < SAMPLES_PER_INTERVAL; i++) {
        lpf[i] = lpf[i] - sum;
    }
}

//FIR low pass filter
void lowpassfilt(long *mag_sqrt, int32_t *lpf)
{
    int8_t lpf_coeffs[LPF_FILT_LEN] = {-5,6,34,68,84,68,34,6,-5};
    uint16_t n;
    uint8_t i;
    int32_t temp_lpf;
    for (n = 0; n < SAMPLES_PER_INTERVAL; n++) {
        temp_lpf = 0;
        for (i = 0; i < LPF_FILT_LEN; i++) {
            if (n-i >= 0) {     //handle the case when n < filter length
                temp_lpf += (int32_t)lpf_coeffs[i]*(int32_t)mag_sqrt[n-i];
            }
        }
        lpf[n] = temp_lpf;
    }
}

float get_mean(int interval)
{
    long long int sum = 0;
    int horizontal = 0;
    int i;

    for (i = 0; i < SAMPLES_PER_INTERVAL; i++) {
        horizontal = ir(&acc[interval*SAMPLES_PER_INTERVAL+i][0]);
        sum += horizontal;
    }

    return (float)sum / (float)SAMPLES_PER_INTERVAL;
}

float get_SD(int interval, float mean)
{
    float sum = 0;
    int horizontal = 0;
    float SD = 0;
    int i;

    for (i = 0; i < SAMPLES_PER_INTERVAL; i++) {
        horizontal = ir(&acc[interval*SAMPLES_PER_INTERVAL+i][0]);
        sum += ((float)horizontal - mean) * ((float)horizontal - mean);
    }

    SD = sqrt(sum / SAMPLES_PER_INTERVAL);

    return SD;
}

void insertionFloat(float *arr, int n)
{
    int i, j;
    float key;

    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i-1;

        while (j >= 0 && arr[j] > key) {
            arr[j+1] = arr[j];
            j = j-1;
        }
        arr[j+1] = key;
    }
}

void insertionSort(long *arr, int n)
{
    int i, j;
    long key;

    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i-1;

        while (j >= 0 && arr[j] > key) {
            arr[j+1] = arr[j];
            j = j-1;
        }
        arr[j+1] = key;
    }
}

long get_median(long *mag_sqrt)
{
    long new_mag[SAMPLES_PER_INTERVAL];
    int i;

    for (i = 0; i < SAMPLES_PER_INTERVAL; i++) {
        new_mag[i] = mag_sqrt[i];
    }

    insertionSort(new_mag, SAMPLES_PER_INTERVAL);

    return new_mag[SAMPLES_PER_INTERVAL/2];
}

float get_RMS(long *mag_sqrt)
{
    uint32_t sum = 0;
    float RMS = 0;
    int i;

    for (i = 0; i < SAMPLES_PER_INTERVAL; i++) {
        sum += mag_sqrt[i] * mag_sqrt[i];
    }

    RMS = sqrt((float)sum/SAMPLES_PER_INTERVAL);

    return RMS;
}

long get_P2P(int interval)
{
    long smallest = 99999999;
    long largest = -99999999;
    int i;

    for (i = 0; i < SAMPLES_PER_INTERVAL; i++) {
        int vertical = ir(&acc[interval*SAMPLES_PER_INTERVAL+i][2]);
        if (vertical < smallest) {
            smallest = vertical;
        }
        if (vertical > largest) {
            largest = vertical;
        }
    }

    p2p = (float)largest - (float)smallest;
    return largest - smallest;
}

float get_ZCR(int interval)
{
    float ZCR = 0;
    int sum = 0;
    int i;

    for (i = 1; i < SAMPLES_PER_INTERVAL; i++) {
        int vertical1 = ir(&acc[interval*SAMPLES_PER_INTERVAL+i-1][2]);
        int vertical2 = ir(&acc[interval*SAMPLES_PER_INTERVAL+i][2]);
        if (vertical1 < 0 && vertical2 > 0)
            sum++;
        else if (vertical1 > 0 && vertical2 < 0)
            sum++;
    }

    ZCR = (float)sum / (SAMPLES_PER_INTERVAL-1);
    zcr = ZCR;
    return ZCR;
}

float get_AAV_vertical(int interval)
{
    float AAV = 0;
    float sum = 0;
    int i;

    for (i = 0; i < SAMPLES_PER_INTERVAL-1; i++) {
         int vertical1 = ir(&acc[interval*SAMPLES_PER_INTERVAL+i][2]);
         int vertical2 = ir(&acc[interval*SAMPLES_PER_INTERVAL+i+1][2]);
         sum += (float)abs(vertical2 - vertical1) / (float)(SAMPLES_PER_INTERVAL-1);
    }

    AAV = sum;

    return AAV;
}

float get_AAV_horizontal(int interval)
{
    float AAV = 0;
    float sum = 0;
    int i;

    for (i = 0; i < SAMPLES_PER_INTERVAL-1; i++) {
        int horizontal1 = ir(&acc[interval*SAMPLES_PER_INTERVAL+i][0]);
        int horizontal2 = ir(&acc[interval*SAMPLES_PER_INTERVAL+i+1][0]);
        sum += (float)abs(horizontal2 - horizontal1) / (float)(SAMPLES_PER_INTERVAL-1);
    }

    AAV = sum;

    return AAV;
}

void feature_extract(int interval)
{
    //printf("feature_extract %d\n", interval);

    long local_mag_sqrt[SAMPLES_PER_INTERVAL] = { 0, };
    int32_t local_lpf[SAMPLES_PER_INTERVAL] = { 0, };
    int64_t local_autocorr_buff[NUM_AUTOCORR_LAGS] = { 0, };
    int64_t local_deriv[NUM_AUTOCORR_LAGS] = { 0, };

    int i = 0;

    for (i = 0; i < SAMPLES_PER_INTERVAL; i++) {
        int x = ir(&acc[interval*SAMPLES_PER_INTERVAL+i][0]);
        int y = ir(&acc[interval*SAMPLES_PER_INTERVAL+i][1]);
        int z = ir(&acc[interval*SAMPLES_PER_INTERVAL+i][2]);
        local_mag_sqrt[i] = (long)SquareRoot(x*x + y*y + z*z);
        //printf(" x y z = %d %d %d %u\n", x, y, z, local_mag_sqrt[i]);
        lw(&mag_sqrt[i], local_mag_sqrt[i]);
    }

    //lowpassfilt(local_mag_sqrt, local_lpf);
    //remove_mean(local_lpf);
    //autocorr(local_lpf, local_autocorr_buff);
    //derivative(local_autocorr_buff, local_deriv);

    //uint8_t peak_ind = 0;

    //for (i = 8; i < NUM_AUTOCORR_LAGS; i++) {
    //    if ((local_deriv[i] > 0) && (local_deriv[i-1] > 0) && (local_deriv[i-2] < 0) && (local_deriv[i-3] < 0)) {
    //        peak_ind = i-1;
    //        break;
    //    }
    //}

    //peak_ind = get_precise_peakind(local_autocorr_buff, peak_ind);

    float f[NUM_FEATURES] = { 0, };

    f[0] = get_mean(interval);
    f[1] = get_SD(interval, f[0]);
    f[2] = (float)get_median(local_mag_sqrt);
    f[3] = get_RMS(local_mag_sqrt);
    f[4] = (float)get_P2P(interval);
    f[5] = get_ZCR(interval);
    f[6] = get_AAV_vertical(interval);
    f[7] = get_AAV_horizontal(interval);
    //f[8] = (float)peak_ind;

    for (i = 0; i < NUM_FEATURES; i++) {
        fw(&features[interval][i], f[i]);
    }
}

void single_normalization(int ni)
{
    int j;
    float f, normalized_f;

    for (j = 0; j < NUM_FEATURES; j++) {
        f = fr(&features[ni][j]);

        float min = fr(&features_min[j]);
        float max = fr(&features_max[j]);
        //printf("ni = %d f = %f min = %f max = %f\n", ni, f, min, max);

        normalized_f = (f - min) / (max - min);
        fw(&normalized_features[ni][j], normalized_f);
    }
}

void normalization(int ni)
{
    int i, j;
    float f_min[NUM_FEATURES] = { 0, };
    float f_max[NUM_FEATURES] = { 0, };
    float f, normalized_f;

    for (i = 0; i < ni; i++) {
        for (j = 0; j < NUM_FEATURES; j++) {
            f = fr(&features[i][j]);
            if (i == 0) {
                f_min[j] = f;
                f_max[j] = f;
            } else {
                if (f < f_min[j]) {
                    f_min[j] = f;
                }

                if (f > f_max[j]) {
                    f_max[j] = f;
                }
            }
        }
    }

    for (j = 0; j < NUM_FEATURES; j++) {
        fw(&features_min[j], f_min[j]);
        fw(&features_max[j], f_max[j]);
    }

    for (i = 0; i < ni; i++) {
        for (j = 0; j < NUM_FEATURES; j++) {
            f = fr(&features[i][j]);
            normalized_f = (f - f_min[j]) / (f_max[j] - f_min[j]);
            fw(&normalized_features[i][j], normalized_f);
        }
    }
}

float get_distance(float f1[NUM_FEATURES], float f2[NUM_FEATURES])
{
    float distance = 0;
    float sum = 0;
    int i;

    for (i = 0; i < NUM_FEATURES; i++) {
        sum += (f1[i] - f2[i]) * (f1[i] - f2[i]);
    }

    distance = sqrt(sum);

    return distance;
}

int find_max(float *array, int len)
{
    float max = -999999;
    int max_idx = 0;
    int i;

    for (i = 0; i < len; i++) {
        if (array[i] > max) {
            max = array[i];
            max_idx = i;
        }
    }

    return max_idx;
}

float getAss(int interval)
{
    float short_distances[K] = { 0, };
    float distance = 0;
    float f1[NUM_FEATURES];
    float f2[NUM_FEATURES];
    float anomaly_score = 0;
    int i, j;
    int k = 0;

    for (j = 0; j < NUM_FEATURES; j++) {
        f1[j] = fr(&normalized_features[interval][j]);
    }

    int ni = ir(&num_intervals);

    for (i = 0; i < ni; i++) {
        for (j = 0; j < NUM_FEATURES; j++) {
            f2[j] = fr(&normalized_features[i][j]);
        }

        distance = get_distance(f1, f2);
        //printf("distance = %f\n", distance);

        if (k < K) {
            short_distances[k++] = distance;
        } else {
            int max_idx = find_max(short_distances, K);
            if (distance < short_distances[max_idx]) {
                short_distances[max_idx] = distance;
            }
        }
    }

    for (i = 0; i < K; i++) {
        anomaly_score += short_distances[i];
    }

    return anomaly_score;
}

int extract()
{
    //printf("extract\n");
    int ns = ir(&num_samples);
    int ni = ir(&num_intervals);
    int i;

    //printf("ns = %d ni = %d\n", ns, ni);

    if (ni % INTERVAL_SET == 0) {
        for (i = 0; i < ni; i++) {
            feature_extract(i);
        }
    } else {
        feature_extract(ni-1);
    }

    //iw(&num_samples, 0);
    return 1;

 #if 0
    if (ni >= INTERVAL_SET) {
        for (i = 0; i < INTERVAL_SET; i++) {
            feature_extract(i);
        }

        //iw(&num_intervals, 0);
        iw(&num_samples, 0);
        return 1;
    } else {
        feature_extract(ni);
        iw(&num_samples, 0);
    }
#endif
    return 0;
}

int learn1()
{
    //printf("learn1\n");
    //printf("ni = %d\n", ni);
    normalization(INTERVAL_SET);

    return 1;
}

int learn2()
{
    int i;
    int ni = ir(&num_intervals);

    float as_scores[INTERVAL_SET] = { 0, };
    for (i = 0; i < INTERVAL_SET; i++) {
        float as = getAss(i);
        as_scores[i] = as;
        //printf("as_scores[%d] = %f\n", i, as);
    }

    insertionFloat(as_scores, INTERVAL_SET);

    int idx = INTERVAL_SET * PERCENT;
    //printf("idx = %d\n", idx);

    fw(&criterion_as, as_scores[idx]);

    return 1;
}

int evaluate()
{
    //printf("evaluate\n");
    /* needs to be programmed by a user */
    return 1;
}

int expect()
{
    //printf("expect\n");
    /* needs to be programmed by a user */
    return 1;
}

int infer1()
{
    //printf("infer1\n");

    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);

    int ns = ir(&num_samples);
    int ni = ir(&num_intervals);
    //printf("ns = %d ni = %d\n", ns, ni);
    //int i;

    //for (i = 0; i < 5; i++) {
    //    printf("%d: %d %d %d\n", ns-5+i, ir(&acc[ns-5+i][0]), ir(&acc[ns-5+i][1]), ir(&acc[ns-5+i][2]));
    //}

    single_normalization(ni-1);

    return 1;
}

int infer2()
{
    //printf("infer2\n");

    int ns = ir(&num_samples);
    int ni = ir(&num_intervals);

    //feature_extract(ni);

    //int ns = ir(&num_samples);
    //iw(&num_samples, ns-1);

    float as = getAss(ni-1);
    float cs = fr(&criterion_as);

    //printf("as = %f cs = %f\n", as, cs);
    iw(&num_samples, ns-SAMPLES_PER_INTERVAL);
    iw(&num_intervals, ni-1);

    if (as > cs) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        return 1;
    }

    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1);
    return 0;
}

int (*sense_[])() = { sense };
int (*preprocess_[])() = { preprocess };
int (*select_[])() = { select };
int (*store_[])() = { store };
int (*pick_[])() = { pick };
int (*learnable_[])() = { learnable };
int (*extract_[])() = { extract };
int (*learn_[])() = { learn1, learn2 };
int (*evaluate_[])() = { evaluate };
int (*expect_[])() = { expect };
int (*infer_[])() = { infer1, infer2 };


int get_num_sense()
{
    return sizeof(sense_) / sizeof(sense_[0]);
}

int get_num_preprocess()
{
    return sizeof(preprocess_) / sizeof(preprocess_[0]);
}

int get_num_select()
{
    return sizeof(select_) / sizeof(select_[0]);
}

int get_num_store()
{
    return sizeof(store_) / sizeof(store_[0]);
}

int get_num_pick()
{
    return sizeof(pick_) / sizeof(pick_[0]);
}

int get_num_learnable()
{
    return sizeof(learnable_) / sizeof(learnable_[0]);
}

int get_num_extract()
{
    return sizeof(extract_) / sizeof(extract_[0]);
}

int get_num_learn()
{
    return sizeof(learn_) / sizeof(learn_[0]);
}

int get_num_evaluate()
{
    return sizeof(evaluate_) / sizeof(evaluate_[0]);
}

int get_num_expect()
{
    return sizeof(expect_) / sizeof(expect_[0]);
}

int get_num_infer()
{
    return sizeof(infer_) / sizeof(infer_[0]);
}

int (*get_sense(int num))()
{
    return sense_[num];
}

int (*get_preprocess(int num))()
{
    return preprocess_[num];
}

int (*get_store(int num))()
{
    return store_[num];
}

int (*get_select(int num))()
{
    return select_[num];
}

int (*get_pick(int num))()
{
    return pick_[num];
}

int (*get_learnable(int num))()
{
    return learnable_[num];
}

int (*get_extract(int num))()
{
    return extract_[num];
}

int (*get_learn(int num))()
{
    return learn_[num];
}

int (*get_evaluate(int num))()
{
    return evaluate_[num];
}

int (*get_expect(int num))()
{
    return expect_[num];
}

int (*get_infer(int num))()
{
    return infer_[num];
}
